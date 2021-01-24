#include "notify.h"
#include "config.h"

//====================================
// Globals
//------------------------------------
sigset_t block_set;
int FD[3] = {-1,-1,-1};
void* self_notify = NULL;

//====================================
// External Globals
//------------------------------------
extern QString myFolder;

//====================================
// Constructor
//------------------------------------
Notify::Notify(Parser* parse, bool enable_notify_events) {
    self_notify = this;
    mFolderNames = parse->folderList();

    setFolders();

    if (enable_notify_events) {
        activateFolderNotification();
    }
}

//====================================
// setFolders
//------------------------------------
void Notify::setFolders(void) {
    QList<QString> subdir;
    subdir.append ("/new");
    subdir.append ("/cur");
    QListIterator<QString> it (mFolderNames);
    while (it.hasNext()) {
        QPoint dirCount;
        QString value = it.next();
        for (int i=0;i<subdir.count();i++) {
            int count = getFiles (
                myFolder + QString(value + subdir.at(i)) + "/*"
            );
            if (i == 0) {
                dirCount.setX (count);
            }
            if (i == 1) {
                dirCount.setY (count);
            }
            if (i == 1) {
                Folder* folder = 0;
                folder = getFolder(value);
                if (! folder) {
                    folder = new Folder (value, dirCount);
                    mFolderList.append (folder);
                } else {
                    folder->setStatus(dirCount.y(), dirCount.x());
                }
            }
        }
    }
}

//=========================================
// activateFolderNotification
//-----------------------------------------
void Notify::activateFolderNotification (void) {
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

    QList<QString> subdir;
    subdir.append ("/new");
    subdir.append ("/cur");
    QListIterator<QString> it (mFolderNames);
    while (it.hasNext()) {
        QString folderName = it.next();
        for (int i=0;i<subdir.count();i++) {
            QString subDir = subdir.at(i);
            for (int n=0;n<2;n++) {
                QString fname (myFolder + folderName+subDir);
                int fd = open (
                    fname.toLatin1().data(), O_RDONLY
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
                QString folder;
                QTextStream(&folder) << folderName << subDir;
                mNotifyDirs.insert (
                    fd, folder
                );
                mFDs << fd;
            }
        }
    }
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
// Real time signal arrived
//-----------------------------------------
void handleNotifyEvent(int, siginfo_t* si , void*) {
    Notify* obj = (Notify*)self_notify;
    if (obj) {
        emit obj->folderChanged(si->si_fd);
    }
    qDebug("Notify for FD %d", si->si_fd);
}

//=========================================
// getFolder
//-----------------------------------------
Folder* Notify::getFolder(QString folder_name) {
    Folder* folder = 0;
    QListIterator<Folder*> it (mFolderList);
    while (it.hasNext()) {
        folder = it.next();
        if (folder->getFolder() == folder_name) {
            return folder;
        }
    }
    return 0;
}

//=========================================
// getInitialFolderList
//-----------------------------------------
QList<Folder*> Notify::getFolderList(void) {
    return mFolderList;
}

