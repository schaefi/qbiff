/**************
FILE          : notify.c
***************
PROJECT       : QBiff - A qt based biff
              :
AUTHOR        : Marcus Schäfer <ms@suse.de>
              :
BELONGS TO    : new mail notification service 
              : 
              :
DESCRIPTION   : kernel F_NOTIFY support
              :
              :
STATUS        : Status: Beta
**************/
#include "notify.h"
#include "config.h"
#include "serverhandler.h"

//====================================
// Globals
//------------------------------------
sigset_t block_set;
int FD[3] = {-1,-1,-1};
void* self = NULL;

//====================================
// External Globals
//------------------------------------
extern ServerHandler* pServer;
extern QString myFolder;

//====================================
// Constructor
//------------------------------------
NotifyCount::NotifyCount (const QString& folder, const QPoint& count) {
	mFolder = new QString ( folder );
	mCount  = new QPoint  ( count );
}

//====================================
// getFolder
//------------------------------------
QString* NotifyCount::getFolder (void) {
	return mFolder;
}

//====================================
// getCount
//------------------------------------
QPoint* NotifyCount::getCount (void) {
	return mCount;
}

//====================================
// Constructor
//------------------------------------
Notify::Notify ( Parser* parse ) {
	mParse = parse; init();
	struct sigaction pending;
	pending.sa_sigaction = handlePendingEvent; 
	sigaction (SIGIO , &pending , 0);

	struct sigaction action;
	action.sa_sigaction = handleNotifyEvent;
	sigemptyset (&action.sa_mask);
	action.sa_flags = SA_SIGINFO;
	sigaction (SIGRTMIN + 0 , &action , 0);
	sigaction (SIGRTMIN + 1 , &action , 0);

	sigemptyset (&block_set);
	sigaddset (&block_set,SIGIO);
	for (int i=0;i<2;i++) {
		sigaddset (&block_set,SIGRTMIN + i);
	}
	self = this;
}

//====================================
// init
//------------------------------------
void Notify::init ( bool clean ) {
	sigprocmask(SIG_BLOCK, &block_set,0);
	QList<char*> mFolderNames = mParse -> folderList();
	if (clean) {
		printf ("________cleaning: pollable event occured\n");
		cleanActiveFolderNotification();
		fdatasync (STDOUT_FILENO);
	}
	QList<char*> subdir;
	subdir.append ((char*)"/new");
	subdir.append ((char*)"/cur");
	QListIterator<char*> it ( mFolderNames );
	int FDcount = 0;
	while (it.hasNext()) {
		QPoint* dirCount = 0;
		char* value = it.next();
		for (int i=0;i<subdir.count();i++) {
			if (i == 0) {
				dirCount = new QPoint;
			}
			int count = getFiles (
				myFolder + QString(value+QString(subdir.at(i))+"/*")
			);
			if (i == 0) {
				dirCount->setX (count);
			}
			if (i == 1) {
				dirCount->setY (count);
			}
			activateFolderNotification (
				value,subdir.at(i)
			);
			if (i == 1) {
				int start = FDcount;
				int ended = FDcount + 4;
				for (int n=start;n<ended;n++) {
					mNotifyCount.insert ( mFDs[n], dirCount );
				}
				FDcount = ended;
				NotifyCount* initial = new NotifyCount (
					value,*dirCount
				);
				mInitialFolderList.append (initial);
			}
		}
	}
	sigprocmask(SIG_UNBLOCK, &block_set,0);
}

//=========================================
// activateFolderNotification
//-----------------------------------------
void Notify::activateFolderNotification (
	const QString& folderName, const QString& subDir
) {
	for (int n=0;n<2;n++) {
		QString fname (myFolder + folderName+subDir);
		int fd = open (
			fname.toLatin1().data(),
			O_RDONLY
		);
		if (fd == -1) {
			return;
		}
		fcntl (fd, F_SETSIG, SIGRTMIN + n);
		long flags = 0;
		switch (n) {
			case 0:
				flags = DN_MULTISHOT | DN_CREATE;
			break;
			case 1:
				flags = DN_MULTISHOT | DN_DELETE;
			break;
			default:
			break;
		}
		if (fcntl (fd,F_NOTIFY, flags) == -1) {
			return;
		}
		QString* folder = new QString (
			folderName+subDir
		);
		mNotifyDirs.insert (
			fd, folder
		);
		mFDs << fd;
	}
}

//=========================================
// cleanActiveFolderNotification
//-----------------------------------------
void Notify::cleanActiveFolderNotification (void) {
	for (int value = 0; value < mFDs.size(); value++) {
		fcntl (value, F_NOTIFY, 0);
		fcntl (value, F_SETSIG, 0);
		close (value);
	}
	mInitialFolderList.clear();
	mNotifyDirs.clear();
	mNotifyCount.clear();
	mFDs.clear();
}

//=========================================
// count files
//-----------------------------------------
int Notify::getFiles (const QString& pattern) {
	int count = 0;
	glob_t globbuf;
	if (glob (pattern.toLatin1().data(), GLOB_MARK, 0, &globbuf) == 0) {
		count = globbuf.gl_pathc;
		#if 0
		for (unsigned int n=0;n< globbuf.gl_pathc;n++) {
			printf ("%s\n",globbuf.gl_pathv[n]);
		}
		#endif
	}
	globfree (&globbuf);
	return count;
}

//=========================================
// Member call for handleNotificationEvent
//-----------------------------------------
bool Notify::sendSignal (int fd,int flag) {
	sigprocmask(SIG_BLOCK, &block_set,0);
	if ( mNotifyDirs[fd] ) {
		QString* pFolder = mNotifyDirs[fd];
		QPoint*  count   = mNotifyCount[fd];
		QStringList tokens = pFolder->split ( "/" );
		QString folder  = tokens.first();
		QString dirname = tokens.last();
		switch (flag) {
			case QBIFF_CREATE:
				printf ("________create %s %p\n",pFolder->toLatin1().data(),count);
				if (dirname == "new") {
					count -> rx()++;
				} else {
					count -> ry()++;
				}
                fdatasync (STDOUT_FILENO);
                sigNotify (folder, count);
			break;
			case QBIFF_DELETE:
				printf ("________delete %s %p\n",pFolder->toLatin1().data(),count);
				if (dirname == "new") {
					count -> rx()--;
				} else {
					count -> ry()--;
				}
                fdatasync (STDOUT_FILENO);
                sigNotify (folder, count);
			break;
			default:
			break;
		}
		sigprocmask(SIG_UNBLOCK, &block_set,0);
		return true;
	}
	sigprocmask(SIG_UNBLOCK, &block_set,0);
	return false;
}

//=========================================
// getInitialFolderList
//-----------------------------------------
QList<NotifyCount*> Notify::getInitialFolderList (void) {
	return mInitialFolderList;
}

//=========================================
// enqueue
//-----------------------------------------
void Notify::enqueue (int fd, int fl) {
	mNotifyQueue.insert (fd,fl);
}

//=========================================
// Real time signal arrived
//-----------------------------------------
void handleNotifyEvent ( int s, siginfo_t* si , void* ) {
	Notify* obj = (Notify*)self;
	if (s == SIGRTMIN) {
		// Real time signal SIGRTMIN0 arrived
		// obj -> enqueue (si->si_fd,QBIFF_CREATE);
		obj -> sendSignal (si->si_fd, QBIFF_CREATE);
	} else {
		// Real time signal SIGRTMIN1 arrived
		//obj -> enqueue (si->si_fd,QBIFF_DELETE);
		obj -> sendSignal (si->si_fd, QBIFF_DELETE);
	}
}

//=========================================
// Pollable event occured (SIGIO)
//-----------------------------------------
void handlePendingEvent ( int, siginfo_t* , void* ) {
	// ... /
	// RT signal queue is full, the result is a SIGIO and we
	// need to check all notify-directories
	// ----
	if (pServer) {
		pServer -> poll();
	}
}

//=========================================
// timerDone
//-----------------------------------------
void Notify::timerDone (void) {
	// .../
	// every time the timer expires this method is called.
	// the function will send a signal including the file
	// descriptor of the touched file and the flag whether this
	// file was created or deleted
	// ----
	QHashIterator<int, int> it (mNotifyQueue);
	while (it.hasNext()) {
		it.next();
		int fd   = it.key();
		int flag = it.value();
		sendSignal (fd,flag);
	}
	mNotifyQueue.clear();
}
