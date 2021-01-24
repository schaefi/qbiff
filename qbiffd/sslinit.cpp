#include "sslinit.h"

//=========================================
// Globals...
//-----------------------------------------
extern QString CAFILE;
extern QString SERVER_CERTFILE;
extern int serverPort;

void* self_init = NULL;

void cleanup(int,siginfo_t*,void*);

//=========================================
// Constructor...
//-----------------------------------------
SSLInit::SSLInit (
    Notify* notify, QObject* parent
): SSLCommon(parent) {
    self_init = this;

    struct sigaction action;
    action.sa_sigaction = cleanup;
    sigaction (SIGHUP , &action , 0);
    sigaction (SIGINT , &action , 0);
    sigaction (SIGTERM, &action , 0);

    QString port;
    QTextStream (&port) << serverPort;
    mNotify = notify;
	init_OpenSSL();
	seed_prng();
	acc = BIO_new_accept ((char*)port.toLatin1().data());
	if (!acc) {
		qerror("Error creating server socket");
	}
	if (BIO_do_accept(acc) <= 0) {
		qerror("Error binding server socket");
	}
}

//=========================================
// getConnections
//-----------------------------------------
QList<SSLConnection*> SSLInit::getConnections(void) {
    return mConnections;
}

//=========================================
// removeConnection
//-----------------------------------------
void SSLInit::removeConnection(SSLConnection* to_delete) {
    QMutableListIterator<SSLConnection*> it (mConnections);
    while (it.hasNext()) {
        SSLConnection* ssl_connection = it.next();
        if (ssl_connection == to_delete) {
            ssl_connection->shutdown();
            it.remove();
            break;
        }
    }
}

//=========================================
// Connection loop(blocking)...
//-----------------------------------------
void SSLInit::openConnection (void) {
    while (1) {
        SSL* ssl = NULL;
        printf("Accepting connections...\n");
        if (BIO_do_accept(acc) <= 0) {
            break;
        }
        BIO* client = BIO_pop(acc);
        SSL_CTX* ctx = setupServerCTX();
        if (!(ssl = SSL_new(ctx))) {
            qDebug("Error creating new SSL context");
            SSL_free (ssl);
            break;
        }
        SSL_set_accept_state(ssl);
        SSL_set_bio(ssl, client, client);
        if (SSL_accept(ssl) <= 0) {
            qDebug("Error accepting SSL connection");
            SSL_free (ssl);
            break;
        }
        qDebug("SSL Connection opened");
        SSLConnection* connection = new SSLConnection(ssl, client, mNotify);
        mConnections.append(connection);
        connection->start();
    }
}

//=========================================
// Certificate setup...
//-----------------------------------------
SSL_CTX* SSLInit::setupServerCTX(void) {
    SSL_CTX* ctx = SSL_CTX_new(SSLv23_method());
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
        ctx, SERVER_CERTFILE.toLatin1().data()) != 1
    ) {
        qerror("Error loading certificate from file");
    }
    if (SSL_CTX_use_PrivateKey_file (
        ctx,SERVER_CERTFILE.toLatin1().data(),SSL_FILETYPE_PEM
    ) != 1) {
        qerror("Error loading private key from file");
    }
    SSL_CTX_set_verify (
        ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
        verify_callback
    );
    SSL_CTX_set_verify_depth(ctx, 4); 
    SSL_CTX_set_options(
        ctx, SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_SINGLE_DH_USE
    );
    SSL_CTX_set_tmp_dh_callback(ctx, tmp_dh_callback);
    if (SSL_CTX_set_cipher_list(ctx, CIPHER_LIST) != 1) {
        qerror("Error setting cipher list (no valid ciphers)");
    }
    return ctx;
}

//=========================================
// shutdown...
//-----------------------------------------
void SSLInit::shutdown(void) {
    QMutableListIterator<SSLConnection*> it (mConnections);
    while (it.hasNext()) {
        SSLConnection* ssl_connection = it.next();
        ssl_connection->shutdown();
    }
    if (acc) {
        int socket_fd = BIO_get_fd(acc, NULL);
        if (socket_fd) {
            close(socket_fd);
        }
    }
}

//=========================================
// cleanup
//-----------------------------------------
void cleanup(int code, siginfo_t*,void*) {
    SSLInit* ssl_init = (SSLInit*)self_init;
    ssl_init->shutdown();
    qDebug("End Server Session");
    exit(code);
}
