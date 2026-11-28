//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This class is a generic line separator for the panel
//===========================================
#ifndef _LUMINA_DESKTOP_PANEL_PLUGIN_LINE_H
#define _LUMINA_DESKTOP_PANEL_PLUGIN_LINE_H

#include "../LPPlugin.h"
#include <QFrame>

class LLinePlugin : public LPPlugin{
	Q_OBJECT
private:
	QFrame *line;

public:
	LLinePlugin(QWidget *parent=0, QString id="spacer", bool horizontal=true) : LPPlugin(parent, id, horizontal){
	  line = new QFrame(this);
	  line->setObjectName("LuminaPanelLine");
	  this->layout()->addWidget(line);
	  OrientationChange();
	}
	~LLinePlugin(){}
		
public slots:
	void OrientationChange(){
	  if(this->layout()->direction()==QBoxLayout::LeftToRight){ //horizontal
	    line->setFrameShape(QFrame::VLine);
	  }else{ //vertical
	    line->setFrameShape(QFrame::HLine);
	  }
	}
};


#endif