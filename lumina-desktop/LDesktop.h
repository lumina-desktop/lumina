//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_LDESKTOP_H
#define _LUMINA_DESKTOP_LDESKTOP_H

#include <QCoreApplication>


#include <QSettings>
#include <QFile>
#include <QList>
#include <QDebug>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QLabel>
#include <QWidgetAction>
#include <QMdiArea>
#include <QMdiSubWindow>


#include <LuminaXDG.h>

#include "LPanel.h"
//#include "Globals.h"
#include "AppMenu.h"
#include "desktop-plugins/LDPlugin.h"
#include "desktop-plugins/LDPluginContainer.h"
#include "desktop-plugins/NewDP.h"

class LDesktop : public QObject{
	Q_OBJECT
public:
	LDesktop(int deskNum=0, bool setdefault = false);
	~LDesktop();
	
	int Screen(); //return the screen number this object is managing
	void show();
	//void hide();
	void prepareToClose();

	WId backgroundID();
	QRect availableScreenGeom();

public slots:
	void SystemLogout();
	void SystemTerminal();
	void SystemFileManager();
	void SystemApplication(QAction*);	

	void checkResolution();

private:
	QSettings *settings;
	QTimer *bgtimer;
	QDesktopWidget *desktop;
	QString DPREFIX;
	int desktopnumber;
	//int xoffset;
	bool defaultdesktop, desktoplocked, issyncing, usewinmenu, bgupdating;
	QStringList oldBGL;
	QList<LPanel*> PANELS;
	QMdiArea *bgDesktop; //desktop widget area
	QWidget *bgWindow; //full screen background
	QMenu *deskMenu, *winMenu;
	//AppMenu *appmenu;
	QLabel *workspacelabel;
	QWidgetAction *wkspaceact;
	QList<LDPlugin*> PLUGINS;
	//QFileSystemWatcher *watcher;
	QString CBG; //current background
	QRect globalWorkRect;
	void CreateDesktopPluginContainer(LDPlugin*);

private slots:
	void InitDesktop();
	void SettingsChanged();
	void UnlockSettings(){ issyncing=false; }
	//Menu functions
	void UpdateMenu(bool fast = false);
	void ShowMenu(){
	  UpdateMenu(true); //run the fast version
	  deskMenu->popup(QCursor::pos());
	}
	void UpdateWinMenu();
	void winClicked(QAction*);
	
	//Desktop plugin system functions
	void UpdateDesktop();
	void ToggleDesktopLock();
	void AlignDesktopPlugins();
	void DesktopPluginRemoved(QString);
	
	void UpdatePanels();
	
	void UpdateDesktopPluginArea(); //make sure the area is not underneath any panels

	void UpdateBackground();
	void UpdateGeometry(int screen){
	  if(screen==desktopnumber){
	    bgWindow->setGeometry(desktop->screenGeometry(desktopnumber));
	    QTimer::singleShot(0, this, SLOT(UpdatePanels()));
	  }
	}
};
#endif
