//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This panel plugin allows the user to control different system settings
//    For example: screen brightness, audio volume, workspace, and battery
//===========================================
#ifndef _LUMINA_DESKTOP_SYSTEM_DASHBOARD_H
#define _LUMINA_DESKTOP_SYSTEM_DASHBOARD_H

//Qt includes

#include <QHBoxLayout>
#include <QDebug>
#include <QCoreApplication>
#include <QPainter>
#include <QPixmap>
#include <QWidgetAction>
#include <QMenu>
#include <QTimer>
#include <QToolButton>

//libLumina includes
#include <LuminaOS.h>
#include <LuminaXDG.h>

//Local includes
#include "../LPPlugin.h"
#include "SysMenuQuick.h"

class LSysDashboard : public LPPlugin{
	Q_OBJECT
public:
	LSysDashboard(QWidget *parent = 0, QString id="systemdashboard", bool horizontal=true);
	~LSysDashboard();

private:
	QMenu *menu;
	QWidgetAction *mact;
	LSysMenuQuick *sysmenu;
	QToolButton *button;
	QTimer *upTimer;
	
private slots:
	void updateIcon(bool force = false);
	void resetIcon();
	void openMenu();
	void closeMenu();
	
public slots:
	void OrientationChange(){
	  if(this->layout()->direction()==QBoxLayout::LeftToRight){
	    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
	    button->setIconSize( QSize(this->height(), this->height()) );
	  }else{
	    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
	    button->setIconSize( QSize(this->width(), this->width()) );
	  }
	  updateIcon(true); //force icon refresh
	  this->layout()->update();
	}
};

#endif
