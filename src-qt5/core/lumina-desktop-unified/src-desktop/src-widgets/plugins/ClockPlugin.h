//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_PLUGIN_CLOCK_H
#define _LUMINA_DESKTOP_PLUGIN_CLOCK_H

#include <Plugin.h>
#include <RootDesktopObject.h>

class ClockPlugin : public PluginButton{
	Q_OBJECT
private slots:
	void updateTime(){
	  //qDebug() << "Clock: Update Time";
	  this->button->setText(RootDesktopObject::instance()->currentTime() );
	}

public:
	ClockPlugin(QWidget *parent, QString id, bool panelplug) : PluginButton(parent, id, panelplug){
	  connect(RootDesktopObject::instance(), SIGNAL(currentTimeChanged()), this, SLOT(updateTime()) );
	  QFont tmp = button->font();
	    tmp.setBold(true);
	  button->setFont( tmp );
	  QTimer::singleShot(0, this, SLOT(updateTime()) );
	}
	~ClockPlugin(){ }

};

#endif
