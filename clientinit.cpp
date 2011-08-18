/**************
FILE          : clientinit.cpp
***************
PROJECT       : QBiff - A qt based biff
              :
AUTHOR        : Marcus Schäfer <ms@suse.de>
              :
BELONGS TO    : new mail notification service 
              : 
              :
DESCRIPTION   : implementation class for a client thread
              : which permanently accesses the SSL port
              :
              :
STATUS        : Status: Beta
**************/
#include "clientinit.h"

//=========================================
// Constructor
//-----------------------------------------
ClientInit::ClientInit ( void ) {
	mClient = new SSLClient;
	connect (
		mClient , SIGNAL (gotLine ( QString )),
		this    , SLOT   (gotLine ( QString ))
	);
	mClient -> writeClient ("INIT");
}

//=========================================
// gotLine
//-----------------------------------------
void ClientInit::gotLine ( QString line ) {
	gotEvent (line);
}

//=========================================
// writeClient
//-----------------------------------------
void ClientInit::writeClient (const QString & cmd) {
	mClient -> writeClient (cmd);
}

//=========================================
// run Thread...
//-----------------------------------------
void ClientInit::run ( void ) {
	mClient -> clientReadWrite ();
}

