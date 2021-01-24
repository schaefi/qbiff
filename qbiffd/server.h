#ifndef SERVER_H
#define SERVER_H 1

#include <sys/types.h>
#include <pwd.h>
#include <dirent.h>
#include <qthread.h>

#include "parser.h"
#include "notify.h"
#include "sslinit.h"
#include "folder.h"

//=========================================
// Class ServerHandler
//-----------------------------------------
class Server: public QThread {
    Q_OBJECT

    public:
    Server(void);

    protected:
    void run(void) override;

    private:
    Notify* mNotify;
    Parser* mParse;
    SSLInit* mConnectionHandler;

    public:
    SSLInit* getConnectionHandler(void);
    Parser* getFolderConfig(void);
};

#endif
