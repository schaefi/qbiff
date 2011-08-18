/**************
FILE          : sslcommon.h
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
#ifndef SSL_COMMON_H
#define SSL_COMMON_H 1

#include <sys/resource.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qhash.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <kwallet.h>
#include <KAboutData>

#include "config.h"

//=========================================
// Defines...
//-----------------------------------------
#define qerror(msg)  handle_error(__FILE__, __LINE__, msg)
#define CIPHER_LIST "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"

//=========================================
// SSL callback Functions...
//-----------------------------------------
int verify_callback (int, X509_STORE_CTX*);
void handle_error (const char*, int , const char*);
DH* tmp_dh_callback (SSL*,int, int);
void init_dhparams (void);
int passwd_cb (char*,int,int,void*);

//=========================================
// Class Headers...
//-----------------------------------------
class SSLCommon : public QObject {
	public:
	SSLCommon ( QObject* = 0);

	public:
	void init_OpenSSL (void);
	void seed_prng    (void);
};

#endif
