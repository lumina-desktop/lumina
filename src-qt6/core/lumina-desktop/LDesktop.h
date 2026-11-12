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
#include <QRegion>
#include <QInputDialog>


#include <LuminaXDG.h>

#include "LPanel.h"
//#include "Globals.h"
#include "AppMenu.h"
#include "LDesktopPluginSpace.h"
#include "desktop-plugins/LDPlugin.h"
//#include "desktop-plugins/NewDP.h"
#include "LDesktopBackground.h"

class LDesktop : public QObject{
	Q_OBJECT
public:
	LDesktop(int deskNum=0, bool setdefault = false);
	~LDesktop();
	
	int Screen(); //return the screen number this object is managing
	void show();
	void hide();
	void prepareToClose();

	WId backgroundID();
	QRect availableScreenGeom();

	void UpdateGeometry();
	
public slots:
	void SystemLock();
	void SystemLogout();
	void SystemTerminal();
	void SystemFileManager();
	void SystemApplication(QAction*);	

	void checkResolution();

private:
	QSettings *settings;
	QTimer *bgtimer;
	//QDesktopWidget *desktop;
	QString DPREFIX, screenID;
	//int desktopnumber;
	QRegion availDPArea;
	bool defaultdesktop, issyncing, usewinmenu, bgupdating;
	QStringList oldBGL;
	QList<LPanel*> PANELS;
	LDesktopPluginSpace *bgDesktop; //desktop plugin area
	//QWidget *bgWindow; //full screen background
	QMenu *deskMenu, *winMenu, *desktopFolderActionMenu;
	QLabel *workspacelabel;
	QWidgetAction *wkspaceact;
	QList<LDPlugin*> PLUGINS;
	QString CBG; //current background
	QRect globalWorkRect;
	bool i_dlg_folder; //folder/file switch
	QInputDialog *inputDLG;	

private slots:
	void InitDesktop();
	void SettingsChanged();
	void UnlockSettings(){ issyncing=false; }
	void LocaleChanged();
	
	//Menu functions
	void UpdateMenu(bool fast = false);
	void ShowMenu(){
	  UpdateMenu(true); //run the fast version
	  deskMenu->popup(QCursor::pos()); //}
	}
	void UpdateWinMenu();
	void winClicked(QAction*);
	
	//Desktop plugin system functions
	void UpdateDesktop();
	void RemoveDeskPlugin(QString);
	void IncreaseDesktopPluginIcons();
	void DecreaseDesktopPluginIcons();
	
	void UpdatePanels();
	
	void UpdateDesktopPluginArea(); //make sure the area is not underneath any panels

	void UpdateBackground();

	//Desktop Folder Interactions
	void i_dlg_finished(int ret);
	void NewDesktopFolder(QString name = "");
	void NewDesktopFile(QString name = "");
	void PasteInDesktop();
	
};
#endif
