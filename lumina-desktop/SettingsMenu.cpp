//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "SettingsMenu.h"
#include "LSession.h"

SettingsMenu::SettingsMenu() : QMenu(){
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
  #ifdef __FreeBSD__
  if(QFile::exists("/usr/local/bin/qtconfig-qt4")){
  #endif
  #ifdef __linux__
  if(QFile::exists("/usr/bin/qtconfig-qt4")){
  #endif
    act = new QAction( LXDG::findIcon("preferences-desktop-theme",""), tr("Window Theme"), this);
	act->setWhatsThis("qtconfig-qt4");
	this->addAction(act);
  }
}

SettingsMenu::~SettingsMenu(){
	
}

void SettingsMenu::runApp(QAction* act){
  LSession::LaunchApplication(act->whatsThis());
}
