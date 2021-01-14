#include "folder.h"

//====================================
// Constructor
//------------------------------------
Folder::Folder (
	QString* folder, QPoint* count
) {
    mFolder = *folder;
    mCurrent = count->y();
    mNew     = count->x();
}

//====================================
// getFolder
//------------------------------------
QString Folder::getFolder(void) {
    return mFolder;
}

//====================================
// set folder status
//------------------------------------
void Folder::setStatus (int m_current, int m_new) {
    mCurrent = m_current;
    mNew = m_new;
}

//====================================
// set folder status current entries
//------------------------------------
void Folder::setCurrent(int m_current) {
    mCurrent = m_current;
}

//====================================
// set folder status, new entries
//------------------------------------
void Folder::setNew(int m_new) {
    mNew = m_new;
}

//====================================
// get folder status
//------------------------------------
QString Folder::getStatus(void) {
    QString stream;
    QTextStream(&stream) << mFolder
        << ":"
        << getStatusText()
        << ":" << mNew
        << ":" << mCurrent;
    return stream;
}

//====================================
// get folder status as text
//------------------------------------
QString Folder::getStatusText(void) {
    if ((mCurrent == 0) && (mNew == 0)) {
        return (FOLDER_EMPTY);
    }
    if ((mCurrent > 0) && (mNew == 0)) {
        return (FOLDER_UPTODATE);
    }
    if (mNew > 0) {
        return (FOLDER_NEW);
    }
    return (FOLDER_UNKNOWN);
}
