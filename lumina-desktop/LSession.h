//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_SESSION_H
#define _LUMINA_DESKTOP_SESSION_H

#include <QApplication>
#include <QDebug>
#include <QString>
#include <QX11Info>
#include <QEvent>
#include <QTranslator>
#include <QSettings>
#include <QProxyStyle>
#include <QDesktopWidget>
#include <QList>
#include <QThread>
#include <QMediaPlayer>
#include <QThread>
#include <QUrl>

#include "Globals.h"
#include "AppMenu.h"
#include "SettingsMenu.h"
#include "SystemWindow.h"
#include "LDesktop.h"
#include "WMProcess.h"

#include <LuminaX11.h>

//SYSTEM TRAY STANDARD DEFINITIONS
#define SYSTEM_TRAY_REQUEST_DOCK 0
#define SYSTEM_TRAY_BEGIN_MESSAGE 1
#define SYSTEM_TRAY_CANCEL_MESSAGE 2

/*class MenuProxyStyle : public QProxyStyle{
public: 
	int pixelMetric(PixelMetric metric, const QStyleOption *option=0, const QWidget *widget=0) const{
	  if(metric==PM_SmallIconSize){ return 22; } //override QMenu icon size (make it larger)
	  else{ return QProxyStyle::pixelMetric(metric, option, widget); } //use the current style for everything else
	}
};*/

class LSession : public QApplication{
	Q_OBJECT
public:
	LSession(int &argc, char **argv);
	~LSession();
	//Functions to be called during startup
	void setupSession();

	//Public System Tray Functions
	QList<WId> currentTrayApps(WId visualTray);
	bool registerVisualTray(WId);
	void unregisterVisualTray(WId);

	//Special functions for XCB event filter parsing only 
	//  (DO NOT USE MANUALLY)
	void WindowPropertyEvent();
        void WindowPropertyEvent(WId);
	void SysTrayDockRequest(WId);
	void WindowClosedEvent(WId);
	void WindowConfigureEvent(WId);
	void WindowDamageEvent(WId);
	void WindowSelectionClearEvent(WId);
	
	//System Access
	//Return a pointer to the current session
	static LSession* handle(){
	  return static_cast<LSession*>(LSession::instance());
	}
	
	static void LaunchApplication(QString cmd);
	QFileInfoList DesktopFiles();
	
	AppMenu* applicationMenu();
	void systemWindow();
	SettingsMenu* settingsMenu();
	LXCB *XCB; //class for XCB usage
	
	QSettings* sessionSettings();
	QSettings* DesktopPluginSettings();
	
	//Temporarily change the session locale (nothing saved between sessions)
	void switchLocale(QString localeCode);
	
	//Play System Audio
	void playAudioFile(QString filepath);
	//Window Adjustment Routine (due to Fluxbox not respecting _NET_WM_STRUT)
	void adjustWindowGeom(WId win, bool maximize = false);
	
private:
	WMProcess *WM;
	QList<LDesktop*> DESKTOPS;
	QFileSystemWatcher *watcher;

	//Internal variable for global usage
	AppMenu *appmenu;
	SettingsMenu *settingsmenu;
	SystemWindow *sysWindow;
	QTranslator *currTranslator;
	QMediaPlayer *mediaObj;
	QSettings *sessionsettings, *DPlugSettings;
	bool cleansession;

	//System Tray Variables
	WId SystemTrayID, VisualTrayID;
	int TrayDmgEvent, TrayDmgError;
	QList<WId> RunningTrayApps;
	bool TrayStopping;

	//Task Manager Variables
	QList<WId> RunningApps;
	QList<WId> checkWin;
	QFileInfoList desktopFiles;

	void CleanupSession();
	
	int VersionStringToNumber(QString version);
	
public slots:
	void launchStartupApps();
	void StartLogout();
	void StartShutdown();
	void StartReboot();

private slots:
	void watcherChange(QString);
	void checkWindowGeoms();

	//System Tray Functions
	void startSystemTray();
	void stopSystemTray(bool detachall = false);
	void attachTrayWindow(WId);
	void removeTrayWindow(WId);

	//Internal simplification functions
	void checkUserFiles();
	void refreshWindowManager();
	void updateDesktops();
	void registerDesktopWindows();


	void SessionEnding();

signals:
	//System Tray Signals
	void VisualTrayAvailable(); //new Visual Tray Plugin can be registered
	void TrayListChanged(); //Item added/removed from the list
	void TrayIconChanged(WId); //WinID of Tray App
	//Task Manager Signals
	void WindowListEvent(WId);
	void WindowListEvent();
	//General Signals
	void LocaleChanged();
	void IconThemeChanged();
	void DesktopConfigChanged();
	void SessionConfigChanged();
	void DesktopFilesChanged();
	
};

#endif
