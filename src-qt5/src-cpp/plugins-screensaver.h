//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This is a simple class for managing all the various desktop
//  screensaver plugins that could be available
//===========================================
//  NOTE:
//  This class has a heirarchy-based lookup system
//  USER plugins > SYSTEM plugins
//  XDG_DATA_HOME/lumina-desktop/screensavers >  XDG_DATA_DIRS/lumina-desktop/screensavers
//===========================================
#ifndef _LUMINA_DESKTOP_SCREENSAVER_PLUGINS_CLASS_H
#define _LUMINA_DESKTOP_SCREENSAVER_PLUGINS_CLASS_H

#include <QJsonObject>
#include <QString>
#include <QUrl>
#include <QObject>

class SSPlugin{
private:
	QString currentfile;

public:
	QJsonObject data; //Hazardous to manually modify

	SSPlugin();
	~SSPlugin();

	void loadFile(QString path);
	bool isLoaded();

	bool isValid();

	QString translatedName();
	QString translatedDescription();
	QUrl scriptURL();
};

class SSPluginSystem{
public:
	static SSPlugin findPlugin(QString name);
	static QList<SSPlugin> findAllPlugins(bool validonly = true);

};

#endif
