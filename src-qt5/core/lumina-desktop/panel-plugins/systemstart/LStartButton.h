//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This panel plugin is the main button that allow the user to run 
//    applications or logout of the desktop
//===========================================
#ifndef _LUMINA_DESKTOP_START_MENU_PLUGIN_H
#define _LUMINA_DESKTOP_START_MENU_PLUGIN_H

// Qt includes
#include <QMenu>
#include <QWidgetAction>
#include <QToolButton>
#include <QString>
#include <QWidget>
#include <QMenu>

// Lumina-desktop includes
//#include "../../Globals.h"
#include "../LPPlugin.h" //main plugin widget

// libLumina includes
#include <LuminaXDG.h>
#include <LUtils.h>
#include <ResizeMenu.h>

#include "StartMenu.h"

//Simple Tool Button For QuickLaunch Items
class LQuickLaunchButton : public QToolButton{
	Q_OBJECT
signals:
	void Launch(QString);
	void Remove(QString);

private slots:
	void rmentry(){
	  emit Remove(this->whatsThis());
	}
	void launchentry(){
	  emit Launch(this->whatsThis());
	}
	
public:
	LQuickLaunchButton(QString path, QWidget* parent = 0) : QToolButton(parent){
	  this->setWhatsThis(path);
	  this->setMenu(new QMenu(this));
	  this->setContextMenuPolicy(Qt::CustomContextMenu);
	  this->menu()->addAction( LXDG::findIcon("edit-delete",""), tr("Remove from Quicklaunch"), this, SLOT(rmentry()) );
	  connect(this, SIGNAL(clicked()), this, SLOT(launchentry()) );
	  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showMenu()) );
	}
	~LQuickLaunchButton(){}

};

// PANEL PLUGIN BUTTON
class LStartButtonPlugin : public LPPlugin{
	Q_OBJECT
	
public:
	LStartButtonPlugin(QWidget *parent = 0, QString id = "systemstart", bool horizontal=true);
	~LStartButtonPlugin();
	
private:
	ResizeMenu *menu;
	//QWidgetAction *mact;
	StartMenu *startmenu;
	QToolButton *button;
	QList<LQuickLaunchButton*> QUICKL;

private slots:
	void openMenu();
	void closeMenu();
	void shortcutActivated();

	void updateButtonVisuals();

	void updateQuickLaunch(QStringList);
	void LaunchQuick(QString);
	void RemoveQuick(QString);
	void SaveMenuSize(QSize);

public slots:
	void OrientationChange(){
	  if(this->layout()->direction()==QBoxLayout::LeftToRight){
	    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
	    button->setIconSize( QSize(this->height(), this->height()) );
	    for(int i=0; i<QUICKL.length(); i++){ QUICKL[i]->setIconSize(QSize(this->height(), this->height())); }
	  }else{
	    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
	    button->setIconSize( QSize(this->width(), this->width()) );
	    for(int i=0; i<QUICKL.length(); i++){ QUICKL[i]->setIconSize(QSize(this->width(), this->width())); }
	  }
	  this->layout()->update();
	  updateButtonVisuals();
	}
	
	void LocaleChange(){ 
	  updateButtonVisuals();
	  if(startmenu!=0){ startmenu->UpdateAll(); }
	}
	
	void ThemeChange(){
	  updateButtonVisuals();
	  if(startmenu!=0){ startmenu->UpdateAll(); }
	}
};

#endif
