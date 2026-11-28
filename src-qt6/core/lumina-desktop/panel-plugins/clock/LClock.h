//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_CLOCK_H
#define _LUMINA_DESKTOP_CLOCK_H

#include <QTimer>
#include <QDateTime>
#include <QLabel>
#include <QWidget>
#include <QString>
#include <QLocale>
#include <QTimeZone>
#include <QCalendarWidget>
#include <QWidgetAction>
#include <QAction>
#include <QToolButton>
#include <QMenu>

#include "../LPPlugin.h"

//#include "../RotateToolButton.h"

class LClock : public LPPlugin{
	Q_OBJECT
public:
	LClock(QWidget *parent = 0, QString id = "clock", bool horizontal=true);
	~LClock();
	
private:
	QTimer *timer;
	QToolButton *button; //RotateToolButton
	QString timefmt, datefmt, datetimeorder;
	bool deftime, defdate;
	QMenu *TZMenu;
	QCalendarWidget *calendar;
	QWidgetAction *calAct;
	
private slots:
	void updateTime(bool adjustformat = false);
	void updateFormats();

	void updateMenu();
	void openMenu();
	void closeMenu();

	void ChangeTZ(QAction*);

public slots:
	void LocaleChange();
	void ThemeChange();
	void OrientationChange();
};

#endif
