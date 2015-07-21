//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This panel plugin allows the user to control different system settings
//    For example: screen brightness, audio volume, workspace, and battery
//===========================================
#ifndef _LUMINA_PANEL_QUICK_SYSTEM_MENU_H
#define _LUMINA_PANEL_QUICK_SYSTEM_MENU_H

#include <QWidget>
#include <QSettings>
#include <QString>

#include <LuminaOS.h>
#include <LuminaXDG.h>

namespace Ui{
	class LSysMenuQuick;
};

class LSysMenuQuick : public QWidget{
	Q_OBJECT
public:
	LSysMenuQuick(QWidget *parent=0);
	~LSysMenuQuick();

	void UpdateMenu();

private:
	Ui::LSysMenuQuick *ui;
	QTimer *brighttimer;
	bool firstrun, hasBat;
	QString getRemainingTime(); //battery time left

private slots:
	void volSliderChanged();
	void brightSliderChanged(); //start the delay/collection timer
	void setCurrentBrightness(); //perform the change
	void startMixer();
	void nextWorkspace();
	void prevWorkspace();
	void startLogout();
	void changeLocale();


signals:
	void CloseMenu();

};

#endif