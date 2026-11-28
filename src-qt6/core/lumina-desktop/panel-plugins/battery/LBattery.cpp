//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Susanne Jaeckel, 2015-2016 Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LBattery.h"
#include "LSession.h"

LBattery::LBattery(QWidget *parent, QString id, bool horizontal) : LPPlugin(parent, id, horizontal){
  iconOld = QString();
  //Setup the widget
  label = new QLabel(this);
    label->setScaledContents(true);
    //label->setAlignment(Qt::AlignCenter);
  this->layout()->addWidget(label);
  //Setup the timer
  timer = new QTimer();
  timer->setInterval(5000); //update every 5 seconds
  connect(timer,SIGNAL(timeout()), this, SLOT(updateBattery()) );
  timer->start();
  QTimer::singleShot(0,this,SLOT(OrientationChange()) ); //update the sizing/icon
  sessionsettings = new QSettings("lumina-desktop", "sessionsettings", this); //owned by the plugin (was parentless and never deleted)
}

LBattery::~LBattery(){
  timer->stop();
  delete timer;
}

void LBattery::updateBattery(bool force){
  // Get current state of charge
  int charge = LOS::batteryCharge();
  bool charging = LOS::batteryIsCharging();
  QString batt_icon = LSession::batteryIconName(charge, charging);
  if(iconOld != batt_icon){
    label->setPixmap( QIcon::fromTheme(batt_icon).pixmap(label->size()) );
    if(charge <= 10 && !charging){
      //Play some audio warning chime when
      bool playaudio = sessionsettings->value("PlayBatteryLowAudio",true).toBool();
      if( playaudio ){
        QString sfile = LSession::handle()->sessionSettings()->value("audiofiles/batterylow", "").toString();
        if(sfile.isEmpty()){ sfile = LOS::LuminaShare()+"low-battery.ogg"; }
        LSession::handle()->playAudioFile(sfile);
      }
    }
    iconOld = batt_icon; //save for later
  }

  /*
   *  Use 40% - 80% rule for icon notifications.  Lithium batteries last
   *  longer if kept between 40% and 80% charge as much as possible.
   *  Turn icon yellow when charge drops to 40% and green when charge
   *  rises to 80%.  This will help the user maintain their battery
   *  in an optimal way.
   */
  QString bg = "transparent";
  if(charge<=20 && !charging){ bg = "red"; }
  else if(charge<=30 && !charging){ bg = "orange"; }
  else if(charge<=40 && !charging){ bg = "yellow"; }
  else if(charge>80 && charging){ bg = "green"; }
  //(setting a stylesheet re-polishes the widget, so only do it when the color actually changes - it was every 5 s)
  QString style = "QLabel{ background: "+bg+";}";
  if(label->styleSheet() != style){ label->setStyleSheet(style); }

  //Now update the display
  QString tt;
  //Make sure the tooltip can be properly translated as necessary (Ken Moore 5/9/14)
  if(charging){ tt = QString(tr("%1 % (Charging)")).arg(QString::number(charge)); }
  else{ tt = QString( tr("%1 % (%2 Remaining)") ).arg(QString::number(charge), getRemainingTime() ); }
  label->setToolTip(tt);
}

QString LBattery::getRemainingTime(){
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
    rem.append(QString::number(secs)+"s");
  }
  return rem;
}
