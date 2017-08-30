//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This is a simple class for managing all the various desktop
//  setting files, and sending out notifications about changes
//===========================================
//  NOTE:
//  This class has a heirarchy-based lookup/write system
//  USER Settings > SYSTEM Settings > DEFAULT Settings
//  XDG_CONFIG_HOME/lumina-desktop >  XDG_CONFIG_DIRS/lumina-desktop > XDG_DATA_DIRS/lumina-desktop
//===========================================
#ifndef _LUMINA_DESKTOP_SETTINGS_CLASS_H
#define _LUMINA_DESKTOP_SETTINGS_CLASS_H

#include <QSettings>
#include <QFileSystemWatcher>
#include <QString>
#include <QStringList>
#include <QHash>
#include <QVariant>

class DesktopSettings : public QObject{
	Q_OBJECT
public:
	enum File{ System, Favorites, Environment, Session, Desktop, Panels, Plugins, ContextMenu, Keys, Animation, ScreenSaver, WM};
	//Changes to this enum need to be added to the "filesForRunMode()" and "rel_path()" functions as well

	DesktopSettings(QObject *parent = 0);
	~DesktopSettings();

	static DesktopSettings* instance();

	//Start/stop routines
	void start();
	void stop();

	//Main Read/Write functions
	QList< DesktopSettings::File > writableFiles(); //return the list of all writable files
	QVariant value(DesktopSettings::File, QString variable, QVariant defaultvalue);
	bool setValue(DesktopSettings::File, QString variable, QVariant value);
	QStringList keys(DesktopSettings::File); //return a list of all variables which are available in this file

	//Information functions
	QStringList filePaths(DesktopSettings::File); //just in case we need to access any of these files outside this class

private:
	enum RunMode{UserFull, SystemFull, SystemInterface };
	DesktopSettings::RunMode runmode; //simple flag for which mode the current session is running in
	QFileSystemWatcher *watcher;
	QHash< DesktopSettings::File, QStringList > files; //location hash for where files are actually located on disk
	QHash< QString, QSettings*> settings; //location hash for the settings files themselves

	//Functions
	void parseSystemSettings(); //run at start - determine the RunMode for this user/session
	void locateFiles(); //run at start - finds the locations of the various files (based on RunMode)
	void touchFile(QString path); //used to create an empty file so it can be watched for changes later

	//The two functions which define the public "File" enumeration (both need updates when the enum changes)
	QList< DesktopSettings::File > filesForRunMode(RunMode mode);
	QString rel_path(DesktopSettings::File); //return the relative file path (starting with "/")

private slots:
	void fileChanged(QString); //QFileSystemWatcher change detected
	void dirChanged(QString); //QFileSystemWatcher change detected

signals:
	void FileModified(DesktopSettings::File);

};
#endif
