//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "BatteryButton.h"

BatteryButton::BatteryButton(QWidget *parent) : QToolButton(parent){
  this->setAutoRaise(true);
  this->setPopupMode(QToolButton::InstantPopup);
  this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  //Setup the menu

  //Now start the initial update routine in a moment
  connect(OSInterface::instance(), SIGNAL(batteryChanged()), this, SLOT(updateButton()) );
  QTimer::singleShot(10, this, SLOT(updateButton()) );
}

BatteryButton::~BatteryButton(){

}

void BatteryButton::updateButton(){
  this->setIcon( QIcon::fromTheme( OSInterface::instance()->batteryIcon() ) );
  //Now get all the info about the battery for the tooltip
  this->setToolTip( OSInterface::instance()->batteryStatus() );
}
