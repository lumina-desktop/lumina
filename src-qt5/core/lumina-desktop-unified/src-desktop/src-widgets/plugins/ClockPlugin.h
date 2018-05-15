//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include <Plugin.h>
#include <RootDesktopObject.h>

class ClockPlugin : public PluginButton{
	Q_OBJECT
private:

public:
	ClockPlugin(QWidget *parent, QString id, bool panelplug) : PluginButton(parent, id, panelplug){
	  connect(RootDesktopObject::instance(), SIGNAL(currentTimeChanged()), this, SLOT(updateTime()) );
	}
	~ClockPlugin(){ }

private slots:
	void updateTime(){
	  this->button->setText(RootDesktopObject::instance()->currentTime() );
	}

};
