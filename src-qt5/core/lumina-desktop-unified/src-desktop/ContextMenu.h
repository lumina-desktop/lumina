//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2012-2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_CONTEXT_MENU_H
#define _LUMINA_DESKTOP_CONTEXT_MENU_H

#include <global-includes.h>

class DesktopContextMenu : public QMenu{
	Q_OBJECT
public slots:
	void SettingsChanged(DesktopSettings::File);
	void UpdateMenu(); //re-create the menu

private:
	QLabel *workspaceLabel;
	QWidgetAction *wkspaceact;

public:
	DesktopContextMenu(QWidget *parent = 0);
	~DesktopContextMenu();

	void start(); //setup connections to global objects

private slots:
	void LaunchAction(QAction *act);
	void showMenu(const QPoint&);

signals:
	void LockSession();
	void showLeaveDialog();
};

#endif
