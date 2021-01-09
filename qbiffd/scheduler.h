#include <qtimer.h>

#include "server.h"

class Scheduler: public QObject {
    Q_OBJECT

    public:
    Scheduler(Server*);

    private:
    Server* mServer;

    private slots:
    void timerDone(void);
};
