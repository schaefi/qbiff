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
void Folder::setStatus (unsigned int m_current, unsigned int m_new) {
    setCurrent(m_current);
    setNew(m_new);
}

//====================================
// set folder status current entries
//------------------------------------
void Folder::setCurrent(unsigned int m_current) {
    if (mCurrent != m_current) {
        hasChanged = true;
    }
    mCurrent = m_current;
}

//====================================
// set folder status, new entries
//------------------------------------
void Folder::setNew(unsigned int m_new) {
    if (mNew != m_new) {
        hasChanged = true;
    }
    mNew = m_new;
}

//====================================
// hasChanged
//------------------------------------
bool Folder::hasChanges(void) {
    return hasChanged;
}

//====================================
// resetChangeFlag
//------------------------------------
void Folder::resetChanges(void) {
    hasChanged = false;
}

//====================================
// get folder status
//------------------------------------
QString Folder::getStatus(void) {
    QString stream;
    QTextStream(&stream) << mFolder
        << ":"
        << _getStatusText()
        << ":" << mNew
        << ":" << mCurrent;
    return stream;
}

//====================================
// get folder status as text
//------------------------------------
QString Folder::_getStatusText(void) {
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
