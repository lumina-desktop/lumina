//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_UTILITIES_TERMINAL_TRAY_ICON_H
#define _LUMINA_DESKTOP_UTILITIES_TERMINAL_TRAY_ICON_H
// QT Includes
#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QTimer>
#include <QSettings>

#include <LuminaXDG.h>

#include "TermWindow.h"

class TrayIcon : public QSystemTrayIcon {
   Q_OBJECT
   
public:
	TrayIcon();
	~TrayIcon();
	
	//First run
	void parseInputs(QStringList); //Note that this is only run on the primary process - otherwise it gets sent to the singleInstance slot below
	   
public slots:
	void slotSingleInstance(QStringList inputs = QStringList());
	void updateIcons();
	
private:
	bool termVisible;
	TermWindow *TERM;
	QMenu *ScreenMenu;
	QStringList adjustInputs(QStringList);
	QSettings *settings;
private slots:
	//Action Buttons
	void startCleanup();
	void stopApplication();
	void ChangeTopBottom(bool ontop);
	void ChangeScreen(QAction*);

	//Tray Updates
	void setupContextMenu();
	void updateScreenMenu();
	void TrayActivated(QSystemTrayIcon::ActivationReason);

	//Slots for the window visibility
	void ToggleVisibility();
	void TermHidden();
	void TermVisible();

};

#endif
