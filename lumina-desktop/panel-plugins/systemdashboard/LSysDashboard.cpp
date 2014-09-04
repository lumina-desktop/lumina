//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LSysDashboard.h"

LSysDashboard::LSysDashboard(QWidget *parent, QString id, bool horizontal) : LPPlugin(parent, id, horizontal){
  upTimer = new QTimer(this);
    upTimer->setInterval(10000); //10 second update ping
    connect(upTimer, SIGNAL(timeout()), this, SLOT(updateIcon()));
  button = new QToolButton(this);
    button->setAutoRaise(true);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setToolTip(QString("System Dashboard"));
    connect(button, SIGNAL(clicked()), this, SLOT(openMenu()));
    this->layout()->setContentsMargins(0,0,0,0);
    this->layout()->addWidget(button);
  menu = new QMenu(this);
  sysmenu = new LSysMenuQuick(this);
    connect(sysmenu, SIGNAL(CloseMenu()), this, SLOT(closeMenu()) );
  mact = new QWidgetAction(this);
    mact->setDefaultWidget(sysmenu);
    menu->addAction(mact);
	
  QTimer::singleShot(0,this, SLOT(OrientationChange())); //Update icons/sizes
}

LSysDashboard::~LSysDashboard(){

}

// ========================
//    PRIVATE FUNCTIONS
// ========================
void LSysDashboard::updateIcon(bool force){
  //For the visual, show battery state only if important
  static bool batcharging = false;
  QPixmap pix;
  if(LOS::hasBattery()){
    int bat = LOS::batteryCharge();
    bool charging = LOS::batteryIsCharging();
    //Set the icon as necessary
      if(charging && !batcharging){
	//Charging and just plugged in
	if(bat < 15){ button->setIcon( LXDG::findIcon("battery-charging-low","") ); QTimer::singleShot(5000, this, SLOT(resetIcon()));}
	else if(bat < 30){ button->setIcon( LXDG::findIcon("battery-charging-caution","") ); QTimer::singleShot(5000, this, SLOT(resetIcon()));}
	else if(force || button->icon().isNull()){ resetIcon(); }
      }else if(!charging){
        //Not charging (critical level or just unplugged)
	if(bat<1){ button->setIcon( LXDG::findIcon("battery-missing","") ); QTimer::singleShot(5000, this, SLOT(resetIcon()));}
	else if(bat < 15){ button->setIcon( LXDG::findIcon("battery-low","") ); QTimer::singleShot(5000, this, SLOT(resetIcon())); }
	else if(bat < 30){ button->setIcon( LXDG::findIcon("battery-caution","") ); QTimer::singleShot(5000, this, SLOT(resetIcon()));}
	else if(bat < 50 && batcharging){ button->setIcon( LXDG::findIcon("battery-040","")); QTimer::singleShot(5000, this, SLOT(resetIcon()));}
	else if(bat < 70 && batcharging){ button->setIcon( LXDG::findIcon("battery-060","")); QTimer::singleShot(5000, this, SLOT(resetIcon()));}
	else if(bat < 90 && batcharging){ button->setIcon( LXDG::findIcon("battery-080","")); QTimer::singleShot(5000, this, SLOT(resetIcon()));}
	else if(batcharging){ button->setIcon( LXDG::findIcon("battery-100","")); QTimer::singleShot(5000, this, SLOT(resetIcon()));}
	else if(force || button->icon().isNull()){ resetIcon(); }
      }else if(force || button->icon().isNull()){
	//Otherwise just use the default icon
	resetIcon();
      }
    //Save the values for comparison later
    batcharging = charging;
    if( !upTimer->isActive() ){ upTimer->start(); } //only use the timer if a battery is present

  // No battery - just use/set the normal icon
  }else if(force || button->icon().isNull()){
    resetIcon();
    if(upTimer->isActive() ){ upTimer->stop(); } //no battery available - no refresh timer needed
  }
  
}

void LSysDashboard::resetIcon(){
  button->setIcon( LXDG::findIcon("dashboard-show",""));
}

void LSysDashboard::openMenu(){
  sysmenu->UpdateMenu();
  menu->popup(this->mapToGlobal(QPoint(0,0)));
}

void LSysDashboard::closeMenu(){
  menu->hide();
}

