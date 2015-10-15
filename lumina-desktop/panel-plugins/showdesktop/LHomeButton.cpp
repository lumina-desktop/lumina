//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LHomeButton.h"
#include "../../LSession.h"

#include <LuminaX11.h>

LHomeButtonPlugin::LHomeButtonPlugin(QWidget *parent, QString id, bool horizontal) : LPPlugin(parent, id, horizontal){
  button = new QToolButton(this);
    button->setAutoRaise(true);
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    connect(button, SIGNAL(clicked()), this, SLOT(showDesktop()));
    this->layout()->setContentsMargins(0,0,0,0);
    this->layout()->addWidget(button);

  QTimer::singleShot(0,this, SLOT(OrientationChange())); //Update icons/sizes
}

LHomeButtonPlugin::~LHomeButtonPlugin(){

}

void LHomeButtonPlugin::updateButtonVisuals(){
  button->setIcon( LXDG::findIcon("user-desktop", "") );
}

// ========================
//    PRIVATE FUNCTIONS
// ========================
void LHomeButtonPlugin::showDesktop(){
  QList<WId> wins = LSession::handle()->XCB->WindowList();
  for(int i=0; i<wins.length(); i++){
    if( LXCB::INVISIBLE != LSession::handle()->XCB->WindowState(wins[i]) ){
      LSession::handle()->XCB->MinimizeWindow(wins[i]);
    }
  }
}

