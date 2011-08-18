/**************
FILE          : parser.h
***************
PROJECT       : QBiff - A qt based biff
              :
AUTHOR        : Marcus Schäfer <ms@suse.de>
              :
BELONGS TO    : new mail notification service 
              : 
DESCRIPTION   : header file for parsing .qbiffrc
              :
              :
STATUS        : Status: Beta
**************/
#ifndef PARSER_H
#define PARSER_H 1

#include <qfile.h>
#include <qlist.h>
#include <qiodevice.h>
#include <qtextstream.h>

//=========================================
// Defines...
//-----------------------------------------
#define MAX_LINE_LENGTH 1024

//=========================================
// Class definitions...
//-----------------------------------------
class Parser {
	private:
	QFile* mFile;
	QList<char*> mList;

	public:
	Parser ( const QString & );

	public:
	QList<char*> folderList (void);
};

#endif
