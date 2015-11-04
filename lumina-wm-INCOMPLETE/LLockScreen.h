//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_LOCK_SCREEN_WIDGET_H
#define _LUMINA_DESKTOP_LOCK_SCREEN_WIDGET_H

#include "GlobalDefines.h"

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

	bool PAM_checkpass(QString user, QString pass, QString &info);
	QString PAM_checkLockInfo(QString user); //info string with any password reset countdown info
	QString PAM_getError(int ret);
	bool PAM_lockSession(QString user);

private slots:
	void TryUnlock();
	
signals:
	void ScreenUnlocked();
	void InputDetected();
};
#endif
