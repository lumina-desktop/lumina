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
  brighttimer = new QTimer(this);
    brighttimer->setSingleShot(true);
    brighttimer->setInterval(50); //50ms delay in setting the new value
  //Now reset the initial saved settings (this is handles by the LOS/session now - 4/22/15)
  firstrun = true;
  UpdateMenu(); //do this once before all the signals/slots are connected below
  firstrun = false;
  //Now setup the connections
  connect(ui->slider_volume, SIGNAL(valueChanged(int)), this, SLOT(volSliderChanged()) );
  connect(ui->slider_brightness, SIGNAL(valueChanged(int)), this, SLOT(brightSliderChanged()) );
  connect(ui->tool_wk_prev, SIGNAL(clicked()), this, SLOT(prevWorkspace()) );
  connect(ui->tool_wk_next, SIGNAL(clicked()), this, SLOT(nextWorkspace()) );
  connect(ui->tool_logout, SIGNAL(clicked()), this, SLOT(startLogout()) );
  connect(ui->tool_vol_mixer, SIGNAL(clicked()), this, SLOT(startMixer()) );
  connect(brighttimer, SIGNAL(timeout()), this, SLOT(setCurrentBrightness()) );
  connect(ui->combo_locale, SIGNAL(currentIndexChanged(int)), this, SLOT(changeLocale()) );
  //And setup the default icons
  ui->label_bright_icon->setPixmap( LXDG::findIcon("preferences-desktop-brightness","").pixmap(ui->label_bright_icon->maximumSize()) );
  ui->tool_wk_prev->setIcon( LXDG::findIcon("go-previous-view",""));
  ui->tool_wk_next->setIcon( LXDG::findIcon("go-next-view","") );
  ui->tool_logout->setIcon( LXDG::findIcon("system-log-out","") );
}

LSysMenuQuick::~LSysMenuQuick(){
	
}

void LSysMenuQuick::UpdateMenu(){
  ui->retranslateUi(this);
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
  if(val < 0){
    //No brightness control - hide it
    ui->group_brightness->setVisible(false);
  }else{
    ui->group_brightness->setVisible(true);
    txt = QString::number(val)+"%";
    if(val<100){ txt.prepend(" "); } //make sure no widget resizing
    ui->label_bright_text->setText(txt);
    if(ui->slider_brightness->value()!=val){ ui->slider_brightness->setValue(val); }
  }
  
  //Do any one-time checks
  if(firstrun){
    hasBat = LOS::hasBattery(); //No need to check this more than once - will not change in the middle of a session
    //Current Locale
    QStringList locales = LUtils::knownLocales();
    ui->combo_locale->clear();
    QLocale curr;
    for(int i=0; i<locales.length(); i++){
      QLocale loc( (locales[i]=="pt") ? "pt_PT" : locales[i] );
      ui->combo_locale->addItem(loc.nativeLanguageName()+" ("+locales[i]+")", locales[i]); //Make the display text prettier later
      if(locales[i] == curr.name() || locales[i] == curr.name().section("_",0,0) ){
        //Current Locale
	ui->combo_locale->setCurrentIndex(ui->combo_locale->count()-1); //the last item in the list right now
      }
    }
    ui->group_locale->setVisible(locales.length() > 1);
  }
  
  //Battery Status
  if(hasBat){
    ui->group_battery->setVisible(true);
    val = LOS::batteryCharge();
    if(LOS::batteryIsCharging()){
      if(val < 15){ ui->label_bat_icon->setPixmap( LXDG::findIcon("battery-charging-low","").pixmap(ui->label_bat_icon->maximumSize()) ); }
      else if(val < 30){ ui->label_bat_icon->setPixmap( LXDG::findIcon("battery-charging-caution","").pixmap(ui->label_bat_icon->maximumSize()) ); }
      else if(val < 50){ ui->label_bat_icon->setPixmap( LXDG::findIcon("battery-charging-040","").pixmap(ui->label_bat_icon->maximumSize()) ); }
      else if(val < 70){ ui->label_bat_icon->setPixmap( LXDG::findIcon("battery-charging-060","").pixmap(ui->label_bat_icon->maximumSize()) ); }
      else if(val < 90){ ui->label_bat_icon->setPixmap( LXDG::findIcon("battery-charging-080","").pixmap(ui->label_bat_icon->maximumSize()) ); }
      else{ ui->label_bat_icon->setPixmap( LXDG::findIcon("battery-charging","").pixmap(ui->label_bat_icon->maximumSize()) ); }
      ui->label_bat_text->setText( QString("%1%\n(%2)").arg(QString::number(val), tr("connected")) );
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
  val = LSession::handle()->XCB->CurrentWorkspace();
  int tot = LSession::handle()->XCB->NumberOfWorkspaces();
  ui->group_workspace->setVisible(val>=0 && tot>1);
  ui->label_wk_text->setText( QString(tr("%1 of %2")).arg(QString::number(val+1), QString::number(tot)) );
}

void LSysMenuQuick::volSliderChanged(){
  int val = ui->slider_volume->value();
  LOS::setAudioVolume(val);
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
  //Brightness controls cannot operate extremely quickly - combine calls as necessary
  if(brighttimer->isActive()){ brighttimer->stop(); }
  brighttimer->start();
  //*DO* update the label right away
  int val = ui->slider_brightness->value();
  QString txt = QString::number(val)+"%";
  if(val<100){ txt.prepend(" "); } //make sure no widget resizing
  ui->label_bright_text->setText( txt );
}

void LSysMenuQuick::setCurrentBrightness(){
  int val = ui->slider_brightness->value();
  LOS::setScreenBrightness(val);
  QString txt = QString::number(val)+"%";
  if(val<100){ txt.prepend(" "); } //make sure no widget resizing
  ui->label_bright_text->setText( txt );	
}

void LSysMenuQuick::nextWorkspace(){
  int cur = LSession::handle()->XCB->CurrentWorkspace();
  int tot = LSession::handle()->XCB->NumberOfWorkspaces();
  //qDebug()<< "Next Workspace:" << cur << tot;
  cur++;
  if(cur>=tot){ cur = 0; } //back to beginning
  //qDebug() << " - New Current:" << cur;
  LSession::handle()->XCB->SetCurrentWorkspace(cur);
ui->label_wk_text->setText( QString(tr("%1 of %2")).arg(QString::number(cur+1), QString::number(tot)) );
}

void LSysMenuQuick::prevWorkspace(){
  int cur = LSession::handle()->XCB->CurrentWorkspace();
  int tot = LSession::handle()->XCB->NumberOfWorkspaces();
  cur--;
  if(cur<0){ cur = tot-1; } //back to last
  LSession::handle()->XCB->SetCurrentWorkspace(cur);
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
  LSession::handle()->systemWindow();
}

void LSysMenuQuick::changeLocale(){
  //Get the currently selected Locale
  QString locale = ui->combo_locale->currentData().toString();
  emit CloseMenu();
  LSession::handle()->switchLocale(locale);
}
