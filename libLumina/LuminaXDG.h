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
// *.desktop Exec Compliance Updated: 9/9/2014
// Mime Application Version Compliance: 1.0.1 (11/14/14) (Skips random *.desktop parsing: ~80% compliant)
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

class XDGDesktopAction{
public:
  //Admin variables
  QString ID; //The ID name for this action (should correspond to an entry in the "actionList" for the XDGDesktop below)
  //General Variables
  QString name, icon, exec;
};

class XDGDesktop{
public:
  enum XDGDesktopType { BAD, APP, LINK, DIR };
  //Admin variables
  QString filePath; //which file this structure contains the information for (absolute path)
  QDateTime lastRead; //when this structure was created from the file
  XDGDesktopType type;
  //General variables
  QString name, genericName, comment, icon;
  QStringList showInList, notShowInList;
  bool isHidden;
  //Type 1 (APP) variables
  QString exec, tryexec, path, startupWM;
  QStringList actionList, mimeList, catList, keyList;
  bool useTerminal, startupNotify;
  QList<XDGDesktopAction> actions;
  //Type 2 (LINK) variables
  QString url;
  
  //Constructor/destructor
  XDGDesktop(){}
  ~XDGDesktop(){}
};


class LXDG{
public:
	//Read/write a *.desktop file
	static XDGDesktop loadDesktopFile(QString filePath, bool& ok);
	static bool saveDesktopFile(XDGDesktop dFile, bool merge = true);
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
	static QString getDesktopExec(XDGDesktop app, QString ActionID = "");
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
	static QString findAppMimeForFile(QString filename, bool multiple = false);
	//Find the file extension for a particular mime-type
	static QStringList findFilesForMime(QString mime);
	// Simplification function for finding all info regarding current mime defaults
	static QStringList listFileMimeDefaults();
	//Find the localized comment string for a particular mime-type
	static QString findMimeComment(QString mime);
	//Find the default application for a mime-type
	static QString findDefaultAppForMime(QString mime);
	//Fine the available applications for a mime-type
	static QStringList findAvailableAppsForMime(QString mime);
	//Set the default application for a mime-type
	static void setDefaultAppForMime(QString mime, QString app);
	//List all the registered audio/video file extensions
	static QStringList findAVFileExtensions();
	//Load all the "globs2" mime database files
	static QStringList loadMimeFileGlobs2();
	
	//Find all the autostart *.desktop files
	static QList<XDGDesktop> findAutoStartFiles(bool includeInvalid = false);
	static bool setAutoStarted(bool autostart, XDGDesktop app);
	static bool setAutoStarted(bool autostart, QString filePath); //for convenience
};

#endif

