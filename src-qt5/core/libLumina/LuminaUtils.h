//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_LIBRARY_UTILS_H
#define _LUMINA_LIBRARY_UTILS_H

#include <QCoreApplication>
#include <QProcess>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QTranslator>
//#include <QApplication>
#include <QMenu>
#include <QMouseEvent>
#include <QSize>
#include <QWidgetAction>

class LUtils{
public:
	//Get the current version/build of the Lumina desktop
	static QString LuminaDesktopVersion();
	static QString LuminaDesktopBuildDate();

	//Run an external command and return the exit code
	static int runCmd(QString cmd, QStringList args = QStringList());
	//Run an external command and return any text output (one line per entry)
	static QStringList getCmdOutput(QString cmd, QStringList args = QStringList());

	//Read a text file
	static QStringList readFile(QString filepath);
	//Write a text file
	static bool writeFile(QString filepath, QStringList contents, bool overwrite=false);

	//Check whether a file/path is a valid binary
	static bool isValidBinary(QString& bin); //full path or name only
	static bool isValidBinary(const char *bin){
	  QString bins(bin);
	  return isValidBinary(bins); //overload for a "junk" binary variable input
	}

	//Create the exec string to open a terminal in a particular directory
	static QString GenerateOpenTerminalExec(QString term, QString dirpath);
	
	//List all the sub-directories of a parent dir (recursive)
	static QStringList listSubDirectories(QString dir, bool recursive = true);

	//Convert an input file/dir path to an absolute file path
	static QString PathToAbsolute(QString path); //This is primarily for CLI usage (relative paths)
	static QString AppToAbsolute(QString path); //This is for looking up a binary/ *.desktop path

	//Get the list of all file extensions which Qt can read (lowercase)
	static QStringList imageExtensions(bool wildcards = false);
	
	//Load a translation file for a Lumina Project
	static QTranslator* LoadTranslation(QApplication *app, QString appname, QString locale = "", QTranslator *cTrans = 0);
	//Other localization shortcuts
	static QStringList knownLocales(); //Note: This only lists locales known to Lumina (so the i18n files need to be installed)
	static void setLocaleEnv(QString lang, QString msg="", QString time="", QString num="" ,QString money="",QString collate="", QString ctype="");
	static QString currentLocale();
	
	//Number format conversions
	static double DisplaySizeToBytes(QString num); //Turn a display size (like 50M or 50KB) into a double for calculations (bytes)
	static QString BytesToDisplaySize(qint64 bytes); //convert into a readable size (like 50M or 50KB)
	
	static QString SecondsToDisplay(int secs); //convert into a readable time
	
	//Various function for finding valid QtQuick plugins on the system
	static bool validQuickPlugin(QString ID);
	static QString findQuickPluginFile(QString ID);
	static QStringList listQuickPlugins(); //List of valid ID's
	static QStringList infoQuickPlugin(QString ID); //Returns: [Name, Description, Icon]
	
	//Various functions for the favorites sub-system
	// Formatting Note: "<name>::::[dir/app/<mimetype>]::::<path>"
	//   the <name> field might not be used for "app" flagged entries
	static QStringList listFavorites();
	static bool saveFavorites(QStringList);
	static bool isFavorite(QString path);
	static bool addFavorite(QString path, QString name = "");
	static void removeFavorite(QString path);
	static void upgradeFavorites(int fromoldversionnumber);
	
	//Load the default setup for the system
	static void LoadSystemDefaults(bool skipOS = false);
	static bool checkUserFiles(QString lastversion); //returns true if something changed
	static int VersionStringToNumber(QString version); //convert the lumina version string to a number for comparisons

};

//Special subclass for a menu which the user can grab the edges and resize as necessary
// Note: Make sure that you don't set 0pixel contents margins on this menu 
//    - it needs at least 1 pixel margins for the user to be able to grab it
class ResizeMenu : public QMenu{
	Q_OBJECT
public:
	ResizeMenu(QWidget *parent = 0);
	virtual ~ResizeMenu();

	void setContents(QWidget *con);

private:
	enum SideFlag{NONE, TOP, BOTTOM, LEFT, RIGHT};
	SideFlag resizeSide;
	QWidget *contents;
	QWidgetAction *cAct;
	
private slots:
	void clearFlags(){
	  resizeSide=NONE;
	}

protected:
	virtual void mouseMoveEvent(QMouseEvent *ev);
	virtual void mousePressEvent(QMouseEvent *ev);
	virtual void mouseReleaseEvent(QMouseEvent *ev);

signals:
	void MenuResized(QSize); //Emitted when the menu is manually resized by the user

};

#endif
