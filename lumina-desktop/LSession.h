//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012, Ken Moore
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

#include "Globals.h"
#include "AppMenu.h"
#include "SettingsMenu.h"
#include "SystemWindow.h"
#include "LDesktop.h"
#include "WMProcess.h"

//LibLumina X11 class
#include <LuminaX11.h>

//SYSTEM TRAY STANDARD DEFINITIONS
//#define SYSTEM_TRAY_REQUEST_DOCK 0
//#define SYSTEM_TRAY_BEGIN_MESSAGE 1
//#define SYSTEM_TRAY_CANCEL_MESSAGE 2

class MenuProxyStyle : public QProxyStyle{
public: 
	int pixelMetric(PixelMetric metric, const QStyleOption *option=0, const QWidget *widget=0) const{
	  if(metric==PM_SmallIconSize){ return 22; } //override QMenu icon size (make it larger)
	  else{ return QProxyStyle::pixelMetric(metric, option, widget); } //use the current style for everything else
	}
};

class LSession : public QApplication{
	Q_OBJECT
public:
	LSession(int &argc, char **argv);
	~LSession();
	//Functions to be called during startup
	void setupSession();

	virtual bool x11EventFilter(XEvent *event);
	
	bool LoadLocale(QString);

	//System Access
	static void LaunchApplication(QString cmd);
	static AppMenu* applicationMenu();
	static void systemWindow();
	static SettingsMenu* settingsMenu();

	static QSettings* sessionSettings();

	//Play System Audio
	static void playAudioFile(QString filepath);

private:
	WMProcess *WM;
	QList<LDesktop*> DESKTOPS;
	QFileSystemWatcher *watcher;

public slots:
	void launchStartupApps();


private slots:
	void watcherChange(QString);

	//Internal simplification functions
	void checkUserFiles();
	void loadStyleSheet();
	void refreshWindowManager();
	void updateDesktops();

	void SessionEnding();

signals:
	void NewSystemTrayApp(WId); //WinID
	void TrayEvent(XEvent*);
	void WindowListEvent(WId);
	void WindowListEvent();
	void LocaleChanged();
	void DesktopConfigChanged();
	
};

#endif
