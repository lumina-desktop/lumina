//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This class is a quick sample desktop plugin
//===========================================
#ifndef _LUMINA_DESKTOP_DESKTOP_VIEW_PLUGIN_CALENDAR_H
#define _LUMINA_DESKTOP_DESKTOP_VIEW_PLUGIN_CALENDAR_H

#include <QListWidget>
#include <QVBoxLayout>
#include "../LDPlugin.h"

class CalendarPlugin : public LDPlugin{
	Q_OBJECT
public:
	DesktopViewPlugin(QWidget* parent, QString ID) : LDPlugin(parent, ID){
	  this->setLayout( new QVBoxLayout());
	    this->layout()->setContentsMargins(0,0,0,0);
	  list = new QListWidget(this);
	  this->layout()->addWidget(list);
	}
	
	~DesktopViewPlugin(){}
	
private:
	QListWidget *list;
};
#endif
