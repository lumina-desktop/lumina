//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_SCREEN_SAVER_H
#define _LUMINA_DESKTOP_SCREEN_SAVER_H

#include "global-includes.h"

#include "SSBaseWidget.h"
#include "LLockScreen.h"

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
	LLockScreen *LOCKER;
	int cBright;
	bool SSRunning, SSLocked, updating;

	void UpdateTimers();

public slots:
	void start();
	void reloadSettings();
	void newInputEvent();
	void LockScreenNow();

private slots:
	void ShowScreenSaver();
	void ShowLockScreen();
	void HideScreenSaver();
	void HideLockScreen();

	void LockScreen();
	void SSFinished();

signals:
	void StartingScreenSaver();
	void ClosingScreenSaver();

protected:
	void mouseMoveEvent(QMouseEvent*){
	  QTimer::singleShot(0,this, SLOT(newInputEvent()));
	}

};

#endif
