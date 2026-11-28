//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This menu auto-updates to keep the list of available applications
//    current at all times - and can launch them as necessary
//===========================================
#ifndef _LUMINA_DESKTOP_APP_MENU_H
#define _LUMINA_DESKTOP_APP_MENU_H

// Qt includes
#include <QMenu>
#include <QFileSystemWatcher>
#include <QString>
#include <QList>
#include <QTimer>
#include <QDateTime>
#include <QHash>
#include <QAction>
#include <QSettings>
//#include <QProcess>

// libLumina includes
#include <LuminaXDG.h>

class AppMenu : public QMenu{
	Q_OBJECT
public:
	AppMenu(QWidget *parent = 0);
	~AppMenu();

	QHash<QString, QList<XDGDesktop*> > *currentAppHash();
	QDateTime lastHashUpdate;

private:
	//QFileSystemWatcher *watcher;
	QString appstorelink, controlpanellink;
	QList<QMenu> MLIST;
	XDGDesktopList *sysApps;
	QHash<QString, QList<XDGDesktop*> > APPS;

	void updateAppList(); //completely update the menu lists

private slots:
	void start(); //This is called in a new thread after initialization
	void watcherUpdate();
	void launchStore();
	void launchControlPanel();
	void launchFileManager();
	void launchApp(QAction *act);

signals:
	void AppMenuUpdated();
};

#endif
