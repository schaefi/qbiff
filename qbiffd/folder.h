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
    unsigned int mCurrent = 0;
    unsigned int mNew = 0;
    bool hasChanged = true;

    public:
    QString getFolder(void);
    QString getStatus(void);

    public:
    void setStatus(unsigned int m_current, unsigned int m_new);
    void setCurrent(unsigned int);
    void setNew(unsigned int);
    bool hasChanges(void);
    void resetChanges(void);

    private:
    QString _getStatusText(void);
};

#endif
