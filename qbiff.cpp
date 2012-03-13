/**************
FILE          : qbiff.cpp
***************
PROJECT       : QBiff - A qt based biff
              :
AUTHOR        : Marcus Schäfer <ms@suse.de>
              :
BELONGS TO    : new mail notification service 
              : 
              :
DESCRIPTION   : qbiff will create a buttonbar for different
              : folders and enables the button to call a 
              : mailreader for the selected folder
              :
              :
STATUS        : Status: Beta
**************/
#include <stdlib.h>
#include <signal.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qfile.h>
#include <getopt.h>
#include <qiodevice.h>
#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocalizedString>
#include <locale.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

#include "config.h"
#include "serverhandler.h"
#include "clientfolder.h"

//=========================================
// Globals
//-----------------------------------------
bool useGUI;
ClientFolder*  pFolder = NULL;
ServerHandler* pServer = NULL;
int  serverPort        = PORT;
QString serverName     = SERVER;
QString mailClient     = MY_MAILCLIENT;
QString mailPrivate    = MY_MAILCLPRIV;
QString baseDir        = BASEDIR;
QString myFolder       = MY_FOLDER;
QString myButtonFont   = "FrutigerNextLT:style=Bold";
QString user_name      = "nobody";
QString group_name     = "nogroup";
int myButtonFontSize   = 10;
KAboutData* about;

//=========================================
// Globals
//-----------------------------------------
QString CAFILE;
QString DH1024;
QString DH512;
QString SERVER_CERTFILE;
QString CLIENT_CERTFILE;
QString PIXSHAPE;
QString PIXSHAPEBG;
QString PIXNEWMAIL;
QString PIXNOMAIL;
QString PIXPUBL;
QString PIXPRIV;

//=========================================
// Global functions
//-----------------------------------------
void quit  (int,siginfo_t*,void*);
void usage (void);

//=========================================
// The magic :-)
//-----------------------------------------
int main(int argc,char*argv[]) {
	struct passwd *user;
	struct group *group;
	//=========================================
	// set locale
	//-----------------------------------------
	setlocale (LC_ALL,"");

	//=========================================
	// setup signal handler
	//-----------------------------------------
	struct sigaction action;
	action.sa_sigaction = quit;
	sigaction (SIGHUP , &action , 0);
	sigaction (SIGINT , &action , 0);
	sigaction (SIGTERM, &action , 0);

	//=========================================
	// about this program
	//-----------------------------------------
	about = new KAboutData (
		"qbiff",
		0,
		ki18n("qbiff"),
		"0.2.0",
		ki18n("QBiff mail notification (maildir)"),
		KAboutData::License_GPL,
		ki18n("(c) 2009 Marcus Schaefer"),
		ki18n("QBiff buttonbar, notify on new mail"),
		"http://www.isny.homelinux.com",
		"ms@suse.de"
	);

	//=========================================
	// create Qt application
	//-----------------------------------------
	KCmdLineArgs::init (argc, argv, about);

	KCmdLineOptions options;
	options.add("d").add("daemon",ki18n("Daemon/Server mode"));
	options.add("u").add("user",ki18n("Run with specified user privileges"));
	options.add("g").add("group",ki18n("Run with specified group privileges"));
	options.add("r").add("remote",ki18n("Remote Mail"));
	options.add("s").add("server <address>",ki18n("Server Address"));
	options.add("p").add("port <number>",ki18n("Port Number"));
	options.add("f").add("mailfolder <path>",ki18n("Mail Folder Path"));
	options.add("F").add("buttonfont <name>",ki18n("Button Font"));
	options.add("Z").add("buttonfontsize <size>",ki18n("Button Font Size"));
	options.add("m").add("readmail <program>",ki18n("Mail Program"));
	options.add("i").add("readpriv <program>",ki18n("Private Mail Program"));
	options.add("b").add("basedir <path>",ki18n("Base Path"),"/usr/share/qbiff");
	options.add("t").add("toggle",ki18n("Activate Toggle Button"));
	options.add("h",ki18n("Help"));

	KCmdLineArgs::addCmdLineOptions( options );
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	if (! args->isSet("daemon")) {
		useGUI = true;
	}
	KApplication app ( useGUI );
	
	//=========================================
	// init variables...
	//-----------------------------------------
	bool remoteMail     = false;
	bool haveToggle     = false;

	//=========================================
	// get options
	//-----------------------------------------
	if (args->isSet("h")) {
		usage();
	}
	if (args->isSet("toggle")) {
		haveToggle = true;
	}
	if (args->isSet("remote")) {
		remoteMail = true;
	}
	user_name  = args->getOption("user");
	group_name = args->getOption("group");
	myFolder   = args->getOption("mailfolder");
	myButtonFontSize = args->getOption("buttonfontsize").toInt();
	myButtonFont = args->getOption("buttonfont");
	serverName = args->getOption("server");
	serverPort = args->getOption("port").toInt();
	mailClient = args->getOption("readmail");
	mailPrivate= args->getOption("readpriv");
	baseDir = args->getOption("basedir");
	myFolder += "/";
	args->clear();
	//=========================================
	// release priviliedge
	//-----------------------------------------
	user  = getpwnam(user_name.toLatin1().data());
	group = getgrnam(group_name.toLatin1().data());
	if (!user || !group) {
		printf ("qbiff::can't change identity to %s.%s, exiting",
			user_name.toLatin1().data(), group_name.toLatin1().data()
		);
		exit(1);
	}
	//=======================================
	// certification stuff
	//---------------------------------------
	if (useGUI) {
		CAFILE = baseDir + "/cert-client/rootcert.pem";
	} else {
		CAFILE = baseDir + "/cert-server/rootcert.pem";
	}
	DH1024         = baseDir + "/cert-server/dh1024.pem";
	DH512          = baseDir + "/cert-server/dh512.pem";
	SERVER_CERTFILE= baseDir + "/cert-server/server.pem";
	CLIENT_CERTFILE= baseDir + "/cert-client/client.pem";

	// .../
	// shape pixmap used to create the bitmask for the non
	// rectangular info window
	// ----
	PIXSHAPE       = baseDir + "/pixmaps/shape.xpm";
	// .../
	// background image set as QFrame stylesheet. Alternative
	// shape.png.transparent can be used, which allows to set
	// the background color within the C++ code
	// ----
	PIXSHAPEBG     = baseDir + "/pixmaps/shape.xpm"; 

	PIXNEWMAIL     = baseDir + "/pixmaps/newmail.png";
	PIXNOMAIL      = baseDir + "/pixmaps/nomail.png";
	PIXPUBL        = baseDir + "/pixmaps/public.png";
	PIXPRIV        = baseDir + "/pixmaps/private.png";

	//=========================================
	// create entity Server or Client
	//-----------------------------------------
	if ( ! useGUI ) {
		QString pidfile;
		pidfile.sprintf ("/var/run/qbiffd/qbiff.%d.pid",serverPort);
		QFile run (pidfile);
		if (run.exists()) {
		if (run.open( QIODevice::ReadOnly )) {
			QTextStream stream ( &run );
			QString pid = stream.readLine();
			run.close();
			if (kill (pid.toInt(),0) == 0) {
				printf ("qbiff::already running: %s\n",pid.toLatin1().data());
				exit (0);
			} else {
				unlink (pidfile.toLatin1().data());
			}
		}
		}
		if ( ! run.open( QIODevice::WriteOnly ) ) {
			printf ("qbiff::couldn't open pid file\n");
			exit (1);
		}
		int pid = getpid();
		QTextStream stream( &run );
		stream << pid << "\n";
		run.close();
		setgid(group->gr_gid);
		setuid(user->pw_uid);
		pServer = new ServerHandler;
	} else {
		setgid(group->gr_gid);
		setuid(user->pw_uid);
		Qt::WFlags wflags = Qt::Window;
		wflags |= 
			Qt::FramelessWindowHint
			// | Qt::X11BypassWindowManagerHint
			// | Qt::WindowStaysOnTopHint
			;
		pFolder = new ClientFolder ( wflags );
		pFolder -> setRemoteMail (remoteMail);
		pFolder -> setToggle (haveToggle);
		pFolder -> hide ();
	}
	return app.exec();
}

//=========================================
// usage
//-----------------------------------------
void usage (void) {
	printf ("Linux QBiff Version 1.2 Server and Client\n");
	printf ("(C) Copyright 2004 SuSE GmbH\n");
	printf ("\n");
	printf ("usage: qbiff [ options ]\n");
	printf ("options:\n");
	printf ("[ -r | --remote ]\n");
	printf ("   in server mode: Enables the flag files to be written.\n");
	printf ("   flag files can be used to start a terminal based program\n");
	printf ("   on the remote side whereas the controling terminal\n");
	printf ("   remains local\n");
	printf ("[ -s | --server <ip> ]\n");
	printf ("   in client mode: specify server to connect.\n");
	printf ("[ -p | --port <number> ]\n");
	printf ("   in client mode: specify server port to connect.\n");
	printf ("[ -t | --toggle ]\n");
	printf ("   show toggle button to be able to switch between\n");
	printf ("   readmail and readmail.private to be called.\n");
	printf ("[ -m | --readmail <program> ]\n");
	printf ("   call this program as standard mail reader\n");
	printf ("[ -i | --privmail <program> ]\n");
	printf ("   call this program as private mail reader (toggle)\n");
	printf ("[ -b | --basedir <directory> ]\n");
	printf ("   base directory for metadata, default: /usr/share/qbiff\n");
	printf ("   certification and pixmap information is stored here\n");
	printf ("[ -f | --mailfolder <directory> ]\n");
	printf ("   path to maildir organised mail files\n");
	printf ("--\n");
	exit (1);
}

//=========================================
// clean sweep and exit
//-----------------------------------------
void quit (int code,siginfo_t*,void*) {
	if (pFolder) {
		printf ("End Client Session\n");
		pFolder->cleanup();
	}
	if (pServer) {
		printf ("End Server Session\n");
	}
	if ( ! useGUI ) {
		QString runfile;
		runfile.sprintf ("/var/run/qbiffd//qbiff.%d.pid",serverPort);
		QFile run (runfile);
		run.remove();
	}
	exit (code);
}
