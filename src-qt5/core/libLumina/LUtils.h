//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012-2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_LIBRARY_UTILS_H
#define _LUMINA_LIBRARY_UTILS_H

#include <QCoreApplication>
#include <QProcess>
#include <QTextStream>
#include <QTextCodec>
#include <QFile>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QTranslator>
#include <QDebug>
#include <QDesktopWidget>
#include <QImageReader>
#include <QRegExp>
#include <QFuture>
#include <QScreen>
#include <QSettings>

class LUtils{
public:
	enum StandardDir {Desktop, Documents, Downloads, Music, Pictures, PublicShare, Templates, Videos};

	//Return the path to one of the XDG standard directories
	static QString standardDirectory(StandardDir dir, bool createAsNeeded = true);

	//Run an external command and return output & exit code
	static QString runCommand(bool &success, QString command, QStringList arguments = QStringList(), QString workdir = "", QStringList env = QStringList());

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

	//Open the right settings file (user/permissions aware)
	static QSettings* openSettings(QString org, QString name, QObject *parent = 0);

	//Return all the dirs on the system which contain .desktop files
        static QStringList systemApplicationDirs();

	//Create the exec string to open a terminal in a particular directory
	static QString GenerateOpenTerminalExec(QString term, QString dirpath);

	//List all the sub-directories of a parent dir (recursive)
	static QStringList listSubDirectories(QString dir, bool recursive = true);

	//Convert an input file/dir path to an absolute file path
	static QString PathToAbsolute(QString path); //This is primarily for CLI usage (relative paths)
	static QString AppToAbsolute(QString path); //This is for looking up a binary/ *.desktop path

	//Get the list of all file extensions which Qt can read (lowercase)
	static QStringList imageExtensions(bool wildcards = false);
	static QStringList videoExtensions();

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
};
#endif
