//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "NetworkButton.h"
#include <global-objects.h>

NetworkButton::NetworkButton(QWidget *parent) : QToolButton(parent){
  this->setAutoRaise(true);
  this->setPopupMode(QToolButton::InstantPopup);
  this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  //Setup the menu

  //Now start the initial update routine in a moment
  connect(OSInterface::instance(), SIGNAL(networkStatusChanged()), this, SLOT(updateButton()) );
  QTimer::singleShot(10, this, SLOT(updateButton()) );
}

NetworkButton::~NetworkButton(){

}

void NetworkButton::updateButton(){
  this->setIcon( QIcon::fromTheme( OSInterface::instance()->networkIcon() ) );
  //Now get all the info about the battery for the tooltip
  this->setToolTip( OSInterface::instance()->networkStatus() );
  //qDebug() << "Network Button Sync:" << OSInterface::instance()->networkIcon();
}

void NetworkButton::buttonClicked(){
  if(OSInterface::instance()->hasNetworkManager()){
    LSession::instance()->LaunchStandardApplication( OSInterface::instance()->networkManagerUtility() );
  }
}
