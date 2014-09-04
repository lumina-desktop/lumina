//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This panel plugin allows the user to quickly access user favorites and applications
//===========================================
#ifndef _LUMINA_PANEL_USER_BUTTON_WIDGET_H
#define _LUMINA_PANEL_USER_BUTTON_WIDGET_H

#include <QWidget>
#include <QString>
#include <QList>
#include <QHash>
#include <QVBoxLayout>
#include <QScrollArea>

#include <LuminaXDG.h>

#include "UserItemWidget.h"

#define APPSTORE QString("/usr/local/share/applications/softmanager.desktop")
#define CONTROLPANEL QString("/usr/local/share/applications/pccontrol.desktop")
#define QTCONFIG QString("/usr/local/bin/qtconfig-qt4")
#define SSAVER QString("xscreensaver-demo")

namespace Ui{
	class UserWidget;
};

class UserWidget : public QWidget{
	Q_OBJECT
public:
	UserWidget(QWidget *parent=0);
	~UserWidget();

	void UpdateMenu();

private:
	Ui::UserWidget *ui;
	QHash<QString, QList<XDGDesktop> > *sysapps;
	int cfav; //current favorite category
	void ClearScrollArea(QScrollArea *area);

private slots:
	void LaunchItem(QString cmd);

	//Favorites Tab
	void FavChanged(); //for ensuring radio-button-like behaviour
	void updateFavItems();

	//Apps Tab
	void updateAppCategories();
	void updateApps();
	
	//Home Tab
	void updateHome();

	//Slots for the special buttons
	void openStore(){
	  LaunchItem("lumina-open \""+APPSTORE+"\"");
	}
	void openControlPanel(){
	  LaunchItem("lumina-open \""+CONTROLPANEL+"\"");
	}
	void openDeskSettings(){
	  LaunchItem("lumina-config");
	}
	void openQtConfig(){
	  LaunchItem(QTCONFIG);
	}
	void openScreenSaverConfig(){
	  LaunchItem(SSAVER);
	}

signals:
	void CloseMenu();

};

#endif