//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "PPJsonMenu.h"
#include "../../JsonMenu.h"

LPJsonMenu::LPJsonMenu(QWidget *parent, QString id, bool horizontal) : LPPlugin(parent, id, horizontal){
  //Setup the button
    button = new QToolButton(this);
    button->setAutoRaise(true);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setPopupMode(QToolButton::InstantPopup); //make sure it runs the update routine first
    //connect(button, SIGNAL(clicked()), this, SLOT(openMenu()));
    this->layout()->setContentsMargins(0,0,0,0);
    this->layout()->addWidget(button);
  //Parse the id and get the extra information needed for the plugin
  QStringList info = id.section("---",0,0).split("::::"); //FORMAT:[ "jsonmenu---<number>",exec,name, icon(optional)]
  if(info.length()>=3){
        qDebug() << "Custom JSON Menu Loaded:" << info;
        JsonMenu *menu = new JsonMenu(info[1], button);
        button->setText(info[2]);
        //connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(SystemApplication(QAction*)) );
        if(info.length()>=4){ button->setIcon( LXDG::findIcon(info[3],"run-build") ); }
        else{ button->setIcon( LXDG::findIcon("run-build","") ); }
        button->setMenu(menu);
      }
  //Now start up the widgets
  QTimer::singleShot(0,this,SLOT(OrientationChange()) ); //update the sizing/icon
}

LPJsonMenu::~LPJsonMenu(){
}
