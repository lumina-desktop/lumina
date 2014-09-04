//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_CONFIG_PLUGINS_H
#define _LUMINA_DESKTOP_CONFIG_PLUGINS_H

#include <QHash>
#include <QString>
#include <QStringList>

// Lumina Plugin Info class 
class LPI{
  public:
    QString name, ID, description, icon;
    int width, height; //only used for desktop plugins
    LPI(){
      width=0; height=0;
    }
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
	//Information on individual plugins
	LPI panelPluginInfo(QString);
	LPI desktopPluginInfo(QString);
	LPI menuPluginInfo(QString);

private:
	QHash<QString, LPI> PANEL, DESKTOP, MENU;
	void LoadPanelPlugins();
	void LoadDesktopPlugins();
	void LoadMenuPlugins();
};
#endif