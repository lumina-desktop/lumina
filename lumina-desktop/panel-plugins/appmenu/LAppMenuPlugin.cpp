//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LAppMenuPlugin.h"
#include "../../LSession.h"

#include <LuminaXDG.h>

LAppMenuPlugin::LAppMenuPlugin(QWidget *parent, QString id, bool horizontal) : LPPlugin(parent, id, horizontal){
  button = new QToolButton(this);
    button->setAutoRaise(true);
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setMenu( LSession::handle()->applicationMenu() );
    connect(button->menu(), SIGNAL(aboutToHide()), this, SIGNAL(MenuClosed()));
    button->setPopupMode(QToolButton::InstantPopup);
    this->layout()->setContentsMargins(0,0,0,0);
    this->layout()->addWidget(button);

	
  QTimer::singleShot(0,this, SLOT(OrientationChange())); //Update icons/sizes
}

LAppMenuPlugin::~LAppMenuPlugin(){

}

void LAppMenuPlugin::updateButtonVisuals(){
    button->setToolTip( tr("Quickly launch applications or open files"));
    button->setText( tr("Start Here") );
    //Use the PC-BSD icon by default (or the Lumina icon for non-PC-BSD systems)
    button->setIcon( LXDG::findIcon("pcbsd","Lumina-DE") );
}

// ========================
//    PRIVATE FUNCTIONS
// ========================
/*void LAppMenuPlugin::openMenu(){
  usermenu->UpdateMenu();
  menu->popup(this->mapToGlobal(QPoint(0,0)));
}

void LAppMenuPlugin::closeMenu(){
  menu->hide();
}*/

