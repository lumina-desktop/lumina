//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "WMSession.h"

#define DEBUG 1
// ==========
//       PUBLIC
// ==========
WMSession::WMSession(){
  if(DEBUG){ qDebug() << "Creating Event Filter..."; }
  EFILTER = new EventFilter();
  if(DEBUG){ qDebug() << "Creating Screen Saver..."; }
  SS = new LScreenSaver();
  if(DEBUG){ qDebug() << "Creating Window Manager..."; }
  WM = new LWindowManager();  
  EVThread = new QThread();
    EFILTER->moveToThread(EVThread);
  //Setup connections
  connect(EFILTER, SIGNAL(NewInputEvent()), SS, SLOT(newInputEvent()) );
  connect(EFILTER, SIGNAL(NewManagedWindow(WId)), WM, SLOT(NewWindow(WId)) );
  connect(EFILTER, SIGNAL(WindowClosed(WId)), WM, SLOT(ClosedWindow(WId)) );
  connect(EFILTER, SIGNAL(ModifyWindow(WId, LWM::WindowAction)), WM, SLOT(ModifyWindow(WId,LWM::WindowAction)) );
  connect(SS, SIGNAL(StartingScreenSaver()), EFILTER, SLOT(StartedSS()) );
  connect(SS, SIGNAL(ClosingScreenSaver()), EFILTER, SLOT(StoppedSS()) );
  connect(WM, SIGNAL(NewFullScreenWindows(QList<WId>)), EFILTER, SLOT(FullScreenChanged(QList<WId>)) );
}

WMSession::~WMSession(){
}

void WMSession::start(bool SSONLY){
  //Get the screensaver initialized/ready
  if(DEBUG){ qDebug() << "Starting Screen Saver..."; }
  SS->start();
  if(SSONLY){ return; }
  //Now start pulling/filtering events
  if(DEBUG){ qDebug() << "Starting Window Manager..."; }
  WM->start();
  if(DEBUG){ qDebug() << "Starting Event Filter..."; }
  EVThread->start();
  EFILTER->start();
  if(DEBUG){ qDebug() << "Done Starting WM session..."; }
}
	
// ==========
//    Public Slots
// ==========
void WMSession::reloadIcons(){
	
}

void WMSession::newInputsAvailable(QStringList inputs){
  for(int i=0; i<inputs.length(); i++){
    if(inputs[i]=="--lock-now" || inputs[i]=="--test-ss"){
      QTimer::singleShot(0,SS, SLOT(LockScreenNow()) );
    }
  }
}
