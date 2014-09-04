//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_TASK_BUTTON_H
#define _LUMINA_DESKTOP_TASK_BUTTON_H

// Qt includes
#include <QWidget>
#include <QList>
#include <QIcon>
#include <QCursor>
#include <QMenu>
#include <QEvent>
#include <QAction>

// libLumina includes
#include <LuminaXDG.h>
#include <LuminaX11.h>

// Local includes
#include "LWinInfo.h"
#include "../LTBWidget.h"

class LTaskButton : public LTBWidget{
	Q_OBJECT
public:
	LTaskButton(QWidget *parent=0);
	~LTaskButton();
	
	//Window Information
	QList<LWinInfo> windows();
	QString classname();

	//Window Management
	void addWindow(LWinInfo win); //Add a window to this button
	void rmWindow(LWinInfo win); //Remove a window from this button

private:
	QList<LWinInfo> WINLIST;
	QList<LWinInfo> LWINLIST;
	QMenu *actMenu; // action menu (custom context menu)
	QMenu *winMenu; // window menu (if more than 1)
	LWinInfo cWin;
	QString cname; //class name for the entire button
	bool noicon;

	LWinInfo currentWindow(); //For getting the currently-active window

public slots:
	void UpdateButton(); //re-sync the current window infomation
	void UpdateMenus(); //re-create the menus (text + icons)

private slots:
	void buttonClicked();
	void closeWindow(); //send the signal to close a window
	void triggerWindow(); //change b/w visible and invisible
	void winClicked(QAction*);
	void openActionMenu();
 
};
#endif
