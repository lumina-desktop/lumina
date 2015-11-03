//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LScreenSaver.h"
#include <QScreen>
#include <QApplication>

#define DEBUG 1

LScreenSaver::LScreenSaver() : QWidget(0,Qt::BypassWindowManagerHint | Qt::WindowStaysOnTopHint){
  starttimer = new QTimer(this);
    starttimer->setSingleShot(true);
  locktimer = new QTimer(this);
    locktimer->setSingleShot(true);
  hidetimer = new QTimer(this);
    hidetimer->setSingleShot(true);
	
  settings = new QSettings("LuminaDE","lumina-screensaver",this);
  SSRunning = SSLocked = false;
  this->setObjectName("LSCREENSAVERBASE");
  this->setStyleSheet("LScreenSaver#LSCREENSAVERBASE{ background: black; }");
  connect(starttimer, SIGNAL(timeout()), this, SLOT(ShowScreenSaver()) );
}

LScreenSaver::~LScreenSaver(){
	
}

bool LScreenSaver::isLocked(){
  return SSLocked;
}

// ===========
//  PUBLIC SLOTS
// ===========
void LScreenSaver::start(){
  reloadSettings(); //setup all the initial time frames
  starttimer->start(1000);// one second delay (for testing)
}

void LScreenSaver::reloadSettings(){
  settings->sync();
  starttimer->setInterval( settings->value("timedelaymin",10).toInt() * 60000 );
  locktimer->setInterval( settings->value("lockdelaymin",1).toInt() * 60000 );
  hidetimer->setInterval( settings->value("hidesecs",15).toInt() * 1000 );
}

void LScreenSaver::newInputEvent(){
  //First stop any timers that are running
  if(starttimer->isActive()){ starttimer->stop();}
  if(locktimer->isActive()){ locktimer->stop(); }
  if(hidetimer->isActive()){ hidetimer->stop(); }
    
  if(SSRunning && SSLocked){
    //Running and locked
    // Hide the running setting, and display the lock screen
    HideScreenSaver();
    ShowLockScreen();
    //Start the timer for restarting the SS and hiding the lockscreen
    hidetimer->start();
	  
  }else if(SSRunning){
    //Only running, not locked
    //De-activate the screensaver and start the main timer
    HideScreenSaver();
    starttimer->start();
	  
  }else if(SSLocked){
    //Only locked, not running
    hidetimer->start(); //restart the time to hide the lock screen
    
  }else{
    //Neither running nor locked
    if( settings->value("timedelaymin",10).toInt() > 0 ){ starttimer->start(); }
  }
  
}

// ===========
//  PRIVATE SLOTS
// ===========
void LScreenSaver::ShowScreenSaver(){
  if(DEBUG){ qDebug() << "Showing Screen Saver:" << QDateTime::currentDateTime().toString(); }
  SSRunning = true;
  //Start the lock timer if necessary
  if(!SSLocked && (settings->value("lockdelaymin",10).toInt()>0) ){ locktimer->start(); }
  //Now go through and create/show all the various widgets
  QList<QScreen*> SCREENS = QApplication::screens();
  QRect bounds;
  for(int i=0; i<SCREENS.length(); i++){
    bounds = bounds.united(SCREENS[i]->geometry());
    if( (BASES.length()-1) < i){
      //Nothing for this screen yet - go ahead and create one
      BASES << new SSBaseWidget(this, settings);
    }
    //Setup the geometry of the base to match the screen
    BASES[i]->setGeometry(SCREENS[i]->geometry());  //match this screen geometry
    BASES[i]->setPlugin(settings->value("screenplugin"+QString::number(i+1), settings->value("defaultscreenplugin","random").toString() ).toString() );

  }
  //Now remove any extra Base widgets (in case a screen was removed)
  for(int i=SCREENS.length(); i<BASES.length(); i++){
    delete BASES.takeAt(i); i--;
  }
  //Now set the overall parent widget geometry and show everything
  this->setGeometry(bounds); //overall background widget
  this->raise();
  this->show();
  this->activateWindow();
  for(int i=0; i<BASES.length(); i++){
    BASES[i]->show();
    QTimer::singleShot(100, BASES[i], SLOT(startPainting()) ); //start in 1/2 second
  }
}

void LScreenSaver::ShowLockScreen(){
  if(DEBUG){ qDebug() << "Locking Screen Saver:" << QDateTime::currentDateTime().toString(); }
  //SSLocked = true;
  //Start the timer for hiding the lock screen due to inactivity
  if(settings->value("hidesecs",15).toInt() > 0 ){ hidetimer->start(); }
}

void LScreenSaver::HideScreenSaver(){
  if(DEBUG){ qDebug() << "Hiding Screen Saver:" << QDateTime::currentDateTime().toString(); }
  SSRunning = false;
  if(!SSLocked){
    this->hide();
    emit ClosingScreenSaver();
  }
  for(int i=0; i<BASES.length(); i++){ QTimer::singleShot(0,BASES[i], SLOT(stopPainting())); }
}

void LScreenSaver::HideLockScreen(){
  if(DEBUG){ qDebug() << "Hiding Lock Screen:" << QDateTime::currentDateTime().toString(); }
  //Leave the Locked flag set (still locked, just not visible)
}
