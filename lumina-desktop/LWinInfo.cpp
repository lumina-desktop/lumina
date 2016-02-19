//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LWinInfo.h"

#include <LuminaX11.h>

#include "LSession.h"

//Information Retrieval
 // Don't cache these results because they can change regularly
QString  LWinInfo::text(){
  if(window==0){ return ""; }
  QString nm = LSession::handle()->XCB->WindowVisibleIconName(window);
  if(nm.simplified().isEmpty()){ nm = LSession::handle()->XCB->WindowIconName(window); }
  if(nm.simplified().isEmpty()){ nm = LSession::handle()->XCB->WindowVisibleName(window); }
  if(nm.simplified().isEmpty()){ nm = LSession::handle()->XCB->WindowName(window); }
  if(nm.simplified().isEmpty()){ nm = LSession::handle()->XCB->OldWindowIconName(window); }
  if(nm.simplified().isEmpty()){ nm = LSession::handle()->XCB->OldWindowName(window); }
  //Make sure that the text is a reasonable size (40 char limit)
  if(nm.length()>40){ nm = nm.left(40)+"..."; }
  return nm;
}

QIcon LWinInfo::icon(bool &noicon){
  if(window==0){ noicon = true; return QIcon();}
  noicon = false;
  QIcon ico = LSession::handle()->XCB->WindowIcon(window);
  //Check for a null icon, and supply one if necessary
  if(ico.isNull()){ ico = LXDG::findIcon( this->Class().toLower(),""); }
  if(ico.isNull()){ico = LXDG::findIcon("preferences-system-windows",""); noicon=true;}
  return ico;
}
	
QString LWinInfo::Class(){
  return LSession::handle()->XCB->WindowClass(window);
}
	
LXCB::WINDOWVISIBILITY LWinInfo::status(bool update){
  if(window==0){ return LXCB::IGNORE; }
  if(update || cstate == LXCB::IGNORE){
    cstate = LSession::handle()->XCB->WindowState(window);
  }
  return cstate;
}