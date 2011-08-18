/**************
FILE          : sslserver.h
***************
PROJECT       : QBiff - A qt based biff
              :
AUTHOR        : Marcus Schäfer <ms@suse.de>
              :
BELONGS TO    : new mail notification service 
              : 
              :
DESCRIPTION   : header file for network socket stuff
              :
              :
STATUS        : Status: Beta
**************/
#ifndef SSL_CONNECTION_H
#define SSL_CONNECTION_H 1

#include <qstringlist.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qhash.h>
#include <signal.h>
#include <qobject.h>
#include <qthread.h>

#include "sslcommon.h"

//=========================================
// Class SSLServerConnection
//-----------------------------------------
class SSLServerConnection : public QThread {
	Q_OBJECT

	private:
	SSL* ssl;

	public:
	SSLServerConnection ( SSL* );
	void writeClient ( const QString & );
	void shutdown ( void );
	virtual void run ( void );
	
	private:
	int readClient ( void );

	signals:
	void clientInit (void);
};

#endif
