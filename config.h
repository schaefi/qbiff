/**************
FILE          : config.h
***************
PROJECT       : QBiff - A qt based biff
              :
AUTHOR        : Marcus Schäfer <ms@suse.de>
              :
BELONGS TO    : new mail notification service 
              : 
              :
DESCRIPTION   : default values
              :
              :
STATUS        : Status: Beta
**************/
#ifndef CONFIG_H
#define CONFIG_H 1
//=======================================
// root of the maildir tree
//---------------------------------------
#define MY_FOLDER      "/home/ms/Mail/"

//=======================================
// program to call on click
//---------------------------------------
#define MY_MAILCLIENT  "/usr/share/qbiff/readmail"
#define MY_MAILCLPRIV  "/usr/share/qbiff/readmail.private"

//=======================================
// network stuff
//---------------------------------------
#define PORT     17142
#define SERVER   "localhost"
#define CLIENT   "localhost"

#define BASE_DEBUG
//=======================================
// Base Directory
//---------------------------------------
#define BASEDIR "/usr/share/qbiff"

//=======================================
// certification stuff
//---------------------------------------
#define CADIR    NULL
#endif
