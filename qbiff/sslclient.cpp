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

void* self = NULL;

void cleanup(int,siginfo_t*,void*);

//=========================================
// Constructor
//-----------------------------------------
SSLClient::SSLClient(void) {
    self = this;

    struct sigaction action;
    action.sa_sigaction = cleanup;
    sigaction (SIGHUP , &action , 0);
    sigaction (SIGINT , &action , 0);
    sigaction (SIGTERM, &action , 0);

    BIO *sbio = NULL;
    printf ("Initialize openssl\n");
    if ( ! SSL_library_init() ) {
        qerror ("OpenSSL initialization failed");
    }
    SSL_load_error_strings();
    RAND_load_file ("/dev/urandom", 1024);
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

    //int ofcmode = fcntl (mSocket,F_GETFL,0);
    //if (fcntl (mSocket,F_SETFL,ofcmode | O_NDELAY)) {
    //  qerror ("Couldn't make socket nonblocking");
    //}
    //FD_ZERO (&readFDs);
    printf ("SSL Connection created\n");
}

//=========================================
// read(thread)
//-----------------------------------------
void SSLClient::run(void) {
    while(1) {
        char buf[2];
        QString line;
        do {
            int r = SSL_read (ssl, buf, 1);
            switch (SSL_get_error(ssl,r)) {
                case SSL_ERROR_WANT_READ:
                    continue;
                case SSL_ERROR_SYSCALL:
                    qerror ("SSL Error: Premature close");
                    SSLFree();
                    return;
                default:
                    break;
            }
            if (buf[0] == '\n') {
                // qDebug("%s", line.toLatin1().data());
                emit gotLine(line);
            } else {
                line.append(buf[0]);
            }
        } while (SSL_pending(ssl));
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
        ctx, CLIENT_CERTFILE.toLatin1().data(), CADIR) != 1
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

//=========================================
// cleanup
//-----------------------------------------
void cleanup(int code,siginfo_t*,void*) {
    SSLClient* client = (SSLClient*)self;
    client->SSLFree();
    qDebug("End Client Session");
    exit (code);
}
