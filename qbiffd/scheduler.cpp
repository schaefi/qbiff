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
        QMutableListIterator<SSLConnection*> it (ssl_connections);
        while (it.hasNext()) {
            SSLConnection* ssl_connection = it.next();
            //qDebug("Active connection %p\n", ssl_connection);
            if (ssl_connection->sendFolderList() == false) {
                //qDebug("Deleting connection %p\n", ssl_connection);
                connection_handler->removeConnection(ssl_connection);
            }
        }
    }
}
