//===========================================
//  Lumina-DE source code
//  Copyright (c) 2013, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// These structures/classes are for conforming to the FreeDesktop standards
// REFERENCE: (*.desktop files) http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s05.html
// -- Current Implementation (OCT 2013) --
// Desktop File Version Compliance: 1.0 (except "DBusActivatable")
// Icon Theme Compliance: Built in to Qt (QIcon::fromTheme()) with "oxygen" theme default
//===========================================

  
#ifndef _LUMINA_LIBRARY_XDG_H
#define _LUMINA_LIBRARY_XDG_H

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QString>
#include <QIcon>
#include <QList>
#include <QHash>
#include <QLocale>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>

class XDGDesktop{
public:
  enum XDGDesktopType { BAD, APP, LINK, DIR };
  //Admin variables
  QString filePath; //which file this structure contains the information for (absolute path)
  XDGDesktopType type;
  //General variables
  QString name, genericName, comment, icon;
  QStringList showInList, notShowInList;
  bool isHidden;
  //Type 1 (APP) variables
  QString exec, tryexec, path, startupWM;
  QStringList actionList, mimeList, catList, keyList;
  bool useTerminal, startupNotify;
  //Type 2 (LINK) variables
  QString url;
  
  //Constructor/destructor
  XDGDesktop(){}
  ~XDGDesktop(){}
};


class LXDG{
public:
	//Read a *.desktop file
	static XDGDesktop loadDesktopFile(QString filePath, bool& ok);
	//Check a *.desktop file for validity (showAll skips the DE-exclusivity checks)
	static bool checkValidity(XDGDesktop dFile, bool showAll = true); 
	//Check for a valid executable
	static bool checkExec(QString exec);
	//Get a list of all the directories where *.desktop files exist
	static QStringList systemApplicationDirs();
	//Get a list of all the *.desktop files available on the system
	static QList<XDGDesktop> systemDesktopFiles(bool showAll = false, bool showHidden = false);
	//Sort a list of Desktop files into the proper categories
	static QHash< QString, QList<XDGDesktop> > sortDesktopCats(QList<XDGDesktop> apps);
	//Sort a list of Desktop files by name
	static QList<XDGDesktop> sortDesktopNames(QList<XDGDesktop> apps);
	//Get the executable line from a Desktop file
	static QString getDesktopExec(XDGDesktop);
	//Set all the default XDG Environment variables
	static void setEnvironmentVars();
	//Find an icon from the current/default theme
	static QIcon findIcon(QString iconName, QString fallback = "");
	//Recursivly compile a list of child directories with *.png files in them
	static QStringList getChildIconDirs(QString parent);
	//List all the mime-type directories
	static QStringList systemMimeDirs();
	//Find the mime-type icon for a particular file extension
	static QIcon findMimeIcon(QString extension);
	//Find the mime-type of a particular file extension
	static QString findAppMimeForFile(QString extension);
	//Find the file extension for a particular mime-type
	static QStringList findFilesForMime(QString mime);
	//Load all the "globs2" mime database files
	static QStringList loadMimeFileGlobs2();
};

#endif

