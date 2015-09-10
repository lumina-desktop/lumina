//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LStartButton.h"
#include "../../LSession.h"

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
  startmenu = new StartMenu(this);
    connect(startmenu, SIGNAL(CloseMenu()), this, SLOT(closeMenu()) );
  mact = new QWidgetAction(this);
    mact->setDefaultWidget(startmenu);
    menu->addAction(mact);
	
  button->setMenu(menu);
  connect(menu, SIGNAL(aboutToHide()), this, SLOT(updateButtonVisuals()) );
  QTimer::singleShot(0,this, SLOT(OrientationChange())); //Update icons/sizes
}

LStartButtonPlugin::~LStartButtonPlugin(){

}

void LStartButtonPlugin::updateButtonVisuals(){
    button->setToolTip(tr(""));
    button->setText( SYSTEM::user() );
    button->setIcon( LXDG::findIcon("pcbsd","Lumina-DE") ); //force icon refresh
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

