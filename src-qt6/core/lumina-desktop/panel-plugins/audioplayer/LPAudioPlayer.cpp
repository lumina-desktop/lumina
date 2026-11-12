//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Susanne Jaeckel
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LPAudioPlayer.h"
#include "LSession.h"

LPAudioPlayer::LPAudioPlayer(QWidget *parent, QString id, bool horizontal) : LPPlugin(parent, id, horizontal){
  //Setup the button
    button = new QToolButton(this);
    button->setAutoRaise(true);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setPopupMode(QToolButton::InstantPopup); //make sure it runs the update routine first
    //connect(button, SIGNAL(clicked()), this, SLOT(openMenu()));
    this->layout()->setContentsMargins(0,0,0,0);
    this->layout()->addWidget(button);
  wact = new QWidgetAction(this);
  aplayer = new PPlayerWidget(this);
  button ->setMenu(new QMenu(this) );
  wact->setDefaultWidget(aplayer);
  button->menu()->addAction(wact);
  //Now start up the widgets
  button->setIcon( LXDG::findIcon("audio-volume-high","") );
  QTimer::singleShot(0,this,SLOT(OrientationChange()) ); //update the sizing/icon
}

LPAudioPlayer::~LPAudioPlayer(){
}
