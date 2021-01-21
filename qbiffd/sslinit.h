#ifndef SSL_INIT_H
#define SSL_INIT_H 1

#include <qstringlist.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qhash.h>
#include <signal.h>
#include <qobject.h>
#include <qlist.h>

#include "sslcommon.h"
#include "sslconnection.h"
#include "config.h"
#include "folder.h"
#include "notify.h"

//=========================================
// Class SSLServerInit
//-----------------------------------------
class SSLInit : public SSLCommon {
    Q_OBJECT

    public:
    SSLInit(Notify*, QObject* = 0);

    private:
    BIO *acc;
    QList<SSLConnection*> mConnections;
    Notify* mNotify;

    private:
    SSL_CTX* setupServerCTX(void);

    public:
    void openConnection(void);
    QList<SSLConnection*> getConnections(void);
    void removeConnection(SSLConnection*);
    void shutdown(void);
};

#endif
