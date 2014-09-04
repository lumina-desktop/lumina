//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_SETTINGS_MENU_H
#define _LUMINA_DESKTOP_SETTINGS_MENU_H

#include <QMenu>
#include <QProcess>
#include <QAction>

#include <LuminaXDG.h>

class SettingsMenu : public QMenu{
	Q_OBJECT
public:
	SettingsMenu();
	~SettingsMenu();
		
private slots:
	void runApp(QAction* act);

};

#endif