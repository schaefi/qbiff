#ifndef PARSER_H
#define PARSER_H 1

#include <qfile.h>
#include <qlist.h>
#include <qiodevice.h>
#include <qtextstream.h>

//=========================================
// Class definitions...
//-----------------------------------------
class Parser {
	private:
    QList<QString> mFolderList;

	public:
	Parser(const QString &);

	public:
    QList<QString> folderList (void);
};

#endif
