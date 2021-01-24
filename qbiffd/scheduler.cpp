#include "scheduler.h"
#include "sslconnection.h"
#include "sslinit.h"

Scheduler::Scheduler(Server* server) {
    mServer = server;
    QTimer* timer = new QTimer(this);
    timer->connect(
        timer, SIGNAL(timeout()),
        this, SLOT(timerDone())
    );
    timer->start(1000);
}

void Scheduler::timerDone(void) {
    SSLInit* connection_handler = mServer->getConnectionHandler();
    if (connection_handler) {
        QList<SSLConnection*> ssl_connections = \
            connection_handler->getConnections();
        SSLConnection* ssl_connection = NULL;
        QMutableListIterator<SSLConnection*> it (ssl_connections);
        while (it.hasNext()) {
            ssl_connection = it.next();
            //qDebug("Active connection %p\n", ssl_connection);
            if (ssl_connection->sendFolderList(true) == false) {
                //qDebug("Deleting connection %p\n", ssl_connection);
                connection_handler->removeConnection(ssl_connection);
            }
        }
        if (ssl_connection) {
            ssl_connection->resetFolderChanges();
        }
    }
}