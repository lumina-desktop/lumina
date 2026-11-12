//===========================================
//  Lumina-DE source code
//  Copyright (c) 2013-2019, Ken Moore
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
#include <QFileSystemWatcher>
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
#include <QMenu>
#include <QAction>
#include <QMutex>

// ======================
// FreeDesktop Desktop Actions Framework (data structure)
// ======================
struct XDGDesktopAction{
//public:
  //Admin variables
  QString ID; //The ID name for this action (should correspond to an entry in the "actionList" for the XDGDesktop below)
  //General Variables
  QString name, icon, exec;
};

// ======================
//  FreeDesktop Desktop Entry Framework (data structure)
// ======================
class XDGDesktop : public QObject{
	Q_OBJECT
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
    //Type 1 Extensions for Lumina (Optional)
    bool useVGL; //X-VGL

  //Type 2 (LINK) variables
  QString url;

	//Constructor/destructor
	XDGDesktop(QString filePath="", QObject *parent = 0);
	~XDGDesktop(){}

	//Functions for using this structure in various ways
	void sync(); //syncronize this structure with the backend file(as listed in the "filePath" variable)
	bool isValid(bool showAll = true); //See if this is a valid .desktop entry (showAll: don't filter out based on DE exclude/include lists)

	QString getDesktopExec(QString ActionID = ""); //Just return the exec field with minimal cleanup
	QString generateExec(QStringList inputfiles = QStringList(), QString ActionID = "");  //Format the exec command to account for input files

	bool saveDesktopFile(bool merge = true); //This will use the "filePath" variable for where to save the file

	bool setAutoStarted(bool autostart = true);

	//Create a menu entry for this application
	void addToMenu(QMenu*);
};

// ========================
//  Data Structure for keeping track of known system applications
// ========================
class XDGDesktopList : public QObject{
	Q_OBJECT
public:
	//Functions
	XDGDesktopList(QObject *parent = 0, bool watchdirs = false);
	~XDGDesktopList();

	static XDGDesktopList* instance();

	//Main Interface functions
	QList<XDGDesktop*> apps(bool showAll, bool showHidden); //showAll: include invalid files, showHidden: include NoShow/Hidden files
	XDGDesktop* findAppFile(QString filename);
	void populateMenu(QMenu *, bool byCategory = true);

	//Administration variables (not typically used directly)
	QDateTime lastCheck;
	QStringList newApps, removedApps; //list of "new/removed" apps found during the last check
	QHash<QString, XDGDesktop*> files; //<filepath>/<XDGDesktop structure>

public slots:
	void updateList(); //run the check routine

private:
	QFileSystemWatcher *watcher;
	QTimer *synctimer;
	bool keepsynced;
	QMutex hashmutex;

private slots:
	void watcherChanged();
signals:
	void appsUpdated();
};

// ========================
// File Information simplification class (combine QFileInfo with XDGDesktop)
//  Need some extra information not usually available by a QFileInfo
// ========================
/*class LFileInfo : public QFileInfo{
private:
	QString mime, icon;
	XDGDesktop *desk;

	void loadExtraInfo();

public:
	//Couple overloaded contructors
	LFileInfo();
	LFileInfo(QString filepath);
	LFileInfo(QFileInfo info);
	~LFileInfo(){
	  desk->deleteLater();
	}

	//Functions for accessing the extra information
	// -- Return the mimetype for the file
	QString mimetype();

	// -- Return the icon file to use for this file
	QString iconfile(); //Note: This string is auto-formatted for use in the LXDG::findIcon() routine.

	// -- Check if this is an XDG desktop file
	bool isDesktopFile();

	// -- Allow access to the internal XDG desktop data structure
	XDGDesktop* XDG();

	//Other file type identification routines
	bool isImage(); //Is a readable image file (for thumbnail support)
	bool isVideo(); //Is a readable video file (for thumbnail support)
	bool isAVFile(); //Is an audio/video file
};
typedef QList<LFileInfo> LFileInfoList;*/

// ================================
//  Collection of FreeDesktop standards interaction routines
// ================================
class LXDG{
public:
	//Read/write a *.desktop file
	//static XDGDesktop* loadDesktopFile(QString filepath, bool&ok, QObject *parent = 0);
	//static bool saveDesktopFile(XDGDesktop *dFile, bool merge = true);
	//Check a *.desktop file for validity (showAll skips the DE-exclusivity checks)
	//static bool checkValidity(XDGDesktop *dFile, bool showAll = true);
	//Check for a valid executable
	static bool checkExec(QString exec);
	//Get a list of all the directories where *.desktop files exist
	static QStringList systemApplicationDirs();
	//Get a list of all the *.desktop files available on the system
        //static XDGDesktopList* systemAppsList(); //return a pointer to the entire class
	//static QList<XDGDesktop*> systemDesktopFiles(bool showAll = false, bool showHidden = false); //simplification for getting just the files
	//Sort a list of Desktop files into the proper categories
	static QHash< QString, QList<XDGDesktop*> > sortDesktopCats(QList<XDGDesktop*> apps);
	//Return the icon to use for the given category
	static QString DesktopCatToIcon(QString cat);
	//Sort a list of Desktop files by name
	static QList<XDGDesktop*> sortDesktopNames(QList<XDGDesktop*> apps);
	//Get the executable line from a Desktop file
	//static QString getDesktopExec(XDGDesktop *app, QString ActionID = "");
	//Set all the default XDG Environment variables
	static void setEnvironmentVars();
	//Find an icon from the current/default theme
	static QIcon findIcon(QString iconName, QString fallback = "");
	//Recursivly compile a list of child directories with *.png files in them
	static QStringList getChildIconDirs(QString parent);
	static QStringList getIconThemeDepChain(QString theme, QStringList paths);
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
	static QList<XDGDesktop*> findAutoStartFiles(bool includeInvalid = false);
	//static bool setAutoStarted(bool autostart, XDGDesktop *app);
	static bool setAutoStarted(bool autostart, QString filePath); //for convenience
};

#endif
