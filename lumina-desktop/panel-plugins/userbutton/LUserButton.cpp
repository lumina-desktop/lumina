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
    button->setToolTip(QString("Quickly launch applications or open files"));
    button->setText( SYSTEM::user() );
    connect(button, SIGNAL(clicked()), this, SLOT(openMenu()));
    this->layout()->setContentsMargins(0,0,0,0);
    this->layout()->addWidget(button);
  menu = new QMenu(this);
    menu->setContentsMargins(1,1,1,1);
  usermenu = new UserWidget(this);
    connect(usermenu, SIGNAL(CloseMenu()), this, SLOT(closeMenu()) );
  mact = new QWidgetAction(this);
    mact->setDefaultWidget(usermenu);
    menu->addAction(mact);
	
  QTimer::singleShot(0,this, SLOT(OrientationChange())); //Update icons/sizes
}

LUserButtonPlugin::~LUserButtonPlugin(){

}

// ========================
//    PRIVATE FUNCTIONS
// ========================
void LUserButtonPlugin::openMenu(){
  usermenu->UpdateMenu();
  menu->popup(this->mapToGlobal(QPoint(0,0)));
}

void LUserButtonPlugin::closeMenu(){
  menu->hide();
}

