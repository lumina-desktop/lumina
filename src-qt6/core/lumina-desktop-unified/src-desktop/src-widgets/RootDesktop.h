//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_WIDGETS_ROOT_DESKTOP_H
#define _LUMINA_DESKTOP_WIDGETS_ROOT_DESKTOP_H

#include <global-includes.h>
#include "ContextMenu.h"

class RootDesktop : public QWidget{
	Q_OBJECT
public:
	RootDesktop(QWindow *root);
	~RootDesktop();

	void start();

private:
	QImage bgimage;
	QStringList lastscreens, lastpanels;
	QTimer *bgTimer;
	DesktopContextMenu *cmenu;

private slots:
	//RootDesktopObject connections
	void on_screensChanged();
	void on_panelsChanged();
	void on_windowsChanged();
	void on_trayWindowsChanged();
	void on_screen_bg_changed();

	//Internal use
	void bgTimerUpdate();
	void updateBG(RootDesktop* obj); //designed to be run in a background thread

protected:
	virtual void paintEvent(QPaintEvent *ev);

};

#endif
