#ifndef SSL_CONNECTION_H
#define SSL_CONNECTION_H 1

#include <qstringlist.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qhash.h>
#include <signal.h>
#include <qobject.h>
#include <qlist.h>
#include <qthread.h>
#include <qmutex.h>

#include "folder.h"
#include "sslcommon.h"
#include "notify.h"

//=========================================
// Class SSLConnection
//-----------------------------------------
class SSLConnection : public QThread {
    Q_OBJECT

    private:
    SSL* ssl;
    BIO* bio;
    Notify* mNotify;

    public:
    SSLConnection(SSL*, BIO*, Notify*);

    protected:
    void run(void) override;

    public:
    bool sendFolderList(bool);
    void resetFolderChanges(void);
    bool write(const QString&);
    void shutdown(void);
};

#endif
