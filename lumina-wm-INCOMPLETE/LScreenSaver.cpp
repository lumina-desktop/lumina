//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LScreenSaver.h"

LScreenSaver::LScreenSaver(){
  starttimer = new QTimer(this);
    starttimer->setSingleShot(true);
  locktimer = new QTimer(this);
    locktimer->setSingleShot(true);
  hidetimer = new QTimer(this);
    hidetimer->setSingleShot(true);
	
  settings = new QSettings("LuminaDE","lumina-screensaver",this);
  SSRunning = SSLocked = false;
	
  connect(starttimer, SIGNAL(timeout()), this, SLOT(ShowScreenSaver()) );
}

LScreenSaver::~LScreenSaver(){
	
}

// ===========
//  PUBLIC SLOTS
// ===========
void LScreenSaver::start(){
  reloadSettings(); //setup all the initial time frames
  
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
	
  SSRunning = true;
  //Start the lock timer if necessary
  if(!SSLocked && (settings->value("lockdelaymin",10).toInt()>0) ){ locktimer->start(); }
}

void LScreenSaver::ShowLockScreen(){
	
  SSLocked = true;
  //Start the timer for hiding the lock screen due to inactivity
  if(settings->value("hidesecs",15).toInt() > 0 ){ hidetimer->start(); }
}

void LScreenSaver::HideScreenSaver(){

  SSRunning = false;
}

void LScreenSaver::HideLockScreen(){

  //Leave the Locked flag set (still locked, just not visible)
}
