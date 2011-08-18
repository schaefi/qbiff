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

#include <KApplication>
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
	int      mNewMailCount;

	public:
	ClientInfo (
		QString&,QWidget*,int,
		bool =false,Qt::WFlags = Qt::FramelessWindowHint
	);

	public:
	void setTip (const QString&,const QString&,bool=true);
	void showTip (void);

	protected:
	virtual void showEvent   ( QShowEvent * );
	
	private slots:
	void timerDone (void);
};

#endif
