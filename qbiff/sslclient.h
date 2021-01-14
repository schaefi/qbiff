/**************
FILE          : sslclient.h
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
#ifndef SSL_CLIENT_H
#define SSL_CLIENT_H 1

#include <qstringlist.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qhash.h>
#include <qthread.h>

#include "sslcommon.h"
#include "config.h"

//=========================================
// Class SSLClient
//-----------------------------------------
class SSLClient : public QThread {
    Q_OBJECT

    public:
    SSLClient(void);

    private:
    bool mDataToWrite;
    int mSocket;
    fd_set readFDs;
    SSL_CTX* ctx;
    SSL* ssl;

    protected:
    void run(void) override;

    private:
    void setupClientCTX (void);
    void connectTCP (void);

    public:
    void SSLFree (void);

    signals:
    void gotLine(QString);
};

#endif
