#include "server.h"

//=========================================
// Constructor
//-----------------------------------------
Server::Server(void) {
    struct passwd *pw = getpwuid(getuid());
    mParse = new Parser (QString(pw->pw_dir)+"/.qbiffrc");
    mWatcher = new QFileSystemWatcher;
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
    mNotify = new Notify(mParse, *mWatcher);
    mConnectionHandler = new SSLInit(mNotify);
    mConnectionHandler->openConnection();
}

//=========================================
// getFolderConfig
//-----------------------------------------
Parser* Server::getFolderConfig(void) {
    return mParse;
}

//=========================================
// getWatcher
//-----------------------------------------
QFileSystemWatcher* Server::getWatcher(void) {
    return mWatcher;
}
