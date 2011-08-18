/**************
FILE          : clientinfo.cpp
***************
PROJECT       : QBiff - A qt based biff
              :
AUTHOR        : Marcus Schäfer <ms@suse.de>
              :
BELONGS TO    : new mail notification service 
              : 
              :
DESCRIPTION   : client info window
              :
              :
STATUS        : Status: Beta
**************/
#include "clientinfo.h"
#include <qpixmap.h>
#include <qbitmap.h>
#include <qdesktopwidget.h>

//============================================
// Globals...
//--------------------------------------------
extern QString PIXSHAPE;
extern QString PIXSHAPEBG;
extern QString PIXNEWMAIL; 
extern QString PIXNOMAIL;
extern QString PIXPUBL;
extern QString PIXPRIV;
extern QString myButtonFont;
extern int myButtonFontSize;

//============================================
// Constructor
//--------------------------------------------
ClientInfo::ClientInfo (
	QString &folder, QWidget* parent, int newcount, bool, Qt::WFlags
) : QWidget ( parent, Qt::Window | Qt::FramelessWindowHint ) {
	mShape = QPixmap (PIXSHAPE);
	QBoxLayout* layer1 = new QVBoxLayout ( this );
	QFrame* shapeFrame = new QFrame (this);
	#if QT_VERSION > 0x040100
	// works only with Qt >= 4.2
	QString style;
	QTextStream (&style)
		<< "border: 0px;"
		<< "background-color: rgb(250,250,210);"
		<< "background-image: url("
		<< PIXSHAPEBG
		<< ");";
	shapeFrame -> setStyleSheet ( style );
	#endif
	shapeFrame -> setFixedWidth  (mShape.width());
	shapeFrame -> setFixedHeight (mShape.height());
	mLabel  = new QLabel ( shapeFrame );
	mLabel -> setFont (QFont (myButtonFont, myButtonFontSize, QFont::Bold));
	mLabel -> setFrameStyle ( QFrame::Plain );
	mLabel -> setLineWidth ( 0 );
	layer1 -> addWidget ( shapeFrame );
	layer1 -> setMargin (0);
	mFolder = folder;
	mTimer = new QTimer ( this );
	mTimer -> setSingleShot ( true );
	connect (
		mTimer , SIGNAL (timeout   (void)),
		this   , SLOT   (timerDone (void))
	);
	mNewMailCount = newcount;
	setMask ( mShape.mask() );
}

//============================================
// set label text
//--------------------------------------------
void ClientInfo::setTip (
	const QString& newmail,const QString& curmail, bool showMe
) {
	QString pix;
	int newcount = newmail.toInt();
	if (mNewMailCount >= newcount) {
		showMe = false;
	}
	mNewMailCount = newcount;
	if (mNewMailCount > 0) {
		pix = PIXNEWMAIL;
	} else {
		pix = PIXNOMAIL;
	}
	int curcount = curmail.toInt();
	int allcount = newcount + curcount;
	QString allmail;
	allmail.sprintf ("%d",allcount);
	mTip = "";
	QTextStream (&mTip)
		<< "<table border=0 cellspacing=4 width=300>"
		<< "<tr>"
		<< "<td width=30></td>"
		<< "<th rowspan=2><img src=\"" << pix << "\"></th>"
		<< "<td><nobr><br>Folder: <b>" << mFolder
		<< " : " << newmail << "</b> new Mail(s)</nobr></td>"
		<< "</tr>"
		<< "<tr>"
		<< "<td width=30></td>"
		<< "<td><hr>Counting <b>" << allmail << "</b> mails</td>"
		<< "</tr>"
		<< "</table>";
	if (showMe) {
		mLabel -> setText ( mTip );
		QFontMetrics metrics ( font() );
		show();
		mTimer -> start ( 2000 );
	}
}

//============================================
// showTip
//--------------------------------------------
void ClientInfo::showTip (void) {
	mLabel -> setText ( mTip );
	QFontMetrics metrics ( font() );
	show();
	mTimer -> start ( 2000 );
}

//============================================
// Show event
//--------------------------------------------
void ClientInfo::showEvent ( QShowEvent* ) {
	QWidget* parent = parentWidget();
	QWidget* main   = parent -> parentWidget();
	int xo = main->x();
	int yo = main->y();
	int moveX = xo + parent->x();
	int moveY = yo + parent->y() + parent->height();
	if (moveY + height() > qApp->desktop()->height()) {
		moveY = yo - height();
	}
	if (moveX + width() > qApp->desktop()->width()) {
		moveX = xo - parent->x();
	}
	move ( moveX,moveY );
}

//============================================
// hide window
//--------------------------------------------
void ClientInfo::timerDone (void) {
	hide();
}
