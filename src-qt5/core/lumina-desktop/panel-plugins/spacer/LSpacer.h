//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This class is a generic invisible spacer for the panel
//===========================================
#ifndef _LUMINA_DESKTOP_PANEL_PLUGIN_SPACER_H
#define _LUMINA_DESKTOP_PANEL_PLUGIN_SPACER_H

#include "../LPPlugin.h"

class LSpacerPlugin : public LPPlugin{
	Q_OBJECT
public:
	LSpacerPlugin(QWidget *parent=0, QString id="spacer", bool horizontal=true) : LPPlugin(parent, id, horizontal){
	  if(horizontal){ this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred); }
	  else{ this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding); }
	}
	~LSpacerPlugin(){}
		
public slots:
	void OrientationChange(){
	  if(this->layout()->direction()==QBoxLayout::LeftToRight){ //horizontal
	    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	  }else{ //vertical
	    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	  }
	}
};


#endif