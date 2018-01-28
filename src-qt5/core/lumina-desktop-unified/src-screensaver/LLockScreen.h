//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_LOCK_SCREEN_WIDGET_H
#define _LUMINA_DESKTOP_LOCK_SCREEN_WIDGET_H

#include "global-includes.h"

namespace Ui{
	class LLockScreen;
};

class LLockScreen : public QWidget{
	Q_OBJECT
public:
	LLockScreen(QWidget *parent = 0);
	~LLockScreen();

	void LoadSystemDetails(); //Run right after the screen is initially locked

public slots:
	void aboutToHide(); //auto-hide timeout (can happen multiple times per lock)
	void aboutToShow(); //about to be re-shown (can happen multiple times per lock)

private:
	Ui::LLockScreen *ui;
	int triesleft, attempts;
	QTimer *waittime;
	QTimer *refreshtime;

private slots:
	void UpdateLockInfo();
	void TryUnlock();
	
signals:
	void ScreenUnlocked();
	void InputDetected();
};
#endif
