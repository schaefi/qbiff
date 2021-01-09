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
#include "clientfolder.h"

//=========================================
// Globals
//-----------------------------------------
ClientFolder* pFolder = NULL;

int serverPort = PORT;
QString serverName = SERVER;

QString mailClient = MY_MAILCLIENT;
QString mailPrivate = MY_MAILCLPRIV;

QString baseDir = BASEDIR;
QString myFolder = MY_FOLDER;

QString myButtonFont = "FrutigerNextLT:style=Bold";
int myButtonFontSize = 10;

//=========================================
// Globals
//-----------------------------------------
QString DH1024;
QString DH512;
QString CLIENT_CERTFILE;

QString PIXSHAPE;
QString PIXSHAPEBG;
QString PIXNEWMAIL;
QString PIXNOMAIL;
QString PIXPUBL;
QString PIXPRIV;

//=========================================
// main
//-----------------------------------------
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

    QCommandLineOption toggleOption(
        QStringList() << "t" << "toggle",
        QCoreApplication::translate("main", "Activate Toggle Button")
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
    parser.addOption(toggleOption);
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

    //=========================================
    // init variables...
    //-----------------------------------------
    bool haveToggle = false;

    //=========================================
    // get options
    //-----------------------------------------
    if (parser.isSet(toggleOption)) {
        haveToggle = true;
    }

    myFolder = parser.value(mailFolderOption);

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
    
    //=======================================
    // certificate setup
    //---------------------------------------
    //CAFILE = baseDir + "/cert-client/rootcert.pem";
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

    pFolder = new ClientFolder();
    pFolder->setToggle(haveToggle);
    pFolder->hide();
    return app.exec();
}
