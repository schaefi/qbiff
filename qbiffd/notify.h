#ifndef NOTIFY_H
#define NOTIFY_H 1

#include <stdio.h>
#include <stdlib.h>
#include <qlist.h>
#include <qvector.h>
#include <qhash.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/types.h>
#include <glob.h>

#include "parser.h"
#include "folder.h"

#include <QFileSystemWatcher>

//=========================================
// Functions...
//-----------------------------------------
void handleNotifyEvent(int,siginfo_t*,void*);

//=========================================
// Class Notify...
//-----------------------------------------
class Notify : public QObject {
    Q_OBJECT

    public:
    Notify(Parser*, QFileSystemWatcher&);

    private:
    QList<QString> mFolderNames;
    QList<Folder*> mFolderList;
    QVector<int> mFDs;

    public:
    QList<Folder*> getFolderList(void);
    void setFolders(void);
    Folder* getFolder(QString);

    private:
    int getFiles (const QString&);
    void activateFolderNotification(QFileSystemWatcher&);

    private:
    QHash<int, QString> mNotifyDirs;

    signals:
    void folderChanged(void);
};

#endif
