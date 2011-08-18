/**************
FILE          : clientinit.h
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
#ifndef CLIENT_INIT_H
#define CLIENT_INIT_H 1

#include <qthread.h>
#include "sslclient.h"

//=========================================
// Class ClientInit
//-----------------------------------------
class ClientInit : public QThread {
	Q_OBJECT

	private:
	SSLClient* mClient;

	private slots:
	void gotLine ( QString );

	public:
	ClientInit ( void );

	public:
	void writeClient ( const QString & );
	virtual void run ( void );

	signals:
	void gotEvent (QString);
};

#endif
