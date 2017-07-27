//===========================================
//  Lumina-DE source code
//  Copyright (c) 2013-2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// These structures/classes are for conforming to the FreeDesktop standards
// REFERENCE:
//    (DATABASE) https://www.freedesktop.org/wiki/Specifications/shared-mime-info-spec/
//    (APPLICATIONS) https://www.freedesktop.org/wiki/Specifications/mime-apps-spec/
// Mime Application Version Compliance: 1.0.1 (11/14/14) (Skips random *.desktop parsing: ~80% compliant)
//===========================================
#ifndef _LUMINA_LIBRARY_XDG_MIME_CLASS_H
#define _LUMINA_LIBRARY_XDG_MIME_CLASS_H

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QString>
#include <QList>
#include <QDateTime>
#include <QTextStream>
#include <QRegExp>
#include <QDebug>

class XDGMime{
public:
	// PRIMARY FUNCTIONS
	static QString fromFileName(QString filename); //Convert a filename into a mimetype
	static QStringList listFromFileName(QString filename); //Convert a filename into a list of mimetypes (arranged in descending priority)
	static QString toIconName(QString mime); //Mime type to icon name
	//Find the file extension for a particular mime-type
	static QStringList toFileExtensions(QString mime);

	// LESSER-USED FUNCTIONS
	//List all the mime-type directories
	static QStringList systemMimeDirs();
	//Find the mime-type of a particular file extension
	static QString findAppMimeForFile(QString filename, bool multiple = false);
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

};
#endif
