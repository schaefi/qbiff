/**************
FILE          : notify.h
***************
PROJECT       : QBiff - A qt based biff
              :
AUTHOR        : Marcus Schäfer <ms@suse.de>
              :
BELONGS TO    : new mail notification service 
              : 
              :
DESCRIPTION   : header file for F_NOTIFY support
              :
              :
STATUS        : Status: Beta
**************/
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
    Notify(Parser*, bool=false);

    private:
    bool mSupportNotifyEvents;
    QList<Folder*> mFolderList;
    QVector<int> mFDs;
    Parser* mParse;

    public:
    QList<Folder*> getFolderList(void);
    void handleNotifySignal(int);
    void setFolders(bool=false);
    Folder* getFolder(QString);

    private:
    int getFiles (const QString&);
    void activateFolderNotification(const QString&,const QString&);
    void cleanActiveFolderNotification(void);

    private:
    QHash<int,QString*> mNotifyDirs;
    QHash<int,QPoint*>  mNotifyCount;
};

#endif
