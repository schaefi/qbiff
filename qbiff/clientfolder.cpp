#include "clientfolder.h"

//=========================================
// Globals
//-----------------------------------------
extern QString mailClient;
extern QString mailPrivate;
extern QString PIXPUBL;
extern QString PIXPRIV;
extern QString myFolder;

//=========================================
// Constructor
//-----------------------------------------
ClientFolder::ClientFolder (Qt::WindowFlags wflags) : QWidget (0,wflags)  {
    mMainFrame = new QFrame (this);
    mButtonBar = new QHBoxLayout (this);
    QString style;
    QTextStream (&style)
        << "border: 1px;"
        << "background-color: qlineargradient"
        << "( x1:0, y1:0, x2:0, y2:1, stop:0 lightgrey, stop:1 grey );";
    mMainFrame -> setFrameStyle ( QFrame::Plain );
    mMainFrame -> setStyleSheet ( style );
    setWindowFlags(Qt::FramelessWindowHint);
    mPrivatePixmap = QPixmap ( PIXPRIV );
    mPublicsPixmap = QPixmap ( PIXPUBL );
    mPrivate = new QPushButton ( mMainFrame );
    mPrivate -> setCheckable ( true );
    mPrivate -> setIcon(QIcon(mPublicsPixmap));
    mPrivate -> setIconSize(mPublicsPixmap.size());
    mPrivate -> setFixedWidth  (mPublicsPixmap.width()  + 20);
    mPrivate -> setFixedHeight (mPublicsPixmap.height() + 10);
    mPrivate -> setAutoFillBackground(true);

    QTextStream (&mStyleBlue)
        << "color: white;"
        << "border: 1px solid #8f8f91;"
        << "background-color: qlineargradient"
        << "( x1:0, y1:0, x2:0, y2:1, stop:0 #3c3c99, stop:1 #3c3c99 );";
    
    QTextStream (&mStyleGreen)
        << "color: white;"
        << "border: 1px solid #8f8f91;"
        << "background-color: qlineargradient"
        << "( x1:0, y1:0, x2:0, y2:1, stop:0 #4ba44b, stop:1 #4ba44b );";

    QTextStream (&mStyleRed)
        << "color: white;"
        << "border: 1px solid #8f8f91;"
        << "background-color: qlineargradient"
        << "( x1:0, y1:0, x2:0, y2:1, stop:0 #915c5c, stop:1 #915c5c );";

    mPrivate -> setFocusPolicy (Qt::NoFocus);
    mPrivate -> setStyleSheet( mStyleRed );
    mPrivate -> update();

    mButtonBar -> addWidget ( mPrivate );
    mButtonBar -> setSpacing (0);
    mButtonBar -> setMargin (0);
    mIsPrivate = false;
    mHeight = 0;
    connect (
        mPrivate, SIGNAL(toggled(bool)),
        this, SLOT(gotToggled(bool))
    );
    mClient = new SSLClient();
    connect (
        mClient, SIGNAL(gotLine(QString)),
        this, SLOT(gotLine(QString))
    );
    mClient -> start();
}

//=========================================
// gotLine
//-----------------------------------------
void ClientFolder::gotLine ( QString line ) {
    if (line == "INIT_DONE") {
        // .../
        // after init is done, move the button bar to
        // the bottem left corner and show the complete
        // window now.
        // ----
        move (0,qApp->desktop()->height() - mHeight);
        show();
        return;
    }
    QStringList tokens = QString::fromLocal8Bit(
        line.toLatin1().data()
    ).split(":");
    QString folder = tokens[0];
    QString status = tokens[1];
    QString newmail= tokens[2];
    QString curmail= tokens[3];
    if (folder.isEmpty()) {
        return;
    }
    if (! mButton.contains(folder)) {
        Button* btn = new Button (folder, mMainFrame);
        ClientInfo* info = new ClientInfo (folder, btn, newmail.toInt());
        btn->setStyleSheet( mStyleBlue );
        btn->setFocusPolicy (Qt::NoFocus);
        btn->setMinimumHeight (mPrivate->height());
        btn->update();
        QObject::connect (
            btn , SIGNAL ( clickedButton (QPushButton*) ),
            this, SLOT   ( folderEvent   (QPushButton*) )
        );
        QObject::connect (
            btn , SIGNAL ( showTip (QPushButton*) ),
            this, SLOT   ( showTip (QPushButton*) )
        );
        QObject::connect (
            btn , SIGNAL ( hideTip (QPushButton*) ),
            this, SLOT   ( hideTip (QPushButton*) )
        );
        mButtonBar -> addWidget ( btn );
        btn->setHidden (false);
        mButton.insert (folder,btn);
        mInfo.insert (folder,info);
        mFolderNames.append (folder.toLatin1().data());

        if ( status == "new" ) {
            btn->setStyleSheet( mStyleGreen );
            btn->update();
        }
        if ( status == "empty" ) {
            btn->setHidden (true);
            btn->update();
        }
        resize (sizeHint()); 
        mMainFrame -> resize (sizeHint());
        mHeight = height();
        mInfo[folder]->setTip (newmail,curmail,false);
    } else {
        if (mButton[folder]->getStatus() != status) {
            mButton[folder]->setStatus (status);
            if ( status == "new" ) {
                mButton[folder]->setHidden (false);
                mButton[folder]->setStyleSheet( mStyleGreen );
            }
            if ( status == "empty" ) {
                mButton[folder]->setHidden (true);
            }
            if ( status == "uptodate" ) {
                mButton[folder]->setHidden (false);
                mButton[folder]->setStyleSheet( mStyleBlue );
            }
            mButton[folder]->update();
            resize (sizeHint());
            mMainFrame -> resize (sizeHint());
            mHeight = height();
        }
        mInfo[folder]->setTip (newmail,curmail);
    }
    QTimer* mTimer = new QTimer ( this );
    connect (
        mTimer , SIGNAL (timeout   (void)),
        this   , SLOT   (timerDone (void))
    );
    mTimer->setSingleShot (true);
    mTimer->start ( 50 );
}

//=========================================
// timerDone
//-----------------------------------------
void ClientFolder::timerDone (void) {
    resize (sizeHint());
}

//=========================================
// showTip
//-----------------------------------------
void ClientFolder::showTip (QPushButton* btn) {
    QString folder = btn->text();
    mInfo[folder]->showTip();
}

//=========================================
// hideTip
//-----------------------------------------
void ClientFolder::hideTip (QPushButton* btn) {
    QString folder = btn->text();
    mInfo[folder]->hide();
}

//=========================================
// folderEvent
//-----------------------------------------
void ClientFolder::folderEvent (QPushButton* btn) {
    btn->setDisabled (true);
    QString text = btn->text();
    QProcess* proc = new QProcess();
    QString program;
    QStringList arguments;
    if (mIsPrivate) {
        program = mailPrivate;
    } else {
        program = mailClient;
    }
    arguments <<  QString(myFolder + text);
    connect (
        proc , SIGNAL (finished    ( int,QProcess::ExitStatus )),
        this , SLOT   (gotFinished ( int,QProcess::ExitStatus ))
    );
    mProcessList.append (proc);
    mButtonsList.append ((Button*)btn);
    proc->start (program,arguments);
}

//=========================================
// setToggle
//-----------------------------------------
void ClientFolder::setToggle ( bool toggle ) {
    if (toggle) {
        mPrivate -> show();
    } else {
        mPrivate -> hide();
    }
}

//=========================================
// gotFinished 
//-----------------------------------------
void ClientFolder::gotFinished ( int,QProcess::ExitStatus ) {
    int pCount = 0;
    int pRemove[mProcessList.count()];
    QListIterator<QProcess*> it (mProcessList);
    while (it.hasNext()) {
        pRemove[pCount] = 0;
        QProcess *proc = it.next();
        if (proc->pid() == 0) {
            mButtonsList.at(pCount)->setDisabled (false);
            pRemove[pCount] = 1;
        }
        pCount++;
    }
    for (int n=0;n<mProcessList.count();n++) {
        if (pRemove[n] == 1) {
            mButtonsList.removeAt (n);
            mProcessList.removeAt (n);
        }
    }
}

//=========================================
// gotToggled
//-----------------------------------------
void ClientFolder::gotToggled (bool on) {
    if (! on) {
        mPrivate -> setIcon (QIcon(mPublicsPixmap));
        mPrivate -> setIconSize (mPublicsPixmap.size());
        mIsPrivate = false;
    } else {
        mPrivate -> setIcon(QIcon(mPrivatePixmap));
        mPrivate -> setIconSize(mPrivatePixmap.size());
        mIsPrivate = true;
    }
}

//=========================================
// getHeight
//-----------------------------------------
int ClientFolder::getHeight (void) {
    return mHeight;
}
