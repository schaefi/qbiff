/**************
FILE          : sslclient.cpp
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
#include "sslclient.h"

//=========================================
// Globals
//-----------------------------------------
extern QString serverName;
extern QString CAFILE;
extern QString DH1024;
extern QString DH512;
extern QString CLIENT_CERTFILE;
extern int serverPort;

//=========================================
// Constructor
//-----------------------------------------
SSLClient::SSLClient ( QObject* parent ) : SSLCommon ( parent ) {
	BIO *sbio = NULL;
	printf ("Initialize openssl\n");
	init_OpenSSL ();
	seed_prng ();
	printf ("Setup client context\n");
	setupClientCTX ();
	printf ("Connecting TCP socket\n");
	connectTCP ();

	printf ("Creating new SSL object\n");
	ssl = SSL_new (ctx);
	printf ("Creating new SSL BIO socket: %d\n",mSocket);
	sbio= BIO_new_socket (mSocket,BIO_NOCLOSE);
	printf ("Setup SSL BIO socket\n");
	SSL_set_bio (ssl,sbio,sbio);
	printf ("Connecting SSL socket\n");
	if (SSL_connect(ssl) <=0) {
		qerror ("Error creating connection BIO");
	}
	int ofcmode = fcntl (mSocket,F_GETFL,0);
	if (fcntl (mSocket,F_SETFL,ofcmode | O_NDELAY)) {
		qerror ("Couldn't make socket nonblocking");
	}
	FD_ZERO (&readFDs);
	printf ("SSL Connection created\n");
}

//=========================================
// clientReadWrite
//-----------------------------------------
void SSLClient::clientReadWrite ( void ) {
	FD_SET (mSocket,&readFDs);
	int width = mSocket + 1;
	while (1) {
		//struct timeval timeout;
		//timeout.tv_sec  = 0;
		//timeout.tv_usec = 1000;
		select (width,&readFDs,0,0,0);
		char buf[2];
		QString line;
		if (FD_ISSET (mSocket,&readFDs)) {
			do {
				int r = SSL_read (ssl,buf,1);
				switch (SSL_get_error(ssl,r)) {
				case SSL_ERROR_NONE:
					break;
				case SSL_ERROR_WANT_READ:
					continue;
					break;
				case SSL_ERROR_ZERO_RETURN:
					continue;
					break;
				case SSL_ERROR_SYSCALL:
					qerror ("SSL Error: Premature close");
					break;
				default:
					continue;
					break;
				}
				if (buf[0] == '\n') {
					gotLine (line);
				} else {
					line.append(buf[0]);
				}
			} while (SSL_pending(ssl));
		}
	}
}

//=========================================
// writeClient
//-----------------------------------------
void SSLClient::writeClient ( const QString & data ) {
	int err = 0;
	QString stream (data + "\n");
	char buf[stream.length()+1];
	memset (&buf, '\0', sizeof(buf));
	strncpy (buf,stream.toLatin1().data(),stream.length());
	for (unsigned int nwritten = 0;nwritten < sizeof(buf); nwritten += err) {
		err = SSL_write(ssl,buf + nwritten, sizeof(buf) - nwritten);
		switch (SSL_get_error(ssl,err)) {
		case SSL_ERROR_NONE:
			break;
		case SSL_ERROR_WANT_WRITE:
			continue;
		case SSL_ERROR_ZERO_RETURN:
			return;
		case SSL_ERROR_SYSCALL:
            qerror ("SSL Error: Premature close");
			return;
		default:
			return;
		}
	}
}

//=========================================
// connectTCP
//-----------------------------------------
void SSLClient::connectTCP (void) {
	QString port;
	QTextStream (&port) << serverPort;
	struct addrinfo hints;
	struct addrinfo *res=NULL;
	struct addrinfo *rp=NULL;
	int err;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol=0;
	err = getaddrinfo (
		serverName.toLatin1().data(), port.toLatin1().data(), &hints, &res
	);
	if (err != 0) {
		qerror ("Couldn't resolve host");
	}
	for (rp = res; rp != NULL; rp = rp->ai_next) {
		mSocket = socket(rp->ai_family,rp->ai_socktype,rp->ai_protocol);
		if (mSocket == -1) {
			continue;
		}
		if (::connect(mSocket,rp->ai_addr,rp->ai_addrlen) != -1) {
			break; // success
		}
		close(mSocket);
	}
	if (rp == NULL) {
		qerror ("Couldn't connect host");
	}
}

//=========================================
// setupClientCTX
//-----------------------------------------
void SSLClient::setupClientCTX (void) {
	ctx = SSL_CTX_new(SSLv23_method(  ));
	SSL_CTX_set_default_passwd_cb (ctx, passwd_cb);
	OpenSSL_add_all_algorithms();
	if (SSL_CTX_load_verify_locations(
		ctx, CAFILE.toLatin1().data(), CADIR) != 1
	) {
		qerror("Error loading CA file and/or directory");
	}
	if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
		qerror("Error loading default CA file and/or directory");
	}
	if (SSL_CTX_use_certificate_chain_file(
		ctx, CLIENT_CERTFILE.toLatin1().data()) != 1
	) {
		qerror("Error loading certificate from file");
	}
	if (SSL_CTX_use_PrivateKey_file(
		ctx,CLIENT_CERTFILE.toLatin1().data(),SSL_FILETYPE_PEM
	) != 1) {
		qerror("Error loading private key from file");
	}
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_callback);
	SSL_CTX_set_verify_depth(ctx, 4);
	SSL_CTX_set_options(ctx, SSL_OP_ALL|SSL_OP_NO_SSLv2);
	if (SSL_CTX_set_cipher_list(ctx, CIPHER_LIST) != 1) {
		qerror("Error setting cipher list (no valid ciphers)");
	}
}

//=========================================
// SSLFree
//-----------------------------------------
void SSLClient::SSLFree ( void ) {
	SSL_shutdown (ssl);
	SSL_free (ssl);
	SSL_CTX_free (ctx);
}
