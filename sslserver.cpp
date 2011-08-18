/**************
FILE          : sslserver.cpp
***************
PROJECT       : QBiff - A qt based biff
              :
AUTHOR        : Marcus Schäfer <ms@suse.de>
              :
BELONGS TO    : new mail notification service 
              : 
              :
DESCRIPTION   : network related socket functions
              :
              :
STATUS        : Status: Beta
**************/
#include "sslserver.h"

//=========================================
// Globals...
//-----------------------------------------
QList<SSLServerConnection> mSSLConnections;

//=========================================
// Constructor...
//-----------------------------------------
SSLServer::SSLServer ( void ) {
	mInit = new SSLServerInit (this);
	QObject::connect (
		mInit , SIGNAL ( clientInit       (void) ),
		this  , SLOT   ( serverClientInit (void) )
	);
}

//=========================================
// run Thread...
//-----------------------------------------
void SSLServer::run ( void ) {
	mInit -> openConnection ();
}

//=========================================
// have open connections ?...
//-----------------------------------------
bool SSLServer::isOpen (void) {
	if (mSSLConnections.isEmpty()) {
		return false;
	}
	return true;
}

//=========================================
// re-emit init clientInit...
//-----------------------------------------
void SSLServer::serverClientInit (void) {
	clientInit();
}

//=========================================
// writeClient...
//-----------------------------------------
void SSLServer::writeClient ( const QString & data ) {
	mInit -> writeClient ( data );
}

//=========================================
// free SSL context...
//-----------------------------------------
void SSLServer::SSLFree ( void ) {
	mInit -> SSLFree();
}
