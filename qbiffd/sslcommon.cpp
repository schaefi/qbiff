#include "sslcommon.h"

//=========================================
// Globals...
//-----------------------------------------
extern QString DH1024;
extern QString DH512;

//=========================================
// Globals...
//-----------------------------------------
extern QString DH1024;
extern QString DH512;
char* pPWCache = NULL;

//=========================================
// Globals...
//-----------------------------------------
DH *dh512  = NULL;
DH *dh1024 = NULL;

//=========================================
// SSL callback Functions...
//-----------------------------------------
int verify_callback (int ok, X509_STORE_CTX *store) {
    char data[256];
    if (!ok) {
        X509 *cert = X509_STORE_CTX_get_current_cert(store);
        int  depth = X509_STORE_CTX_get_error_depth(store);
        int  err = X509_STORE_CTX_get_error(store);

        fprintf(stderr, "-Error with certificate at depth: %i\n", depth);
        X509_NAME_oneline(X509_get_issuer_name(cert), data, 256);
        fprintf(stderr, "  issuer   = %s\n", data);
        X509_NAME_oneline(X509_get_subject_name(cert), data, 256);
        fprintf(stderr, "  subject  = %s\n", data);
        fprintf(stderr, "  err %i:%s\n",
            err, X509_verify_cert_error_string(err)
        );
    }
    return ok;
}

//=========================================
// passwd_cb...
//-----------------------------------------
int passwd_cb (char* buf, int size, int, void*) {
    if (! pPWCache) {
        char password[80];
        (void*) fgets (password, sizeof(password), stdin);
        password[strlen(password)-1]='\0';
        strncpy(buf, (char *)(password), size);
        buf[size - 1] = '\0';
        pPWCache = (char*)malloc(sizeof(char)*strlen(password)+1);
        memset ((char*)pPWCache, '\0', strlen(password)+1);
        strncpy (pPWCache, password, strlen(password));
    } else {
        strncpy (buf, pPWCache, strlen(pPWCache));
    }
    return (strlen(buf));
}

//=========================================
// init_dhparams...
//-----------------------------------------
void init_dhparams (void) {
    BIO *bio;
    bio = BIO_new_file(DH512.toLatin1().data(), "r");
    if (!bio) {
        qerror("Error opening file dh512.pem");
    }
    dh512 = PEM_read_bio_DHparams(bio, NULL, NULL, NULL);
    if (!dh512) {
        qerror("Error reading DH parameters from dh512.pem");
    }
    BIO_free(bio);

    bio = BIO_new_file(DH1024.toLatin1().data(), "r");
    if (!bio) {
        qerror("Error opening file dh1024.pem");
    }
    dh1024 = PEM_read_bio_DHparams(bio, NULL, NULL, NULL);
    if (!dh1024) {
        qerror("Error reading DH parameters from dh1024.pem");
    }
    BIO_free(bio);
}

//=========================================
// tmp_dh_callback...
//-----------------------------------------
DH* tmp_dh_callback (SSL*,int, int keylength) {
	DH *ret;
	if (!dh512 || !dh1024) {
		init_dhparams(  );
	}
	switch (keylength) {
	case 512:
		ret = dh512;
		break;
	case 1024:
	default:
		ret = dh1024;
		break;
	}
	return ret;
}

//=========================================
// handle_error...
//-----------------------------------------
void handle_error (const char *file, int lineno, const char *msg) {
	fprintf(stderr, "** %s:%i %s\n", file, lineno, msg);
	ERR_print_errors_fp(stderr);
	exit (1);
}

//=========================================
// SSLCommon member  Functions...
//-----------------------------------------
//=========================================
// Constructor
//-----------------------------------------
SSLCommon::SSLCommon ( QObject* ) {
    // ...
}

//=========================================
// init_OpenSSL...
//-----------------------------------------
void SSLCommon::init_OpenSSL (void) {
    if (! SSL_library_init()) {
        qerror ("OpenSSL initialization failed");
    }
    SSL_load_error_strings();
}

//=========================================
// seed_prng...
//-----------------------------------------
void SSLCommon::seed_prng (void) {
    RAND_load_file ("/dev/urandom", 1024);
}
