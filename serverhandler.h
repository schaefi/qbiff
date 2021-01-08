/**************
FILE          : serverhandler.h
***************
PROJECT       : QBiff - A qt based biff
              :
AUTHOR        : Marcus Schäfer <ms@suse.de>
              :
BELONGS TO    : new mail notification service 
              : 
              :
DESCRIPTION   : header file for server only usage
              :     
              :
STATUS        : Status: Beta
**************/
#ifndef SERVERHANDLER_H
#define SERVERHANDLER_H 1

#include <sys/types.h>
#include <pwd.h>
#include <dirent.h>

#include "parser.h"
#include "notify.h"
#include "sslserver.h"

//=========================================
// Defines...
//-----------------------------------------
#define FOLDER_EMPTY     "empty"
#define FOLDER_UPTODATE  "uptodate"
#define FOLDER_NEW       "new"
#define FOLDER_UNKNOWN   "unknown"

//=========================================
// Globals...
//-----------------------------------------
extern QString user_name;

//=========================================
// Class ServerFolder
//-----------------------------------------
class ServerFolder {
	public:
	ServerFolder (QString*,QPoint*,SSLServer*);

	private:
	SSLServer* mServer;
	QString    mFolder;
	unsigned int mCurrent;
	unsigned int mNew;

	public:
	QString status    (void);
	QString getFolder (void);

	public:
	void setStatus (QPoint*);
	void updateFolder (void);
};

//=========================================
// Class ServerHandler
//-----------------------------------------
class ServerHandler : public QObject {
	Q_OBJECT

	public:
	ServerHandler ( QObject * p = 0);

	private:
	QList<ServerFolder*> mFolderList;
	SSLServer* mServer;
	Notify*    mNotify; 

	private slots:
	void slotNotify (QString,QPoint*);
	void clientInit (void);

	public:
	void poll (void);
};

#endif
