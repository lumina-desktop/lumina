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
	
  //Setup connections
  connect(EFILTER, SIGNAL(NewInputEvent()), SS, SLOT(newInputEvent()) );
}

WMSession::~WMSession(){
}

void WMSession::start(bool SSONLY){
  //Get the screensaver initialized/ready
  if(DEBUG){ qDebug() << "Starting Screen Saver..."; }
  SS->start();
  if(SSONLY){ return; }
  //Now start pulling/filtering events
  if(DEBUG){ qDebug() << "Starting Event Filter..."; }
  EFILTER->start();
  if(DEBUG){ qDebug() << "Done Starting WM session..."; }
}
	
// ==========
//    Public Slots
// ==========
void WMSession::reloadIcons(){
	
}

void WMSession::newInputsAvailable(QStringList){
	
}
