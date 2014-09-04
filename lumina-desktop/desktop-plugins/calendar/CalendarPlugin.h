//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This class is a quick sample desktop plugin
//===========================================
#ifndef _LUMINA_DESKTOP_DESKTOP_PLUGIN_CALENDAR_H
#define _LUMINA_DESKTOP_DESKTOP_PLUGIN_CALENDAR_H

#include <QCalendarWidget>
#include <QVBoxLayout>
#include "../LDPlugin.h"

class CalendarPlugin : public LDPlugin{
	Q_OBJECT
public:
	CalendarPlugin(QWidget* parent, QString ID) : LDPlugin(parent, ID){
	  this->setLayout( new QVBoxLayout());
	    this->layout()->setContentsMargins(0,0,0,0);
	  cal = new QCalendarWidget(this);
	  this->layout()->addWidget(cal);
	}
	
	~CalendarPlugin(){}
	
private:
	QCalendarWidget *cal;
};
#endif
