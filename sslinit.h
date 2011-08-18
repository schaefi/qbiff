/**************
FILE          : sslinit.h
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
#ifndef SSL_INIT_H
#define SSL_INIT_H 1

#include <qstringlist.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qhash.h>
#include <signal.h>
#include <qobject.h>
#include <qthread.h>

#include "sslcommon.h"
#include "config.h"

//=========================================
// Class SSLServerInit
//-----------------------------------------
class SSLServerInit : public SSLCommon {
	Q_OBJECT

	public:
	SSLServerInit ( QObject* = 0);

	private:
	BIO     *acc;
	SSL_CTX *ctx;

	private:
	void setupServerCTX (void);

	public:
	void writeClient ( const QString & );
	void openConnection ( void );
	void SSLFree ( void );

	private slots:
	void initClientInit ( void );

	signals:
	void clientInit (void);
};

#endif
