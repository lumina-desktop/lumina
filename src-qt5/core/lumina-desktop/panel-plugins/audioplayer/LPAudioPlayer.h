//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_PANEL_AUDIO_PLAYER_PLUGIN_H
#define _LUMINA_PANEL_AUDIO_PLAYER_PLUGIN_H

#include "../../Globals.h"
#include "../LTBWidget.h"
#include "../LPPlugin.h"
#include "PPlayerWidget.h"

class LPAudioPlayer : public LPPlugin{
	Q_OBJECT
public:
	LPAudioPlayer(QWidget *parent = 0, QString id = "audioplayer", bool horizontal=true);
	~LPAudioPlayer();
	
private:
	QToolButton *button;
	QWidgetAction *wact;
	PPlayerWidget *aplayer;

	//int iconOld;
	
private slots:
	//void updateBattery(bool force = false);
	//QString getRemainingTime();

public slots:
	void LocaleChange(){
	  //updateBattery(true);
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
