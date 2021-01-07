/**************
FILE          : xbox.h
***************
PROJECT       : SaX2 - SuSE advanced X configuration
              :
AUTHOR        : Marcus Schäfer <ms@suse.de>
              :
BELONGS TO    : configuration tool for the X window system 
              : released under the GPL license
              :
DESCRIPTION   : xbox wrapper class for QMessageBox
              : to create a frame around the dialog because
              : YaST2 does not provide a Windowmanager
              :
STATUS        : Status: Up-to-date
**************/
#ifndef CLIENTINFO_H
#define CLIENTINFO_H 1

#include <QApplication>
#include <qlayout.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qtextstream.h>
#include <qpixmap.h>

#include "config.h"

//=========================================
// Class ClientInfo
//-----------------------------------------
class ClientInfo : public QWidget {
	Q_OBJECT

	private:
	QPixmap  mShape;
	QLabel*  mLabel;
	QString  mFolder;
	QTimer*  mTimer;
	QString  mTip;
    QWidget* mButton;
	int      mNewMailCount;

	public:
	ClientInfo (
		QString&,QWidget*,int, bool =false
	);

	public:
	void setTip (const QString&,const QString&,bool=true);
	void showTip (void);

	protected:
	virtual void showEvent (QShowEvent*) override;
	
	private slots:
	void timerDone (void);
};

#endif
