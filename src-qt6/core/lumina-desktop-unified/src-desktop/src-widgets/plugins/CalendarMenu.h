//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// Simple menu popup with a calendar in it
//===========================================
#ifndef _LUMINA_DESKTOP_WIDGET_MENU_CALENDAR_H
#define _LUMINA_DESKTOP_WIDGET_MENU_CALENDAR_H

#include <QMenu>
#include <QWidgetAction>
#include <QCalendarWidget>
#include <QDate>

class CalendarMenu : public QMenu {
	Q_OBJECT
private:
	QCalendarWidget *cal;
	QWidgetAction *WA;
public:
	CalendarMenu() : QMenu(){
	  cal = new QCalendarWidget();
	  WA = new QWidgetAction(this);
	    WA->setDefaultWidget(cal);
	  this->addAction(WA);
	  connect(this, SIGNAL(aboutToShow()), this, SLOT(resetDate()) );
	}
	~CalendarMenu(){
	  cal->deleteLater();
	}

private slots:
	void resetDate(){
	  cal->setSelectedDate(QDate::currentDate());
	}

};
#endif
