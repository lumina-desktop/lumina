//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This panel plugin is a simple button to hide all windows so the desktop is visible
//===========================================
#ifndef _LUMINA_DESKTOP_GO_HOME_PLUGIN_H
#define _LUMINA_DESKTOP_GO_HOME_PLUGIN_H

// Qt includes
#include <QToolButton>
#include <QString>
#include <QWidget>


// Lumina-desktop includes
#include "../LPPlugin.h" //main plugin widget

// libLumina includes
#include "LuminaXDG.h"

// PANEL PLUGIN BUTTON
class LHomeButtonPlugin : public LPPlugin{
	Q_OBJECT
	
public:
	LHomeButtonPlugin(QWidget *parent = 0, QString id = "homebutton", bool horizontal=true);
	~LHomeButtonPlugin();
	
private:
	QToolButton *button;

	void updateButtonVisuals();

private slots:
	void showDesktop();

public slots:
	void OrientationChange(){
	  if(this->layout()->direction()==QBoxLayout::LeftToRight){
	    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
	    button->setIconSize( QSize(this->height(), this->height()) );
	  }else{
	    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
	    button->setIconSize( QSize(this->width(), this->width()) );
	  }
	  this->layout()->update();
	  updateButtonVisuals();
	}
	
	void LocaleChange(){ 
	  updateButtonVisuals();
	}
	
	void ThemeChange(){
	  updateButtonVisuals();
	}
};

#endif