//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012, Ken Moore
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
#include <QApplication>

class LUtils{
public:
	//Get the current version of the Lumina desktop
	static QString LuminaDesktopVersion();
	
	//Run an external command and return the exit code
	static int runCmd(QString cmd, QStringList args = QStringList());
	//Run an external command and return any text output (one line per entry)
	static QStringList getCmdOutput(QString cmd, QStringList args = QStringList());

	//Read a text file
	static QStringList readFile(QString filepath);
	//Write a text file
	static bool writeFile(QString filepath, QStringList contents, bool overwrite=false);

	//Check whether a file/path is a valid binary
	static bool isValidBinary(QString bin); //full path or name only

	//List all the sub-directories of a parent dir (recursive)
	static QStringList listSubDirectories(QString dir, bool recursive = true);

	//Load a translation file for a Lumina Project
	static void LoadTranslation(QApplication *app, QString appname);

	//Load the default setup for the system
	static void LoadSystemDefaults(bool skipOS = false);
	
};

#endif
