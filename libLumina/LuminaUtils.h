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
#include <QString>
#include <QStringList>

class LUtils{
public:
	//Run an external command and return the exit code
	static int runCmd(QString cmd, QStringList args = QStringList());
	//Run an external command and return any text output (one line per entry)
	static QStringList getCmdOutput(QString cmd, QStringList args = QStringList());

	//Read a text file
	static QStringList readFile(QString filepath);
	//Write a text file
	static bool writeFile(QString filepath, QStringList contents, bool overwrite=false);
};

#endif
