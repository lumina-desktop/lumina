//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LUserButton.h"
#include "../../LSession.h"

LUserButtonPlugin::LUserButtonPlugin(QWidget *parent, QString id, bool horizontal) : LPPlugin(parent, id, horizontal){
  button = new QToolButton(this);
    button->setAutoRaise(true);
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setPopupMode(QToolButton::DelayedPopup); //make sure it runs the update routine first
    connect(button, SIGNAL(clicked()), this, SLOT(openMenu()));
    this->layout()->setContentsMargins(0,0,0,0);
    this->layout()->addWidget(button);
  menu = new QMenu(this);
    menu->setContentsMargins(1,1,1,1);
    connect(menu, SIGNAL(aboutToHide()), this, SIGNAL(MenuClosed()));
  usermenu = new UserWidget(this);
    connect(usermenu, SIGNAL(CloseMenu()), this, SLOT(closeMenu()) );
  mact = new QWidgetAction(this);
    mact->setDefaultWidget(usermenu);
    menu->addAction(mact);
	
  button->setMenu(menu);
  connect(menu, SIGNAL(aboutToHide()), this, SLOT(updateButtonVisuals()) );
  //Setup the global shortcut handling for opening the start menu
  connect(QApplication::instance(), SIGNAL(StartButtonActivated()), this, SLOT(shortcutActivated()) );
  LSession::handle()->registerStartButton(this->type());

  QTimer::singleShot(0,this, SLOT(OrientationChange())); //Update icons/sizes
}

LUserButtonPlugin::~LUserButtonPlugin(){

}

void LUserButtonPlugin::updateButtonVisuals(){
    button->setToolTip(tr("Quickly launch applications or open files"));
    button->setText( SYSTEM::user() );
    if( QFile::exists(QDir::homePath()+"/.loginIcon.png") ){
      button->setIcon( QIcon(QDir::homePath()+"/.loginIcon.png") );
    }else{
      button->setIcon( LXDG::findIcon("user-identity", ":/images/default-user.png") ); //force icon refresh
    }
}

// ========================
//    PRIVATE FUNCTIONS
// ========================
void LUserButtonPlugin::openMenu(){
  usermenu->UpdateMenu();
  button->showMenu();
}

void LUserButtonPlugin::closeMenu(){
  menu->hide();
}

void LUserButtonPlugin::shortcutActivated(){
  if(LSession::handle()->registerStartButton(this->type())){
    if(menu->isVisible()){ closeMenu(); }
    else{ openMenu(); }
  }
}
