//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012-2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_LIBRARY_DESKTOP_UTILS_H
#define _LUMINA_LIBRARY_DESKTOP_UTILS_H

#include <QString>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QSettings>

//Other classes needed
#include <LUtils.h>
#include <LuminaXDG.h>
#include <LuminaOS.h>

class LDesktopUtils{
public:
	//Get the current version/build of the Lumina desktop
	static QString LuminaDesktopVersion();
	static QString LuminaDesktopBuildDate();

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
	static bool checkUserFiles(QString lastversion, QString currentversion); //returns true if something changed
	static int VersionStringToNumber(QString version); //convert the lumina version string to a number for comparisons

	//Migrating desktop settings from one ID to another
	static void MigrateDesktopSettings(QSettings *settings, QString fromID, QString toID);

};

#endif
