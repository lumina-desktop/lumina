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

  LOCKER = new LLockScreen(this);
	LOCKER->hide();
  settings = new QSettings("lumina-desktop","lumina-screensaver",this);
  SSRunning = SSLocked = updating = false;
  this->setObjectName("LSCREENSAVERBASE");
  this->setStyleSheet("LScreenSaver#LSCREENSAVERBASE{ background: grey; }");
  this->setMouseTracking(true);
  connect(starttimer, SIGNAL(timeout()), this, SLOT(ShowScreenSaver()) );
  connect(locktimer, SIGNAL(timeout()), this, SLOT(LockScreen()) );
  connect(hidetimer, SIGNAL(timeout()), this, SLOT(HideLockScreen()) );
  connect(LOCKER, SIGNAL(ScreenUnlocked()), this, SLOT(SSFinished()) );
  connect(LOCKER, SIGNAL(InputDetected()), this, SLOT(newInputEvent()) );
}

LScreenSaver::~LScreenSaver(){

}

bool LScreenSaver::isLocked(){
  return SSLocked;
}

void LScreenSaver::UpdateTimers(){
  //This is generally used for programmatic changes
  if(starttimer->isActive()){ starttimer->stop();}
  if(locktimer->isActive()){ locktimer->stop(); }
  if(hidetimer->isActive()){ hidetimer->stop(); }

  if(!SSRunning && !SSLocked && (starttimer->interval() > 1000) ){ starttimer->start(); }  //time to SS start
  else if( SSRunning && !SSLocked && (locktimer->interval() > 1000 ) ){ locktimer->start(); } //time to lock
  else if( !SSRunning && SSLocked ){ hidetimer->start(); } //time to hide lock screen
}

// ===========
//  PUBLIC SLOTS
// ===========
void LScreenSaver::start(){
  reloadSettings(); //setup all the initial time frames
  starttimer->start();
}

void LScreenSaver::reloadSettings(){
  settings->sync();
  starttimer->setInterval( settings->value("timedelaymin",10).toInt() * 60000 );
  locktimer->setInterval( settings->value("lockdelaymin",1).toInt() * 60000 );
  hidetimer->setInterval( settings->value("hidesecs",15).toInt() * 1000 );
}

void LScreenSaver::newInputEvent(){
  if(updating){ return; } //in the middle of making changes which could cause an event
  if(DEBUG){ qDebug() << "New Input Event"; }
  if(SSRunning && SSLocked){
    //Running and locked
    // Hide the running setting, and display the lock screen
    HideScreenSaver();
    ShowLockScreen();
  }else if(SSRunning){
    //Only running, not locked
    HideScreenSaver();
  }
  UpdateTimers();
}

void LScreenSaver::LockScreenNow(){
  ShowScreenSaver();
  LockScreen();
}

// ===========
//  PRIVATE SLOTS
// ===========
void LScreenSaver::ShowScreenSaver(){
  if(DEBUG){ qDebug() << "Showing Screen Saver:" << QDateTime::currentDateTime().toString(); }
  //QApplication::setOverrideCursor(QCursor::BlankCursor);
  SSRunning = true;
  updating = true;
  //Now remove any current Base widgets (prevent any lingering painting between sessions)
  for(int i=0; i<BASES.length(); i++){
    if(DEBUG){ qDebug() << " - Removing SS Base"; }
    delete BASES.takeAt(i); i--;
  }
  //Now go through and create/show all the various widgets
  QList<QScreen*> SCREENS = QApplication::screens();
  QRect bounds;
  cBright = LOS::ScreenBrightness();
  if(cBright>0){ LOS::setScreenBrightness(cBright/2); } //cut to half while the screensaver is active
  for(int i=0; i<SCREENS.length(); i++){
    bounds = bounds.united(SCREENS[i]->geometry());
    if(DEBUG){ qDebug() << " - New SS Base:" << i; }
    BASES << new SSBaseWidget(this, settings);
    connect(BASES[i], SIGNAL(InputDetected()), this, SLOT(newInputEvent()) );
    //Setup the geometry of the base to match the screen
    BASES[i]->setGeometry(SCREENS[i]->geometry());  //match this screen geometry
    BASES[i]->setPlugin(settings->value("screenplugin"+QString::number(i+1), settings->value("defaultscreenplugin","random").toString() ).toString() );
  }
  //Now set the overall parent widget geometry and show everything
  this->setGeometry(bounds); //overall background widget
  if(!this->isActiveWindow()){
    this->raise();
    this->show();
    this->activateWindow();
  }
  for(int i=0; i<BASES.length(); i++){
    BASES[i]->show();
    BASES[i]->startPainting();
  }
  updating = false;
  UpdateTimers();
}

void LScreenSaver::ShowLockScreen(){
  if(DEBUG){ qDebug() << "Showing Lock Screen:" << QDateTime::currentDateTime().toString(); }
  LOCKER->aboutToShow();
  //Move the screen locker to the appropriate spot
  QPoint ctr = QApplication::desktop()->screenGeometry(QCursor::pos()).center();
  LOCKER->resize(LOCKER->sizeHint());
  LOCKER->move(ctr - QPoint(LOCKER->width()/2, LOCKER->height()/2) );
  LOCKER->show();
  //Start the timer for hiding the lock screen due to inactivity
  UpdateTimers();
}

void LScreenSaver::HideScreenSaver(){
  //QApplication::restoreOverrideCursor();
  if(DEBUG){ qDebug() << "Hiding Screen Saver:" << QDateTime::currentDateTime().toString(); }
  SSRunning = false;
  if(cBright>0){ LOS::setScreenBrightness(cBright); } //return to current brightness
  if(!SSLocked){
    this->hide();
    emit ClosingScreenSaver();
    emit LockStatusChanged(false);
  }
  for(int i=0; i<BASES.length(); i++){
    BASES[i]->hide();
    BASES[i]->stopPainting();
  }
  UpdateTimers();
}

void LScreenSaver::HideLockScreen(){
  if(DEBUG){ qDebug() << "Hiding Lock Screen:" << QDateTime::currentDateTime().toString(); }
  //Leave the Locked flag set (still locked, just not visible)
  LOCKER->aboutToHide();
  LOCKER->hide();
  this->repaint();
  if(SSLocked){ ShowScreenSaver(); }
  UpdateTimers();
}

void LScreenSaver::LockScreen(){
  if(SSLocked){ return; }
  if(DEBUG){ qDebug() << "Locking Screen:" << QDateTime::currentDateTime().toString(); }
  SSLocked = true;
  emit LockStatusChanged(true);
  LOCKER->LoadSystemDetails();
  UpdateTimers();
}

void LScreenSaver::SSFinished(){
  if(DEBUG){ qDebug() << "Screensaver Finished:" << QDateTime::currentDateTime().toString(); }
  SSLocked = false;
  emit LockStatusChanged(false);
  HideLockScreen();
  HideScreenSaver();
}
