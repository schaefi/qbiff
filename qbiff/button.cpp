/**************
FILE          : button.cpp
***************
PROJECT       : QBiff - A qt based biff
              :
AUTHOR        : Marcus Schäfer <ms@suse.de>
              :
BELONGS TO    : new mail notification service 
              : 
              :
DESCRIPTION   : implementation for button widget
              :
              :
STATUS        : Status: Beta
**************/
#include "button.h"

//=========================================
// Globals...
//-----------------------------------------
extern QString PIXNEWMAIL;
extern QString PIXNOMAIL; 
extern QString myButtonFont;
extern int myButtonFontSize;

//=========================================
// Constructor
//-----------------------------------------
Button::Button(
    const QString &text, QWidget*parent 
): QPushButton (text,parent) {
    QObject::connect (
        this , SIGNAL ( clicked     (void) ),
        this,  SLOT   ( slotClicked (void) )
    );
    mFolder = text;
    //setFont (QFont ("Dejavu Sans", 10, QFont::Normal));
    setFont (QFont (myButtonFont, myButtonFontSize, QFont::Bold));
    mFontMetrics = new QFontMetrics(myButtonFont);
    setFixedWidth(mFontMetrics -> width(mFolder) + 20);
    mLastNewCount = 0;
    mTimer = new QTimer ( this );
    mTimer -> setSingleShot ( true );
    connect (
        mTimer , SIGNAL (timeout   (void)),
        this   , SLOT   (timerDone (void))
    );
    installEventFilter (this);
    mStatus = "unknown";
}

//=========================================
// slotClicked
//-----------------------------------------
void Button::slotClicked (void) {
    clickedButton (this);
}

//=========================================
// timerDone
//-----------------------------------------
void Button::timerDone (void) {
    showTip (this);
}

//=========================================
// eventFilter
//-----------------------------------------
bool Button::eventFilter ( QObject*, QEvent* event ) {
    QMouseEvent* mouse = (QMouseEvent*)event;
    if (! mouse) {
        return (false);
    }
    //printf ("%d\n",mouse->type());
    switch (mouse->type()) {
        case QEvent::HoverEnter:
            mTimer->start ( 2000 );
        break;
        case QEvent::HoverLeave:
            mTimer->stop();
            hideTip (this);
        break;
        default:
        break;
    }
    return (false);
}

//=========================================
// setStatus
//-----------------------------------------
void Button::setStatus (const QString& status) {
    mStatus = status;
}

//=========================================
// getStatus
//-----------------------------------------
QString Button::getStatus (void) {
    return mStatus;
}
