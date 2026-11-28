//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017-2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This is the main class that updates the interface objects
//    on-demand as settings files and other stuff changes
//===========================================
#ifndef _LUMINA_DESKTOP_OBJECT_MANAGER_H
#define _LUMINA_DESKTOP_OBJECT_MANAGER_H

#include <global-includes.h>

class DesktopManager : public QObject {
	Q_OBJECT
public:
	DesktopManager();
	~DesktopManager();

	void start();
	void stop();

private:
	void updateWallpaper(QString screen_id, int wkspace);
	void updatePlugins(QString plugin_id);

public slots:
	void workspaceChanged(int);
	void settingsChanged(DesktopSettings::File);

	void NewWindowAvailable(NativeWindowObject*);
	void NewTrayWindowAvailable(NativeWindowObject*);

	void syncWindowList();
	void syncTrayWindowList();

private slots:
	void updateSessionSettings();
	void updateDesktopSettings();
	void updatePanelSettings();
	void updatePanelReservations();
	void updatePluginSettings();
	void updateMenuSettings();
	void updateAnimationSettings();

signals:
	void PanelLocationsChanged(); //reserved screen space changed
};

#endif
