//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_PANEL_JSON_PLUGIN_H
#define _LUMINA_PANEL_JSON_PLUGIN_H

#include "../../Globals.h"
#include "../LPPlugin.h"


class LPJsonMenu : public LPPlugin{
	Q_OBJECT
public:
	LPJsonMenu(QWidget *parent = 0, QString id = "jsonmenu", bool horizontal=true);
	~LPJsonMenu();
	
private:
	QToolButton *button;
	
private slots:
	//void SystemApplication(QAction*);

public slots:
	void LocaleChange(){
	}
	
	void OrientationChange(){
	  if(this->layout()->direction()==QBoxLayout::LeftToRight){
	    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
	    button->setIconSize( QSize(this->height(), this->height()) );
	  }else{
	    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
	    button->setIconSize( QSize(this->width(), this->width()) );
	  }
	  this->layout()->update();
	}
};

#endif
