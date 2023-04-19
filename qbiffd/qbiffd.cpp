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
#include "server.h"
#include "scheduler.h"

//=========================================
// Globals
//-----------------------------------------
Scheduler* pScheduler = NULL;
Server* pServer = NULL;
int serverPort = PORT;
QString serverName = SERVER;
QString mailClient = MY_MAILCLIENT;
QString mailPrivate = MY_MAILCLPRIV;
QString baseDir = BASEDIR;
QString myFolder = MY_FOLDER;

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

int main(int argc,char*argv[]) {
	//=========================================
	// set locale
	//-----------------------------------------
	setlocale (LC_ALL,"");

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
        QStringLiteral("qbiffd"),
        // A displayable program name string. (displayName)
        i18n("qbiffd"),
        // The program version string. (version)
        QStringLiteral("0.2.0"),
        // Short description of what the app does. (shortDescription)
        i18n("QBiff buttonbar server, notify on new mail"),
        // The license this code is released under
        KAboutLicense::GPL,
        // Copyright Statement (copyrightStatement = QString())
        i18n("(c) 2021"),
        // Optional text shown in the About box.
        // Can contain any information desired. (otherText)
        i18n("Servers maildir folder information"),
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
    parser.addOption(serverOption);
    parser.addOption(portOption);
    parser.addOption(mailFolderOption);

    aboutData.setupCommandLine(&parser);
    parser.process(app);

    aboutData.processCommandLine(&parser);

	myFolder   = parser.value(mailFolderOption);
	serverName = parser.value(serverOption);
	serverPort = parser.value(portOption).toInt();

	myFolder += "/";
	
	//=======================================
	// certification stuff
	//---------------------------------------
	CAFILE = baseDir + "/cert-client/rootcert.pem";
	DH1024 = baseDir + "/cert-server/dh1024.pem";
	DH512 = baseDir + "/cert-server/dh512.pem";
	SERVER_CERTFILE= baseDir + "/cert-server/server.pem";

    pServer = new Server;
    pServer->start();

    pScheduler = new Scheduler(pServer);

    QObject::connect(
        pServer->getWatcher(), SIGNAL(directoryChanged(QString)),
        pScheduler, SLOT(timerDone())
    );

    return app.exec();
}
