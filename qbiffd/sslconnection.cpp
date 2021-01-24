#include "sslconnection.h"

//=========================================
// Constructor...
//-----------------------------------------
SSLConnection::SSLConnection (
    SSL* pSSL, BIO* pBIO, Notify* notify
) {
    ssl = pSSL;
    bio = pBIO;
    mNotify = notify;
}

//=========================================
// run(thread)
//-----------------------------------------
void SSLConnection::run(void) {
    QMutex mutex;
    mutex.lock();
    qDebug("Sending initial folder list");
    sendFolderList(false);
    write("INIT_DONE");
    mutex.unlock();
}

//=========================================
// sendFolderList
//-----------------------------------------
bool SSLConnection::sendFolderList(bool check_for_changes) {
    mNotify->setFolders();
    QList<Folder*> folder_list = mNotify->getFolderList();
    QListIterator<Folder*> it (folder_list);
    while (it.hasNext()) {
        Folder* folder = it.next();
        bool has_changes = true;
        if (check_for_changes) {
            has_changes = folder->hasChanges();
        }
        if (has_changes) {
            if (! write(folder->getStatus())) {
                return false;
            }
        }
    }
    if (! write("\n")) {
        // write empty line to keep the socket check active
        return false;
    }
    return true;
}

//=========================================
// resetFolderChanges
//-----------------------------------------
void SSLConnection::resetFolderChanges(void) {
    QList<Folder*> folder_list = mNotify->getFolderList();
    QListIterator<Folder*> it (folder_list);
    while (it.hasNext()) {
        Folder* folder = it.next();
        folder->resetChanges();
    }
}

//=========================================
// shutdown...
//-----------------------------------------
void SSLConnection::shutdown(void) {
    if (ssl) {
        qDebug("Shutdown connection");
        SSL_shutdown (ssl);
        SSL_free (ssl);
    }
}

//=========================================
// write
//-----------------------------------------
bool SSLConnection::write(const QString& data) {
    if (ssl) {
        //qDebug(
        //    "SSL_write(client): %s:%d",
        //    data.toLatin1().data(), data.length()
        //);
        QString stream(data + '\n');
        do {
            int num = SSL_write(ssl, stream.toLatin1().data(), stream.length());
            int err=SSL_get_error(ssl, num);
            switch(err) {
            case SSL_ERROR_NONE: // nothing was written: ignore
                break;
            case SSL_ERROR_WANT_WRITE: // retry
                break;
            case SSL_ERROR_WANT_READ: // retry
                break;
            case SSL_ERROR_SYSCALL: // socket error
                if (! num) {
                    SSL_set_shutdown(
                        ssl, SSL_SENT_SHUTDOWN|SSL_RECEIVED_SHUTDOWN
                    );
                    return false;
                }
                return false;
            case SSL_ERROR_ZERO_RETURN: // close_notify alert received
                break;
            case SSL_ERROR_SSL: // fatal
                qDebug("SSL_write issue");
                shutdown();
                return false;
            default: // fatal
                qDebug("SSL_write issue");
                shutdown();
                return false;
            }
        } while (SSL_pending(ssl));
    }
    return true;
}
