//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// Simple menu which shows icons for all the system tray apps
//===========================================
#ifndef _LUMINA_DESKTOP_WIDGET_MENU_SYSTEMTRAY_H
#define _LUMINA_DESKTOP_WIDGET_MENU_SYSTEMTRAY_H

#include <global-includes.h>
class TrayIcon : public QLabel {
	Q_OBJECT
private:
	NativeWindowObject *WIN;

public:
	TrayIcon(QWidget *parent, NativeWindowObject *win);
	~TrayIcon();

private slots:
	void updateIcon();
	void aboutToShow();

};

class SystemTrayMenu : public QMenu {
	Q_OBJECT
private:
	QWidget *widget;
	QWidgetAction *WA;
	QGridLayout *layout;
	QList<TrayIcon*> TIcons;

public:
	SystemTrayMenu();
	~SystemTrayMenu();

	int numTrayIcons();

private slots:
	void trayWindowsChanged();

signals:
	void hasTrayIcons(bool);

};
#endif
