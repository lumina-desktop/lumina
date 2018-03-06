//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_SCREEN_CONFIG_DIALOG_H
#define _LUMINA_SCREEN_CONFIG_DIALOG_H

#include <QMainWindow>
#include <QRect>
#include <QString>
#include <QList>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenu>
#include <QAction>

#include "ScreenSettings.h"

namespace Ui{
	class MainUI;
};

class MainUI : public QMainWindow{
	Q_OBJECT
public:
	MainUI();
	~MainUI();

public slots:
	void slotSingleInstance(){
	  this->raise();
	  this->show();
	}
	void loadIcons();

private:
	Ui::MainUI *ui;
	QList<ScreenInfo> SCREENS;
	double scaleFactor;
	QMenu *singleTileMenu, *profilesMenu;

	ScreenInfo currentScreenInfo();

	//QStringList currentOpts();
	QString currentSelection();

	void AddScreenToWidget(ScreenInfo);
	void SyncBackend(); //sync backend structures to current settings

private slots:
	void UpdateScreens(QString profile = "");
	void ScreenSelected();
	void updateNewScreenResolutions();
	void tileScreensY(bool activeonly = false);
	void tileScreensX(bool activeonly = false);
	void tileScreens();
	void tileSingleScreen(QAction*);
	void showMenu(){ singleTileMenu->popup(QCursor::pos()); }

	void DeactivateScreen();
	void ActivateScreen();
	void ApplyChanges(); //config changes
	void SaveSettings(bool quiet = false);
	void RestartFluxbox();

	void removeProfile();
	void updateProfiles();
	void loadProfile();
	void saveAsProfile(QAction *);
};

#endif
