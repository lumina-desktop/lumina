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
  if(nm.isEmpty()){ nm = LSession::handle()->XCB->WindowIconName(window); }
  if(nm.isEmpty()){ nm = LSession::handle()->XCB->WindowVisibleName(window); }
  if(nm.isEmpty()){ nm = LSession::handle()->XCB->WindowName(window); }
  return nm;
}

QIcon LWinInfo::icon(bool &noicon){
  if(window==0){ noicon = true; return QIcon();}
  noicon = false;
  QIcon ico = LX11::WindowIcon(window);
  //Check for a null icon, and supply one if necessary
  if(ico.isNull()){ ico = LXDG::findIcon( this->Class().toLower(),""); }
  if(ico.isNull()){ico = LXDG::findIcon("preferences-system-windows",""); noicon=true;}
  return ico;
}
	
QString LWinInfo::Class(){
  return LSession::handle()->XCB->WindowClass(window);
}
	
LXCB::WINDOWSTATE LWinInfo::status(){
  if(window==0){ return LXCB::IGNORE; }
  LXCB::WINDOWSTATE ws = LSession::handle()->XCB->WindowState(window);
  //LX11::WINDOWSTATE ws = LX11::GetWindowState(window);
 /*Lumina::STATES state;
  switch(ws){
    case LXCB::VISIBLE:
	    state = Lumina::VISIBLE; break;
    case LXCB::INVISIBLE:
	    state = Lumina::INVISIBLE; break;
    case LXCB::ACTIVE:
	    state = Lumina::ACTIVE; break;
    case LXCB::ATTENTION:
	    state = Lumina::NOTIFICATION; break;
    default:
	    state = Lumina::NOSHOW;
  }*/
  //qDebug() << "Window State:" << ws << state;
  return ws;
}