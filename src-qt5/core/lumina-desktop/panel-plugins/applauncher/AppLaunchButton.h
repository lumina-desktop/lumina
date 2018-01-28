//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This panel plugin is a simple button to launch a single application
//===========================================
#ifndef _LUMINA_DESKTOP_LAUNCH_APP_PANEL_PLUGIN_H
#define _LUMINA_DESKTOP_LAUNCH_APP_PANEL_PLUGIN_H

// Qt includes
#include <QToolButton>
#include <QString>
#include <QWidget>


// Lumina-desktop includes
#include "../LPPlugin.h" //main plugin widget

// libLumina includes
#include "LuminaXDG.h"

// PANEL PLUGIN BUTTON
class AppLaunchButtonPlugin : public LPPlugin{
	Q_OBJECT

public:
	AppLaunchButtonPlugin(QWidget *parent = 0, QString id = "applauncher", bool horizontal=true);
	~AppLaunchButtonPlugin();

private:
	QToolButton *button;
	QString appfile;

	void updateButtonVisuals();

private slots:
	void AppClicked();

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
protected:
	void changeEvent(QEvent *ev){
	  LPPlugin::changeEvent(ev);
	  QEvent::Type tmp = ev->type();
	  if(tmp == QEvent::StyleChange || tmp==QEvent::ThemeChange || tmp==QEvent::LanguageChange || tmp==QEvent::LocaleChange){ updateButtonVisuals(); }
	}
};

#endif
