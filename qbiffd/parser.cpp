#include <stdlib.h>
#include <stdio.h>

#include "parser.h"

//=========================================
// Constructor
//-----------------------------------------
Parser::Parser ( const QString & file ) {
	QFile folder_config(file);
	if (! folder_config.open(QIODevice::ReadOnly)) {
		printf ("Parser::couldn't open file: %s\n",file.toLatin1().data());
		exit (1);
	}
	QTextStream stream(&folder_config);
    QString folder_name;
	while (!stream.atEnd()) {
        folder_name = stream.readLine();
        if ((folder_name.at(0) == '#') || (folder_name.isEmpty())) {
			continue;
		}
        mFolderList.append(folder_name);
	}
}

//=========================================
// return file list
//-----------------------------------------
QList<QString> Parser::folderList (void) {
    return mFolderList;
}
