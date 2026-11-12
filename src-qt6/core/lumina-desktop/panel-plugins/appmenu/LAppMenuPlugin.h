//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This panel plugin is a re-creation of the classic "start" menu
//===========================================
#ifndef _LUMINA_DESKTOP_APP_MENU_PANEL_PLUGIN_H
#define _LUMINA_DESKTOP_APP_MENU_PANEL_PLUGIN_H

// Qt includes
#include <QMenu>
#include <QToolButton>
#include <QString>
#include <QWidget>


// Lumina-desktop includes
#include "../LPPlugin.h" //main plugin widget


// PANEL PLUGIN BUTTON
class LAppMenuPlugin : public LPPlugin{
	Q_OBJECT
	
public:
	LAppMenuPlugin(QWidget *parent = 0, QString id = "appmenu", bool horizontal=true);
	~LAppMenuPlugin();
	
private:
	QToolButton *button;
	QMenu *mainmenu;

	void updateButtonVisuals();

private slots:
	void shortcutActivated();
	void LaunchItem(QAction* item);
	void UpdateMenu();

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
