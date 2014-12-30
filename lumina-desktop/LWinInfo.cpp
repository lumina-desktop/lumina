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
  qDebug() << "Window Visible Icon Name:" << window;
  if(window==0){ return ""; }
  QString nm = LSession::handle()->XCB->WindowVisibleIconName(window);
  if(nm.isEmpty()){ qDebug() << " - Window Icon Name"; nm = LSession::handle()->XCB->WindowIconName(window); }
  if(nm.isEmpty()){ qDebug() << " - Window Visible Name";nm = LSession::handle()->XCB->WindowVisibleName(window); }
  if(nm.isEmpty()){ qDebug() << " - Window Name";nm = LSession::handle()->XCB->WindowName(window); }
  return nm;
}

QIcon LWinInfo::icon(bool &noicon){
  if(window==0){ noicon = true; return QIcon();}
  qDebug() << "Check for Window Icon:" << window;
  noicon = false;
  QIcon ico = LX11::WindowIcon(window);
  //Check for a null icon, and supply one if necessary
  if(ico.isNull()){ qDebug() << " - Class Icon"; ico = LXDG::findIcon( this->Class().toLower(),""); }
  if(ico.isNull()){qDebug() << " - Default Icon"; ico = LXDG::findIcon("preferences-system-windows",""); noicon=true;}
  return ico;
}
	
QString LWinInfo::Class(){
  qDebug() << "Window Class:" << window;
  return LSession::handle()->XCB->WindowClass(window);
}
	
Lumina::STATES LWinInfo::status(){
  if(window==0){ return Lumina::NOSHOW; }
  LXCB::WINDOWSTATE ws = LSession::handle()->XCB->WindowState(window);
  //LX11::WINDOWSTATE ws = LX11::GetWindowState(window);
  Lumina::STATES state;
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
  }
  //qDebug() << "Window State:" << ws << state;
  return state;
}