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
#include <QDateTime>
#include <QTransform>
#include <QMouseEvent>
#include <QTabWidget>

#include <LuminaXDG.h>
#include <LuminaOS.h>
#include "UserItemWidget.h"

#define SSAVER QString("xscreensaver-demo")

namespace Ui{
	class UserWidget;
};

class UserWidget : public QTabWidget{
	Q_OBJECT
public:
	UserWidget(QWidget *parent=0);
	~UserWidget();

public slots:
	void UpdateAll(); //for re-translation/icon changes
	void UpdateMenu(bool forceall = false); //for item changes

private:
	Ui::UserWidget *ui;
	QHash<QString, QList<XDGDesktop*> > *sysapps;
	QDateTime lastUpdate, lastHomeUpdate;
	QStringList favs;
	QFileInfoList homefiles;
	int cfav; //current favorite category
	void ClearScrollArea(QScrollArea *area);
	void SortScrollArea(QScrollArea *area);
	QIcon rotateIcon(QIcon);
	bool updatingfavs;

private slots:
	void LaunchItem(QString path, bool fix = true);

	//Favorites Tab
	void FavChanged(); //for ensuring radio-button-like behaviour
	void updateFavItems(bool newfilter = true); //if false, will only update if filesystem changes

	//Apps Tab
	void updateAppCategories();
	void updateApps();

	//Home Tab
	void updateHome();
	void slotGoToDir(QString dir);
	void slotGoHome();
	void slotOpenDir();
	void slotOpenSearch();

	//Slots for the special buttons
	void openStore(){
	  LaunchItem(LOS::AppStoreShortcut());
	}
	void openControlPanel(){
	  LaunchItem(LOS::ControlPanelShortcut());
	}
	void openDeskSettings(){
	  LaunchItem("lumina-config", false);
	}
	void openScreenSaverConfig(){
	  LaunchItem(SSAVER, false);
	}
	void openScreenConfig(){
	  LaunchItem("lumina-xconfig",false);
	}
	void openLuminaInfo(){
	  LaunchItem("lumina-info",false);
	}
	
protected:
	void mouseMoveEvent( QMouseEvent *event);

signals:
	void CloseMenu();

};

#endif
