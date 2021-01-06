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
#ifndef SSL_SERVER_H
#define SSL_SERVER_H 1

#include <qapplication.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qhash.h>
#include <signal.h>
#include <qobject.h>
#include <qthread.h>

#include "sslinit.h"
#include "sslconnection.h"

//=========================================
// Class SSLServer
//-----------------------------------------
class SSLServer : public QThread {
	Q_OBJECT

	private:
	SSLServerInit* mInit;

	public:
	SSLServer ( void );

	public:
	bool isOpen (void);
	void writeClient ( const QString & );
	virtual void run ( void ) override;
	void SSLFree ( void );

	private slots:
	void serverClientInit ( void );

	signals:
	void clientInit (void);
};

#endif
