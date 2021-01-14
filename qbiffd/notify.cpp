/**************
FILE          : notify.c
***************
PROJECT       : QBiff - A qt based biff
              :
AUTHOR        : Marcus Schäfer <ms@suse.de>
              :
BELONGS TO    : new mail notification service 
              : 
              :
DESCRIPTION   : kernel F_NOTIFY support
              :
              :
STATUS        : Status: Beta
**************/
#include "notify.h"
#include "config.h"

//====================================
// Globals
//------------------------------------
sigset_t block_set;
int FD[3] = {-1,-1,-1};
void* self = NULL;

//====================================
// External Globals
//------------------------------------
extern QString myFolder;

//====================================
// Constructor
//------------------------------------
Notify::Notify(Parser* parse, bool enable_notify_events) {
    self = this;
    mParse = parse;
    mSupportNotifyEvents = enable_notify_events;

    setFolders();

    // NOTE: I stepped away from the concept of using the
    // notify SIGRTMIN[X] signals to react on changes of
    // folder files. The reason was that the code in the
    // event handler (see handleNotifySignal) contains
    // code that is not race free. The code writes data
    // via folder->setXXX method and sends data as received
    // via folder->getXXX. If there is another SIGRTMIN[X]
    // signale between setter and getter calls it can happen
    // that the getter doesn't see what actually was set.
    // In an event of many SIGRTMIN[X] signals on e.g
    // [rm Mail/INBOX.suse.spam/new/*] the race condition
    // can be reproduced relatively easy.
    //
    // Therefore the support for notify events is still
    // present but not in use at the moment since I moved
    // the serialization into QThreads and Mutex locks
    //
    if (mSupportNotifyEvents) {
        struct sigaction action;
        action.sa_sigaction = handleNotifyEvent;
        sigemptyset (&action.sa_mask);
        action.sa_flags = SA_SIGINFO;
        sigaction (SIGRTMIN + 0 , &action , 0);
        sigaction (SIGRTMIN + 1 , &action , 0);

        sigemptyset (&block_set);
        sigaddset (&block_set,SIGIO);
        for (int i=0;i<2;i++) {
            sigaddset (&block_set,SIGRTMIN + i);
        }
    }
}

//====================================
// setFolders
//------------------------------------
void Notify::setFolders(bool clean) {
    sigprocmask(SIG_BLOCK, &block_set,0);
    QList<char*> mFolderNames = mParse -> folderList();
    if (clean) {
        cleanActiveFolderNotification();
        fdatasync (STDOUT_FILENO);
    }
    QList<char*> subdir;
    subdir.append ((char*)"/new");
    subdir.append ((char*)"/cur");
    QListIterator<char*> it ( mFolderNames );
    int FDcount = 0;
    while (it.hasNext()) {
        QPoint* dirCount = 0;
        char* value = it.next();
        for (int i=0;i<subdir.count();i++) {
            if (i == 0) {
                dirCount = new QPoint;
            }
            int count = getFiles (
                myFolder + QString(value+QString(subdir.at(i))+"/*")
            );
            if (i == 0) {
                dirCount->setX (count);
            }
            if (i == 1) {
                dirCount->setY (count);
            }
            if (mSupportNotifyEvents) {
                activateFolderNotification (value,subdir.at(i));
            }
            if (i == 1) {
                int start = FDcount;
                int ended = FDcount + 4;
                for (int n=start;n<ended;n++) {
                    mNotifyCount.insert ( mFDs[n], dirCount );
                }
                FDcount = ended;
                Folder* initial = new Folder (new QString(value), dirCount);
                mFolderList.append (initial);
            }
        }
    }
    sigprocmask(SIG_UNBLOCK, &block_set,0);
}

//=========================================
// activateFolderNotification
//-----------------------------------------
void Notify::activateFolderNotification (
    const QString& folderName, const QString& subDir
) {
    for (int n=0;n<2;n++) {
        QString fname (myFolder + folderName+subDir);
        int fd = open (
            fname.toLatin1().data(),
            O_RDONLY
        );
        if (fd == -1) {
            return;
        }
        fcntl (fd, F_SETSIG, SIGRTMIN + n);
        long flags = 0;
        switch (n) {
            case 0:
                flags = DN_MULTISHOT | DN_CREATE;
            break;
            case 1:
                flags = DN_MULTISHOT | DN_DELETE;
            break;
            default:
            break;
        }
        if (fcntl (fd,F_NOTIFY, flags) == -1) {
            return;
        }
        QString* folder = new QString (
            folderName+subDir
        );
        mNotifyDirs.insert (
            fd, folder
        );
        mFDs << fd;
    }
}

//=========================================
// cleanActiveFolderNotification
//-----------------------------------------
void Notify::cleanActiveFolderNotification (void) {
    for (int value = 0; value < mFDs.size(); value++) {
        fcntl (value, F_NOTIFY, 0);
        fcntl (value, F_SETSIG, 0);
        close (value);
    }
    mFolderList.clear();
    mNotifyDirs.clear();
    mNotifyCount.clear();
    mFDs.clear();
}

//=========================================
// count files
//-----------------------------------------
int Notify::getFiles (const QString& pattern) {
    int count = 0;
    glob_t globbuf;
    if (glob (pattern.toLatin1().data(), GLOB_MARK, 0, &globbuf) == 0) {
        count = globbuf.gl_pathc;
        //for (unsigned int n=0;n< globbuf.gl_pathc;n++) {
        //    printf ("%s\n",globbuf.gl_pathv[n]);
        //}
    }
    globfree (&globbuf);
    return count;
}

//=========================================
// handleNotifySignal
//-----------------------------------------
void Notify::handleNotifySignal(int fd) {
    if (mNotifyDirs[fd]) {
        // block signals when we are here
        sigprocmask(SIG_BLOCK, &block_set, 0);

        // read current file status from folder
        QString* pFolder = mNotifyDirs[fd];
        QStringList tokens = pFolder->split ( "/" );
        QString folder_name  = tokens.first();
        QString dirname = tokens.last();
        QString folder_path;
        QTextStream(&folder_path) << myFolder
            << folder_name << "/" << dirname << "/*";
        int file_count = getFiles(folder_path);

        // get matching Folder pointer from folder list
        Folder* folder = NULL;
        QListIterator<Folder*> it (mFolderList);
        while (it.hasNext()) {
            folder = it.next();
            if (folder->getFolder() == folder_name) {
                break;
            }
        }
        fdatasync (STDOUT_FILENO);
        if (folder) {
            if (dirname == "new") {
                folder->setNew(file_count);
            } else {
                folder->setCurrent(file_count);
            }
            qDebug("--> event: %s", folder->getStatus().toLatin1().data());
        }

        // unblock notify signals
        sigprocmask(SIG_UNBLOCK, &block_set, 0);
    }
}

//=========================================
// Real time signal arrived
//-----------------------------------------
void handleNotifyEvent(int, siginfo_t* si , void*) {
    Notify* obj = (Notify*)self;
    if (obj) {
        obj -> handleNotifySignal(si->si_fd);
    }
}

//=========================================
// getInitialFolderList
//-----------------------------------------
QList<Folder*> Notify::getFolderList(void) {
    return mFolderList;
}

