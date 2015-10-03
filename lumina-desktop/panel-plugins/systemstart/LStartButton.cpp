//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LStartButton.h"
#include "../../LSession.h"

#include <LuminaXDG.h>

LStartButtonPlugin::LStartButtonPlugin(QWidget *parent, QString id, bool horizontal) : LPPlugin(parent, id, horizontal){
  button = new QToolButton(this);
    button->setAutoRaise(true);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setPopupMode(QToolButton::DelayedPopup); //make sure it runs the update routine first
    connect(button, SIGNAL(clicked()), this, SLOT(openMenu()));
    this->layout()->setContentsMargins(0,0,0,0);
    this->layout()->addWidget(button);
  menu = new QMenu(this);
    menu->setContentsMargins(1,1,1,1);
    connect(menu, SIGNAL(aboutToHide()), this, SIGNAL(MenuClosed()));
  startmenu = new StartMenu(this);
    connect(startmenu, SIGNAL(CloseMenu()), this, SLOT(closeMenu()) );
    connect(startmenu, SIGNAL(UpdateQuickLaunch(QStringList)), this, SLOT(updateQuickLaunch(QStringList)));
  mact = new QWidgetAction(this);
    mact->setDefaultWidget(startmenu);
    menu->addAction(mact);
	
  button->setMenu(menu);
  connect(menu, SIGNAL(aboutToHide()), this, SLOT(updateButtonVisuals()) );
  QTimer::singleShot(0,this, SLOT(OrientationChange())); //Update icons/sizes
  QTimer::singleShot(0, startmenu, SLOT(ReLoadQuickLaunch()) );
}

LStartButtonPlugin::~LStartButtonPlugin(){

}

void LStartButtonPlugin::updateButtonVisuals(){
    button->setToolTip(tr(""));
    button->setText( SYSTEM::user() );
    button->setIcon( LXDG::findIcon("pcbsd","Lumina-DE") ); //force icon refresh
}

void LStartButtonPlugin::updateQuickLaunch(QStringList apps){
  //First clear any obsolete apps
  QStringList old;
  for(int i=0; i<QUICKL.length(); i++){
    if( !apps.contains(QUICKL[i]->whatsThis()) ){
      //App was removed
      delete QUICKL.takeAt(i);
      i--;
    }else{
      //App still listed - update the button
      old << QUICKL[i]->defaultAction()->whatsThis(); //add the list of current buttons
      LFileInfo info(QUICKL[i]->whatsThis());
      QUICKL[i]->defaultAction()->setIcon( LXDG::findIcon(info.iconfile(),"unknown") );
      if(info.isDesktopFile()){ QUICKL[i]->defaultAction()->setToolTip( info.XDG()->name ); }
      else{ QUICKL[i]->defaultAction()->setToolTip( info.fileName() ); }
    }
  }
  //Now go through and create any new buttons
  for(int i=0; i<apps.length(); i++){
    if( !old.contains(apps[i]) ){
      //New App
      QToolButton *tmp = new QToolButton(this);
      QAction *act = new QAction(tmp);
	tmp->setDefaultAction(act);
        act->setWhatsThis(apps[i]);
      QUICKL << tmp;
      LFileInfo info(apps[i]);
      act->setIcon( LXDG::findIcon( info.iconfile() ) );
      if(info.isDesktopFile()){ act->setToolTip( info.XDG()->name ); }
      else{ act->setToolTip( info.fileName() ); }
      //Now add the button to the layout and connect the signal/slots
      this->layout()->insertWidget(i+1,tmp); //"button" is always in slot 0
      connect(tmp, SIGNAL(triggered(QAction*)), this, SLOT(LaunchQuick(QAction*)) );
    }
  }
  QTimer::singleShot(0,this, SLOT(OrientationChange())); //Update icons/sizes
}

void LStartButtonPlugin::LaunchQuick(QAction* act){
  //Need to get which button was clicked
  qDebug() << "Quick Launch triggered:" << act->whatsThis();
  if(!act->whatsThis().isEmpty()){
    LSession::LaunchApplication("lumina-open \""+act->whatsThis()+"\"");
    emit MenuClosed();
  }
}

// ========================
//    PRIVATE FUNCTIONS
// ========================
void LStartButtonPlugin::openMenu(){
  startmenu->UpdateMenu();
  button->showMenu();
}

void LStartButtonPlugin::closeMenu(){
  menu->hide();
}

