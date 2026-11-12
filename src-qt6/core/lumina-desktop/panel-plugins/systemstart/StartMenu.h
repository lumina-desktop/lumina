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
#include <QMouseEvent>
#include <QMutex>

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
	QStringList favs;
	QString CCat, CSearch, topsearch; //current category/search
	QTimer *searchTimer;
	QMutex searchmutex;

	//Simple utility functions
	//void deleteChildren(QWidget *obj); //recursive function
	void ClearScrollArea(QScrollArea *area);
	void SortScrollArea(QScrollArea *area);
	void do_search(QString search, bool force, Ui::StartMenu *tui);

	bool promptAboutUpdates(bool &skip);

private slots:
	void LaunchItem(QString path, bool fix = true);

	//Application/Favorite Listings
	void ChangeCategory(QString cat);
	void UpdateApps();
	void UpdateFavs();

	// Page update routines
	void on_stackedWidget_currentChanged(int); //page changed
	void catViewChanged(); //application categorization view mode changed

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
	void on_tool_restart_updates_clicked();
	void on_tool_shutdown_clicked();
	void on_tool_suspend_clicked();

	//Audio Volume
	void on_slider_volume_valueChanged(int);
	void on_tool_launch_mixer_clicked();
	void on_tool_mute_audio_clicked();

	//Screen Brightness
	void on_slider_bright_valueChanged(int);

	//Workspace
	void on_tool_set_nextwkspace_clicked();
	void on_tool_set_prevwkspace_clicked();

	//Locale
	void on_combo_locale_currentIndexChanged(int);

	//Search
	void on_line_search_textEdited(QString);
        void startSearch();
	void on_line_search_returnPressed();

signals:
	void CloseMenu();
	void UpdateQuickLaunch(QStringList);

};

#endif
