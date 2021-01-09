#ifndef FOLDER_H
#define FOLDER_H 1

#include <qstring.h>
#include <qtextstream.h>
#include <qpoint.h>

//=========================================
// Defines...
//-----------------------------------------
#define FOLDER_EMPTY     "empty"
#define FOLDER_UPTODATE  "uptodate"
#define FOLDER_NEW       "new"
#define FOLDER_UNKNOWN   "unknown"

//=========================================
// Class Folder
//-----------------------------------------
class Folder {
    public:
    Folder (QString*, QPoint*);

    private:
    QString mFolder;
    unsigned int mCurrent;
    unsigned int mNew;

    public:
    QString getFolder(void);
    QString getStatus(void);

    public:
    void setStatus(int m_current, int m_new);
    void setCurrent(int);
    void setNew(int);

    private:
    QString getStatusText(void);
};

#endif
