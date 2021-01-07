/**************
FILE          : serverhandler.cpp
***************
PROJECT       : QBiff - A qt based biff
              :
AUTHOR        : Marcus Schäfer <ms@suse.de>
              :
BELONGS TO    : new mail notification service 
              : 
              :
DESCRIPTION   : implementation class for the server
              : only mode
              :
STATUS        : Status: Beta
**************/
#include "serverhandler.h"

//====================================
// Constructor
//------------------------------------
ServerFolder::ServerFolder (
	QString* folder, QPoint* count,SSLServer* server
) {
	mServer  = server;
	mFolder  = *folder;
	mCurrent = count->y();
	mNew     = count->x();
}

//====================================
// getFolder
//------------------------------------
QString ServerFolder::getFolder (void) {
	return mFolder;
}

//====================================
// set folder status
//------------------------------------
void ServerFolder::setStatus (QPoint* count) {
	mCurrent = count->y();
	mNew     = count->x();
}

//====================================
// get folder status
//------------------------------------
QString ServerFolder::status (void) {
	if ((mCurrent == 0) && (mNew == 0)) {
		return (FOLDER_EMPTY);
	}
	if ((mCurrent > 0) && (mNew == 0)) {
		return (FOLDER_UPTODATE);
	}
	if (mNew > 0) {
		return (FOLDER_NEW);
	}
	return (FOLDER_UNKNOWN);
}

//====================================
// send update string to ssl socket
//------------------------------------
void ServerFolder::updateFolder (void) {
	if (mServer->isOpen()) {
		QString folderStatus = status();
		QString stream;
		QTextStream(&stream) << mFolder
			<< ":" << folderStatus << ":" << mNew << ":" << mCurrent;
		mServer -> writeClient (stream);
	}
}

//=========================================
// Constructor
//-----------------------------------------
ServerHandler::ServerHandler ( QObject * p ) : QObject (p) {
	struct passwd *pw = getpwuid(getuid());
	Parser* parse = new Parser (QString(pw->pw_dir)+"/.qbiffrc");
	mNotify = new Notify (parse);
	mServer = new SSLServer;
	connect (
		mNotify , SIGNAL ( sigNotify  (QString*,QPoint*) ),
		this    , SLOT   ( slotNotify (QString*,QPoint*) )
	);
	connect (
		mServer , SIGNAL ( clientInit (void) ),
		this    , SLOT   ( clientInit (void) )
	);
	QList<NotifyCount*> initialFolders = mNotify->getInitialFolderList();
	QListIterator<NotifyCount*> io (initialFolders);
	while (io.hasNext()) {
		NotifyCount* data = io.next();
		ServerFolder* folder = new ServerFolder (
			data->getFolder(),data->getCount(),mServer
		);
		mFolderList.append (folder);
	}
	mServer->start();
}

//=========================================
// slotNotify
//-----------------------------------------
void ServerHandler::slotNotify (QString* mailbox,QPoint* count) {
	QListIterator<ServerFolder*> it (mFolderList);
	while (it.hasNext()) {
		ServerFolder* thisFolder = it.next();
        QString mail_box(mailbox->toLatin1());
        QString folder_name(thisFolder->getFolder());
        if (folder_name == mail_box) {
            // qDebug("Updating mailbox: " + mailbox->toLatin1());
			thisFolder->setStatus (count);
			thisFolder->updateFolder();
			break;
		}
	}
}

//=========================================
// clientInit
//-----------------------------------------
void ServerHandler::clientInit (void) {
	QListIterator<ServerFolder*> it (mFolderList);
	while (it.hasNext()) {
		ServerFolder* thisFolder = it.next();
		thisFolder -> updateFolder();
	}
	mServer -> writeClient ("INIT_DONE");
}

//=========================================
// poll
//-----------------------------------------
void ServerHandler::poll (void) {
	// ... /
	// RT signal queue is full, the result is a SIGIO and we
	// need to check all notify-directories
	// ...
	mNotify -> init (true);
	mFolderList.clear();
	QList<NotifyCount*> initialFolders = mNotify->getInitialFolderList();
	QListIterator<NotifyCount*> io (initialFolders);
	while (io.hasNext()) {
		NotifyCount* data = io.next();
		ServerFolder* folder = new ServerFolder (
			data->getFolder(),data->getCount(),mServer
		);
		folder -> updateFolder();
		mFolderList.append (folder);
	}
}
