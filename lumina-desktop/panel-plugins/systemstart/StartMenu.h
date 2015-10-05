//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_SYSTEM_START_PANEL_PLUGIN_H
#define _LUMINA_DESKTOP_SYSTEM_START_PANEL_PLUGIN_H

#include <QWidget>
#include <QScrollArea>

#include <LuminaXDG.h>

namespace Ui{
	class StartMenu;
};

class StartMenu : public QWidget{
	Q_OBJECT
public:
	StartMenu(QWidget *parent = 0);
	~StartMenu();

public slots:
	void UpdateAll(); //for re-translation/icon changes
	void UpdateMenu(bool forceall = false); //for item changes

	void ReLoadQuickLaunch();
	void UpdateQuickLaunch(QString, bool);

private:
	Ui::StartMenu *ui;
	QHash<QString, QList<XDGDesktop> > *sysapps;
	QStringList favs;

	//Simple utility functions
	void ClearScrollArea(QScrollArea *area);
	void SortScrollArea(QScrollArea *area);
	
private slots:
	void LaunchItem(QString path, bool fix = true);

	//Application/Favorite Listings
	void UpdateApps();
	void UpdateFavs();

	// Page update routines
	void on_stackedWidget_currentChanged(int); //page changed

	//Page Change Buttons
	void on_tool_goto_apps_clicked();
	void on_tool_goto_settings_clicked();
	void on_tool_goto_logout_clicked();
	void on_tool_back_clicked();

	//Launch Buttons
	void on_tool_launch_controlpanel_clicked();
	void on_tool_launch_fm_clicked();
	void on_tool_launch_store_clicked();
	void on_tool_launch_desksettings_clicked();
	void on_tool_launch_deskinfo_clicked();

	//Logout Buttons
	void on_tool_lock_clicked();
	void on_tool_logout_clicked();
	void on_tool_restart_clicked();
	void on_tool_shutdown_clicked();
	void on_tool_suspend_clicked();

	//Audio Volume
	void on_slider_volume_valueChanged(int);
	void on_tool_launch_mixer_clicked();
	
	//Screen Brightness
	void on_slider_bright_valueChanged(int);
	
	//Workspace
	void on_tool_set_nextwkspace_clicked();
	void on_tool_set_prevwkspace_clicked();
	
	//Locale
	void on_combo_locale_currentIndexChanged(int);
	
	//Search
	void on_line_search_editingFinished();

signals:
	void CloseMenu();
	void UpdateQuickLaunch(QStringList);
	
};

#endif