//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_PLUGIN_SPACER_H
#define _LUMINA_DESKTOP_PLUGIN_SPACER_H
#include <Plugin.h>

class SpacerPlugin : public Plugin{
	Q_OBJECT

public:
	SpacerPlugin(QWidget *parent, QString id, bool panelplug, bool vertical) : Plugin(parent, id, panelplug, vertical){}
	~SpacerPlugin(){ }

	//Replace the virtual sizing function
	void setupSizing(){
	  this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	}
};

#endif
