//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This is the Widgets version of a floating, top-level panel
//===========================================
#ifndef _DESKTOP_WIDGETS_PANEL_H
#define _DESKTOP_WIDGETS_PANEL_H

#include <global-includes.h>
#include <QBoxLayout>
#include "Plugin.h"

class Panel : public QWidget {
	Q_OBJECT
private:
	PanelObject *obj;
	QBoxLayout *layout;
	QStringList lastplugins;
	//Stuff for managing the plugins
	QList<Plugin*> PLUGINS;
	Plugin* findPlugin(QString id);
	Plugin* createPlugin(QString id);

public:
	Panel(PanelObject *pobj);
	~Panel();

private slots:
	void objectDestroyed(QObject*);
	void updateGeom();
	void updateBackground();
	void updatePlugins();

};
#endif
