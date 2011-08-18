/**************
FILE          : parser.cpp
***************
PROJECT       : QBiff - A qt based biff
              :
AUTHOR        : Marcus Schäfer <ms@suse.de>
              :
BELONGS TO    : new mail notification service 
              : 
DESCRIPTION   : implementation for parsing .qbiffrc
              :
              :
STATUS        : Status: Beta
**************/
#include <stdlib.h>
#include <stdio.h>

#include "parser.h"

//=========================================
// Constructor
//-----------------------------------------
Parser::Parser ( const QString & file ) {
	mFile = new QFile (file);
	if (! mFile -> open(QIODevice::ReadOnly)) {
		printf ("Parser::couldn't open file: %s\n",file.toLatin1().data());
		exit (1);
	}
	QTextStream stream( mFile );
	QString* line = new QString();
	while ( !stream.atEnd() ) {
		*line = stream.readLine();
		if ((line->at(0) == '#') || (line->isEmpty())) {
			continue;
		}
		char* name = (char*)malloc(sizeof(char)*line->length()+1);
		strcpy (name,line->toLatin1().data());
		mList.append (name);
	}
}

//=========================================
// return file list
//-----------------------------------------
QList<char*> Parser::folderList (void) {
	return mList;
}
