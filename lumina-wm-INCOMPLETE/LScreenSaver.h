//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_SCREEN_SAVER_H
#define _LUMINA_DESKTOP_SCREEN_SAVER_H

#include <QWidget>
#include <QStringList>
#include <QTimer>
#include <QSettings>

#include "SSBaseWidget.h"

class LScreenSaver : public QWidget{
	Q_OBJECT
public:
	LScreenSaver();
	~LScreenSaver();

	bool isLocked();

private:
	QTimer *starttimer, *locktimer, *hidetimer;
	QSettings *settings;
	QList<SSBaseWidget*> BASES;
	
	bool SSRunning, SSLocked;

public slots:
	void start();
	void reloadSettings();
	void newInputEvent();

private slots:
	void ShowScreenSaver();
	void ShowLockScreen();
	void HideScreenSaver();
	void HideLockScreen();

signals:
	void StartingScreenSaver();
	void ClosingScreenSaver();

};

#endif