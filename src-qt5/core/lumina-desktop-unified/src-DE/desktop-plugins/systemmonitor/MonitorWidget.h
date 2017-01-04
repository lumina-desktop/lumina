//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This plugin is a simple hardware status monitor on the desktop
//===========================================
#ifndef _LUMINA_DESKTOP_PLUGIN_HW_MONITOR_WIDGET_H
#define _LUMINA_DESKTOP_PLUGIN_HW_MONITOR_WIDGET_H

#include <QTimer>
#include <QWidget>

#include "../LDPlugin.h"

namespace Ui{
	class MonitorWidget;
};

class MonitorWidget : public QWidget{
	Q_OBJECT
public:
	MonitorWidget(QWidget *parent = 0);
	~MonitorWidget();

public slots:
	void LoadIcons();

private:
	Ui::MonitorWidget *ui;
	QTimer *upTimer;

private slots:
	void UpdateStats();
};

// Wrapper class to put this into a desktop plugin container
class SysMonitorPlugin : public LDPlugin{
	Q_OBJECT
public:
	SysMonitorPlugin(QWidget* parent, QString ID);
	~SysMonitorPlugin();

	virtual QSize defaultPluginSize(){
	  // The returned QSize is in grid points (typically 100 or 200 pixels square)
	  return QSize(3,2);
	}
	
private:
	MonitorWidget *monitor;

public slots:
	void LocaleChange(){
	  QTimer::singleShot(0,monitor, SLOT(LoadIcons()));
	}
	void ThemeChange(){
	  QTimer::singleShot(0,monitor, SLOT(LoadIcons()));
	}
};

#endif
