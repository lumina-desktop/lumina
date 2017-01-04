//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This plugin is a listing/launcher for things in the ~/Desktop folder
//===========================================
#ifndef _LUMINA_DESKTOP_DESKTOP_VIEW_PLUGIN_H
#define _LUMINA_DESKTOP_DESKTOP_VIEW_PLUGIN_H

#include <QListWidget>
#include <QVBoxLayout>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QMouseEvent>

#include "../LDPlugin.h"

class DesktopViewPlugin : public LDPlugin{
	Q_OBJECT
public:
	DesktopViewPlugin(QWidget* parent, QString ID);
	~DesktopViewPlugin();
	
	virtual QSize defaultPluginSize(){
	  // The returned QSize is in grid points (typically 100 or 200 pixels square)
	  return QSize(3,3);
	}
private:
	QListWidget *list;
	QMenu *menu;

private slots:
	void runItems();
	void copyItems();
	void cutItems();
	void deleteItems();
	void showMenu(const QPoint&);
	void increaseIconSize();
	void decreaseIconSize();
	void updateContents();
	void displayProperties();


public slots:
	void LocaleChange(){
	  QTimer::singleShot(0,this, SLOT(updateContents()));
	}
	void ThemeChange(){
	  QTimer::singleShot(0,this, SLOT(updateContents()));
	}

};
#endif
