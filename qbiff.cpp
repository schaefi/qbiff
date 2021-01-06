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
#include <QApplication>
#include <KAboutData>

#include <KLocalizedString>
#include <locale.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <QCommandLineParser>
#include <QCommandLineOption>

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
	// create Qt application
	//-----------------------------------------
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("qbiff");

	//=========================================
	// about this program
	//-----------------------------------------
    KAboutData aboutData(
        // The program name used internally. (componentName)
        QStringLiteral("qbiff"),
        // A displayable program name string. (displayName)
        i18n("qbiff"),
        // The program version string. (version)
        QStringLiteral("0.2.0"),
        // Short description of what the app does. (shortDescription)
        i18n("QBiff buttonbar, notify on new mail"),
        // The license this code is released under
        KAboutLicense::GPL,
        // Copyright Statement (copyrightStatement = QString())
        i18n("(c) 2021"),
        // Optional text shown in the About box.
        // Can contain any information desired. (otherText)
        i18n("Prints horizontal aligned buttons for each maildir folder"),
        // The program homepage string. (homePageAddress = QString())
        QStringLiteral("http://example.com/"),
        // The bug report email address
        // (bugsEmailAddress = QLatin1String("submit@bugs.kde.org")
        QStringLiteral("submit@bugs.kde.org")
    );

    aboutData.addAuthor(
        i18n("Marcus"),
        i18n("Developer"),
        QStringLiteral("ms@suse.com"),
        QStringLiteral("http://example.com"),
        QStringLiteral("schaefi")
    );
    
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption daemonOption(
        QStringList() << "d" << "daemon",
        QCoreApplication::translate("main", "Daemon/Server mode")
    );
    QCommandLineOption remoteMailOption(
        QStringList() << "r" << "remote",
        QCoreApplication::translate("main", "Remote Mail")
    );
    QCommandLineOption toggleOption(
        QStringList() << "t" << "toggle",
        QCoreApplication::translate("main", "Activate Toggle Button")
    );
    QCommandLineOption userOption(
        QStringList() << "u" << "user",
        QCoreApplication::translate("main", "Run with specified user privileges <name>"),
        QCoreApplication::translate("main", "name")
    );
    QCommandLineOption groupOption(
        QStringList() << "g" << "group",
        QCoreApplication::translate("main", "Run with specified group privileges <name>"),
        QCoreApplication::translate("main", "name")
    );
    QCommandLineOption serverOption(
        QStringList() << "s" << "server",
        QCoreApplication::translate("main", "Server Address <address>"),
        QCoreApplication::translate("main", "address")
    );
    QCommandLineOption portOption(
        QStringList() << "p" << "port",
        QCoreApplication::translate("main", "Port Number <number>"),
        QCoreApplication::translate("main", "number")
    );
    QCommandLineOption mailFolderOption(
        QStringList() << "f" << "mailfolder",
        QCoreApplication::translate("main", "Mail Folder Path <path>"),
        QCoreApplication::translate("main", "path")
    );
    QCommandLineOption buttonFontOption(
        QStringList() << "F" << "buttonfont",
        QCoreApplication::translate("main", "Button Font <name>"),
        QCoreApplication::translate("main", "name")
    );
    QCommandLineOption buttonFontSizeOption(
        QStringList() << "Z" << "buttonfontsize",
        QCoreApplication::translate("main", "Button Font Size <size>"),
        QCoreApplication::translate("main", "size")
    );
    QCommandLineOption readmailOption(
        QStringList() << "m" << "readmail",
        QCoreApplication::translate("main", "Mail Program <program>"),
        QCoreApplication::translate("main", "program")
    );
    QCommandLineOption readmailPrivOption(
        QStringList() << "i" << "readpriv",
        QCoreApplication::translate("main", "Private Mail Program <program>"),
        QCoreApplication::translate("main", "program")
    );
    QCommandLineOption basedirOption(
        QStringList() << "b" << "basedir",
        QCoreApplication::translate("main", "Base Path <path>"),
        QCoreApplication::translate("main", "path")
    );
    parser.addOption(daemonOption);
    parser.addOption(remoteMailOption);
    parser.addOption(toggleOption);
    parser.addOption(userOption);
    parser.addOption(groupOption);
    parser.addOption(serverOption);
    parser.addOption(portOption);
    parser.addOption(mailFolderOption);
    parser.addOption(buttonFontOption);
    parser.addOption(buttonFontSizeOption);
    parser.addOption(readmailOption);
    parser.addOption(readmailPrivOption);
    parser.addOption(basedirOption);

    aboutData.setupCommandLine(&parser);
    parser.process(app);

    aboutData.processCommandLine(&parser);

	if (! parser.isSet(daemonOption)) {
		useGUI = true;
	}

	//=========================================
	// init variables...
	//-----------------------------------------
	bool remoteMail     = false;
	bool haveToggle     = false;

	//=========================================
	// get options
	//-----------------------------------------
	if (parser.isSet(toggleOption)) {
		haveToggle = true;
	}
	if (parser.isSet(remoteMailOption)) {
		remoteMail = true;
	}
	user_name  = parser.value(userOption);
	group_name = parser.value(groupOption);
	myFolder   = parser.value(mailFolderOption);
	myButtonFontSize = parser.value(buttonFontSizeOption).toInt();
	myButtonFont = parser.value(buttonFontOption);
	serverName = parser.value(serverOption);
	serverPort = parser.value(portOption).toInt();
	mailClient = parser.value(readmailOption);
	mailPrivate= parser.value(readmailPrivOption);
    if (parser.isSet(basedirOption)) {
	    baseDir = parser.value(basedirOption);
    } else {
        baseDir = "/usr/share/qbiff";
    }
	myFolder += "/";
	
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
		QString pidfile ("/var/run/qbiff.pid");
		QFile run (pidfile);
		if (run.exists()) {
			if (run.open( QIODevice::ReadOnly )) {
				QTextStream stream ( &run );
				QString pid = stream.readLine();
				run.close();
				if (kill (pid.toInt(),0) == 0) {
					printf (
						"qbiff::already running: %s\n",pid.toLatin1().data()
					);
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
		pFolder = new ClientFolder();
		pFolder -> setRemoteMail (remoteMail);
		pFolder -> setToggle (haveToggle);
		pFolder -> hide ();
	}
    return app.exec();
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
		QString runfile ("/var/run/qbiff.pid");
		QFile run (runfile);
		run.remove();
	}
	exit (code);
}
