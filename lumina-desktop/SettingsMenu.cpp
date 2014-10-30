//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "SettingsMenu.h"
#include "LSession.h"

#include <LuminaOS.h>

SettingsMenu::SettingsMenu() : QMenu(){
  QTimer::singleShot(10, this, SLOT(InitMenu()) );
}

SettingsMenu::~SettingsMenu(){
	
}

void SettingsMenu::InitMenu(){
  this->setTitle( tr("Desktop Settings") );
  this->setIcon( LXDG::findIcon("configure","") );
  connect(this, SIGNAL(triggered(QAction*)), this, SLOT(runApp(QAction*)) );
  //Now setup the possible configuration options
  QAction *act = new QAction(LXDG::findIcon("preferences-desktop-screensaver",""), tr("Screensaver"), this);
	act->setWhatsThis("xscreensaver-demo");
	this->addAction(act);
  act = new QAction( LXDG::findIcon("preferences-desktop",""), tr("Desktop"), this);
	act->setWhatsThis("lumina-config");
	this->addAction(act);
  QString qtconfig = LOS::QtConfigShortcut();
  if(QFile::exists(qtconfig) && !qtconfig.isEmpty()){
    act = new QAction( LXDG::findIcon("preferences-desktop-theme",""), tr("Window Theme"), this);
	act->setWhatsThis(qtconfig);
	this->addAction(act);
  }
  QString CONTROLPANEL = LOS::ControlPanelShortcut();
  if(QFile::exists(CONTROLPANEL) && !CONTROLPANEL.isEmpty()){
    //Now load the info
    bool ok = false;
    XDGDesktop cpan = LXDG::loadDesktopFile(CONTROLPANEL, ok);
    if(ok){
      act = new QAction( LXDG::findIcon(cpan.icon,""), tr("Control Panel"), this);
	act->setWhatsThis("lumina-open \""+CONTROLPANEL+"\"");
	this->addAction(act);
    }
  }	
}

void SettingsMenu::runApp(QAction* act){
  LSession::LaunchApplication(act->whatsThis());
}
