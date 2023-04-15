#include "notify.h"
#include "config.h"

//====================================
// Globals
//------------------------------------
Notify* self_notify = 0;

//====================================
// External Globals
//------------------------------------
extern QString myFolder;

//====================================
// Constructor
//------------------------------------
Notify::Notify(Parser* parse, QFileSystemWatcher& watcher) {
    mFolderNames = parse->folderList();

    setFolders();

    self_notify = this;
    activateFolderNotification(watcher);
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
void Notify::activateFolderNotification (QFileSystemWatcher& watcher) {
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
                watcher.addPath(fname);
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
void handleNotifyEvent(int, siginfo_t*, void*) {
    if (self_notify) {
        emit self_notify->folderChanged();
    }
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

