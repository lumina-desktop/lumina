//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "WMSession.h"

// ==========
//       PUBLIC
// ==========
WMSession::WMSession(){
  EFILTER = new EventFilter();
  SS = new LScreenSaver();
	
  //Setup connections
  connect(EFILTER, SIGNAL(NewInputEvent()), SS, SLOT(newInputEvent()) );
}

WMSession::~WMSession(){
}

void WMSession::start(){
  //Get the screensaver initialized/ready
  SS->start();
  //Now start pulling/filtering events
  EFILTER->start();
	
}
	
// ==========
//    Public Slots
// ==========
void WMSession::reloadIcons(){
	
}

void WMSession::newInputsAvailable(QStringList){
	
}
