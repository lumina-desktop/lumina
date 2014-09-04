//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "SysMenuQuick.h"
#include "ui_SysMenuQuick.h"

#include "../../LSession.h"
#include <LuminaX11.h>

LSysMenuQuick::LSysMenuQuick(QWidget *parent) : QWidget(parent), ui(new Ui::LSysMenuQuick){
  ui->setupUi(this);
  settings = new QSettings("panel-plugins","systemdashboard");
  //Now reset the initial saved settings (if any)
  LOS::setScreenBrightness( settings->value("screenbrightness",100).toInt() ); //default to 100%
  LOS::setAudioVolume( settings->value("audiovolume", 100).toInt() ); //default to 100%
  //Now setup the connections
  connect(ui->slider_volume, SIGNAL(valueChanged(int)), this, SLOT(volSliderChanged()) );
  connect(ui->slider_brightness, SIGNAL(valueChanged(int)), this, SLOT(brightSliderChanged()) );
  connect(ui->tool_wk_prev, SIGNAL(clicked()), this, SLOT(prevWorkspace()) );
  connect(ui->tool_wk_next, SIGNAL(clicked()), this, SLOT(nextWorkspace()) );
  connect(ui->tool_logout, SIGNAL(clicked()), this, SLOT(startLogout()) );
  connect(ui->tool_vol_mixer, SIGNAL(clicked()), this, SLOT(startMixer()) );
  //And setup the default icons
  ui->label_bright_icon->setPixmap( LXDG::findIcon("preferences-system-power-management","").pixmap(ui->label_bright_icon->maximumSize()) );
  ui->tool_wk_prev->setIcon( LXDG::findIcon("go-previous-view",""));
  ui->tool_wk_next->setIcon( LXDG::findIcon("go-next-view","") );
  ui->tool_logout->setIcon( LXDG::findIcon("system-log-out","") );
}

LSysMenuQuick::~LSysMenuQuick(){
	
}

void LSysMenuQuick::UpdateMenu(){
  //Audio Volume
  int val = LOS::audioVolume();	
  QIcon ico;
  if(val > 66){ ico= LXDG::findIcon("audio-volume-high",""); }
  else if(val > 33){ ico= LXDG::findIcon("audio-volume-medium",""); }
  else if(val > 0){ ico= LXDG::findIcon("audio-volume-low",""); }
  else{ ico= LXDG::findIcon("audio-volume-muted",""); }
  bool hasMixer = LOS::hasMixerUtility();
  ui->label_vol_icon->setVisible(!hasMixer);
  ui->tool_vol_mixer->setVisible(hasMixer);
  if(!hasMixer){ ui->label_vol_icon->setPixmap( ico.pixmap(ui->label_vol_icon->maximumSize()) ); }
  else{ ui->tool_vol_mixer->setIcon(ico); }
  QString txt = QString::number(val)+"%";
  if(val<100){ txt.prepend(" "); } //make sure no widget resizing
  ui->label_vol_text->setText(txt);
  if(ui->slider_volume->value()!= val){ ui->slider_volume->setValue(val); }
  //Screen Brightness
  val = LOS::ScreenBrightness();
  txt = QString::number(val)+"%";
  if(val<100){ txt.prepend(" "); } //make sure no widget resizing
  ui->label_bright_text->setText(txt);
  if(ui->slider_brightness->value()!=val){ ui->slider_brightness->setValue(val); }
  //Battery Status
  if(LOS::hasBattery()){
    ui->group_battery->setVisible(true);
    val = LOS::batteryCharge();
    if(LOS::batteryIsCharging()){
      if(val < 15){ ui->label_bat_icon->setPixmap( LXDG::findIcon("battery-charging-low","").pixmap(ui->label_bat_icon->maximumSize()) ); }
      else if(val < 30){ ui->label_bat_icon->setPixmap( LXDG::findIcon("battery-charging-caution","").pixmap(ui->label_bat_icon->maximumSize()) ); }
      else if(val < 50){ ui->label_bat_icon->setPixmap( LXDG::findIcon("battery-charging-040","").pixmap(ui->label_bat_icon->maximumSize()) ); }
      else if(val < 70){ ui->label_bat_icon->setPixmap( LXDG::findIcon("battery-charging-060","").pixmap(ui->label_bat_icon->maximumSize()) ); }
      else if(val < 90){ ui->label_bat_icon->setPixmap( LXDG::findIcon("battery-charging-080","").pixmap(ui->label_bat_icon->maximumSize()) ); }
      else{ ui->label_bat_icon->setPixmap( LXDG::findIcon("battery-charging","").pixmap(ui->label_bat_icon->maximumSize()) ); }
      ui->label_bat_text->setText( QString("%1%\n(%2)").arg(QString::number(val), tr("charging")) );
    }else{
      if(val < 1){ ui->label_bat_icon->setPixmap( LXDG::findIcon("battery-missing","").pixmap(ui->label_bat_icon->maximumSize()) ); }
      else if(val < 15){ ui->label_bat_icon->setPixmap( LXDG::findIcon("battery-low","").pixmap(ui->label_bat_icon->maximumSize()) ); }
      else if(val < 30){ ui->label_bat_icon->setPixmap( LXDG::findIcon("battery-caution","").pixmap(ui->label_bat_icon->maximumSize()) ); }
      else if(val < 50){ ui->label_bat_icon->setPixmap( LXDG::findIcon("battery-040","").pixmap(ui->label_bat_icon->maximumSize()) ); }
      else if(val < 70){ ui->label_bat_icon->setPixmap( LXDG::findIcon("battery-060","").pixmap(ui->label_bat_icon->maximumSize()) ); }
      else if(val < 90){ ui->label_bat_icon->setPixmap( LXDG::findIcon("battery-080","").pixmap(ui->label_bat_icon->maximumSize()) ); }
      else{ ui->label_bat_icon->setPixmap( LXDG::findIcon("battery-100","").pixmap(ui->label_bat_icon->maximumSize()) ); }
      ui->label_bat_text->setText( QString("%1%\n(%2)").arg(QString::number(val), getRemainingTime()) );
    }
  }else{
    ui->group_battery->setVisible(false);
  }
  //Workspace
  val = LX11::GetCurrentDesktop();
  int tot = LX11::GetNumberOfDesktops();
  ui->group_workspace->setVisible(val>=0 && tot>1);
  ui->label_wk_text->setText( QString(tr("%1 of %2")).arg(QString::number(val+1), QString::number(tot)) );
}

void LSysMenuQuick::volSliderChanged(){
  int val = ui->slider_volume->value();
  LOS::setAudioVolume(val);
  settings->setValue("audiovolume",val);
  QString txt = QString::number(val)+"%";
  if(val<100){ txt.prepend(" "); } //make sure no widget resizing
  ui->label_vol_text->setText( txt );
  if(val > 66){ ui->label_vol_icon->setPixmap( LXDG::findIcon("audio-volume-high","").pixmap(ui->label_vol_icon->maximumSize()) ); }
  else if(val > 33){ ui->label_vol_icon->setPixmap( LXDG::findIcon("audio-volume-medium","").pixmap(ui->label_vol_icon->maximumSize()) ); }
  else if(val > 0){ ui->label_vol_icon->setPixmap( LXDG::findIcon("audio-volume-low","").pixmap(ui->label_vol_icon->maximumSize()) ); }
  else{ ui->label_vol_icon->setPixmap( LXDG::findIcon("audio-volume-muted","").pixmap(ui->label_vol_icon->maximumSize()) ); }
}

void LSysMenuQuick::startMixer(){
  emit CloseMenu();
  LOS::startMixerUtility();
}

void LSysMenuQuick::brightSliderChanged(){
  int val = ui->slider_brightness->value();
  LOS::setScreenBrightness(val);
  settings->setValue("screenbrightness",val);
  QString txt = QString::number(val)+"%";
  if(val<100){ txt.prepend(" "); } //make sure no widget resizing
  ui->label_bright_text->setText( txt );
}

void LSysMenuQuick::nextWorkspace(){
  int cur = LX11::GetCurrentDesktop();
  int tot = LX11::GetNumberOfDesktops();
  cur++;
  if(cur>=tot){ cur = 0; } //back to beginning
  LX11::SetCurrentDesktop(cur);
ui->label_wk_text->setText( QString(tr("%1 of %2")).arg(QString::number(cur+1), QString::number(tot)) );
}

void LSysMenuQuick::prevWorkspace(){
  int cur = LX11::GetCurrentDesktop();
  int tot = LX11::GetNumberOfDesktops();
  cur--;
  if(cur<0){ cur = tot-1; } //back to last
  LX11::SetCurrentDesktop(cur);
  ui->label_wk_text->setText( QString(tr("%1 of %2")).arg(QString::number(cur+1), QString::number(tot)) );	
}

QString LSysMenuQuick::getRemainingTime(){
  int secs = LOS::batterySecondsLeft();
  if(secs < 0){ return "??"; }
  QString rem; //remaining
  if(secs > 3600){
    int hours = secs/3600;
    rem.append( QString::number(hours)+"h ");
    secs = secs - (hours*3600);
  }
  if(secs > 60){
    int min = secs/60;
    rem.append( QString::number(min)+"m ");
    secs = secs - (min*60);
  }
  if(secs > 0){
    rem.append( QString::number(secs)+"s");
  }else{
    rem.append( "0s" );
  }
  return rem;
}

void LSysMenuQuick::startLogout(){
  emit CloseMenu();
  LSession::systemWindow();
}