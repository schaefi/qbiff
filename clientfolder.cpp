/**************
FILE          : clientfolder.cpp
***************
PROJECT       : QBiff - A qt based biff
              :
AUTHOR        : Marcus Schäfer <ms@suse.de>
              :
BELONGS TO    : new mail notification service 
              : 
              :
DESCRIPTION   : implementation class for a client folder
              : connected to a TCP port
              :
              :
STATUS        : Status: Beta
**************/
#include "clientfolder.h"

//=========================================
// Globals
//-----------------------------------------
extern QString mailClient;
extern QString mailPrivate;
extern QString PIXPUBL;
extern QString PIXPRIV;
extern QString myFolder;

static ClientFolder* pClientFolder;

//=========================================
// Constructor
//-----------------------------------------
ClientFolder::ClientFolder (Qt::WindowFlags wflags) : QWidget (0,wflags)  {
	pClientFolder = this;
	mMainFrame = new QFrame (this);
	mButtonBar = new QHBoxLayout (this);
	#if QT_VERSION > 0x040100
	QString style;
	QTextStream (&style)
		<< "border: 1px;"
		<< "background-color: qlineargradient"
		<< "( x1:0, y1:0, x2:0, y2:1, stop:0 lightgrey, stop:1 grey );";
	mMainFrame -> setFrameStyle ( QFrame::Plain );
	mMainFrame -> setStyleSheet ( style );
	#endif
	mPrivatePixmap = QPixmap ( PIXPRIV );
	mPublicsPixmap = QPixmap ( PIXPUBL );
	mPrivate = new QPushButton ( mMainFrame );
	mPrivate -> setCheckable ( true );
	mPrivate -> setIcon(QIcon(mPublicsPixmap));
	mPrivate -> setIconSize(mPublicsPixmap.size());
	mPrivate -> setFixedWidth  (mPublicsPixmap.width()  + 20);
	mPrivate -> setFixedHeight (mPublicsPixmap.height() + 10);
	mPDefault = mPrivate -> palette();
	mPBlue = mPDefault;
	mPBlue.setColor( QPalette::Button, QColor(60,60,153));
	mPBlue.setColor( QPalette::ButtonText, QColor(255,255,255));
	mPGreen = mPDefault;
	mPGreen.setColor( QPalette::Button, QColor(75,164,75));
	mPGreen.setColor( QPalette::ButtonText, QColor(255,255,255));
	mPRed = mPDefault;
	mPRed.setColor( QPalette::Button, QColor(145,92,92));
	mPrivate -> setFocusPolicy (Qt::NoFocus);
	mPrivate -> setPalette (mPRed);
	mButtonBar -> addWidget ( mPrivate );
	mButtonBar -> setSpacing (0);
	mButtonBar -> setMargin (0);
	mIsPrivate = false;
	mHeight = 0;
	connect (
		mPrivate , SIGNAL (toggled    ( bool )),
		this     , SLOT   (gotToggled ( bool ))
	);
	mClient = new ClientInit();
	connect (
        mClient , SIGNAL (gotEvent ( QString )),
        this    , SLOT   (gotLine  ( QString ))
    );
	mClient -> start();
	hide();
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
		Button* btn = new Button (folder,mMainFrame);
		ClientInfo* info = new ClientInfo (folder,btn,newmail.toInt());
		btn->setPalette (mPBlue);
		btn->setFocusPolicy (Qt::NoFocus);
		btn->setMinimumHeight (mPrivate->height());
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
			btn->setPalette (mPGreen);
		}
		if ( status == "empty" ) {
			btn->setHidden (true);
		}
		mInfo[folder]->setTip (newmail,curmail,false);
	} else {
		mButton[folder]->setStatus (status);
		if ( status == "new" ) {
			mButton[folder]->setHidden (false);
			mButton[folder]->setPalette (mPDefault);
			mButton[folder]->setPalette (mPGreen);
		}
		if ( status == "empty" ) {
			mButton[folder]->setHidden (true);
		}
		if ( status == "uptodate" ) {
			mButton[folder]->setHidden (false);
			mButton[folder]->setPalette (mPDefault);
			mButton[folder]->setPalette (mPBlue);
		}
		mInfo[folder]->setTip (newmail,curmail);
	}
	resize (sizeHint());
	mMainFrame -> resize (sizeHint());
	mHeight = height();
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
	if (mRemoteMail) {
		QString servercmd;
		QTextStream(&servercmd) << "WRITE " << text;
		mClient -> writeClient (servercmd);
	}
	QProcess* proc = new QProcess();
	QString program;
	QStringList arguments;
	if (mIsPrivate) {
		program = mailPrivate;
	} else {
		program = mailClient;
	}
	arguments <<  QString(myFolder + text);
	//printf ("%s %s\n",
	//	program.toLatin1().data(),arguments.join(":").toLatin1().data()
	//);
	connect (
		proc , SIGNAL (finished    ( int,QProcess::ExitStatus )),
		this , SLOT   (gotFinished ( int,QProcess::ExitStatus ))
	);
	mProcessList.append (proc);
	mButtonsList.append ((Button*)btn);
	proc->start (program,arguments);
}

//=========================================
// setRemoteMail
//-----------------------------------------
void ClientFolder::setRemoteMail ( bool arg ) {
	mRemoteMail = arg;
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
// End Session
//-----------------------------------------
void ClientFolder::cleanup (void) {
	mClient -> writeClient ("QUIT");
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
