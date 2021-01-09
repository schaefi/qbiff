#include "server.h"

//=========================================
// Constructor
//-----------------------------------------
Server::Server(void) {
    struct passwd *pw = getpwuid(getuid());
    mParse = new Parser (QString(pw->pw_dir)+"/.qbiffrc");
}

//=========================================
// getConnectionHandler
//-----------------------------------------
SSLInit* Server::getConnectionHandler(void) {
    return mConnectionHandler;
}

//=========================================
// run(thread)
//-----------------------------------------
void Server::run(void) {
    mNotify = new Notify(mParse);
    mConnectionHandler = new SSLInit(mNotify);
    mConnectionHandler->openConnection();
}
