//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_CONFIG_PLUGINS_H
#define _LUMINA_DESKTOP_CONFIG_PLUGINS_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QStringList>

// Lumina Plugin Info class 
class LPI{
  public:
    QString name, ID, description, icon;
    LPI(){}
    ~LPI(){}
};

// Utility class for getting information
class LPlugins{
public:
	LPlugins();
	~LPlugins();
	//Total lists of plugins
	QStringList panelPlugins();
	QStringList desktopPlugins();
	QStringList menuPlugins();
	QStringList colorItems();
	//Information on individual plugins
	LPI panelPluginInfo(QString);
	LPI desktopPluginInfo(QString);
	LPI menuPluginInfo(QString);
	LPI colorInfo(QString);

private:
	QHash<QString, LPI> PANEL, DESKTOP, MENU, COLORS;
	void LoadPanelPlugins();
	void LoadDesktopPlugins();
	void LoadMenuPlugins();
	void LoadColorItems();
};
#endif
