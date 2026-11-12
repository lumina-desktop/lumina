//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "VolumeButton.h"

VolumeButton::VolumeButton(QWidget *parent) : QToolButton(parent){
  this->setAutoRaise(true);
  this->setPopupMode(QToolButton::InstantPopup);
  this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  //Setup the menu

  //Now start the initial update routine in a moment
  connect(OSInterface::instance(), SIGNAL(volumeChanged()), this, SLOT(updateButton()) );
  QTimer::singleShot(10, this, SLOT(updateButton()) );
}

VolumeButton::~VolumeButton(){

}

void VolumeButton::updateButton(){
  this->setIcon( QIcon::fromTheme( OSInterface::instance()->volumeIcon() ) );
  int vol = OSInterface::instance()->volume();
  this->setToolTip( QString("%1%").arg(QString::number(vol)) );
}
