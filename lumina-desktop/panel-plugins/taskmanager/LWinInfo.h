//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_WINDOW_INFO_H
#define _LUMINA_DESKTOP_WINDOW_INFO_H

// Qt includes
#include <QString>
#include <QPixmap>
#include <QIcon>
#include <QPainter>

// libLumina includes
#include <LuminaX11.h>
#include <LuminaXDG.h>

// Local includes
#include "../../Globals.h" //For the STATES enumeration definition


class LWinInfo{
private:
	WId window;

public:
	LWinInfo(WId id = 0){
	  window = id;
	}
	~LWinInfo(){};
	
	//The current window ID
	WId windowID(){
	  return window;
	}
	
	//Information Retrieval
	 // Don't cache these results because they can change regularly
	QString  text(){
	  if(window==0){ return ""; }
	  QString nm = LX11::WindowVisibleIconName(window);
	  if(nm.isEmpty()){ nm = LX11::WindowIconName(window); }
	  if(nm.isEmpty()){ nm = LX11::WindowVisibleName(window); }
	  if(nm.isEmpty()){ nm = LX11::WindowName(window); }
	  return nm;
	}

	QIcon icon(){
	  if(window==0){ return QIcon(); }
	  //qDebug() << "Check for Window Icon:" << window;
	  QIcon ico = LX11::WindowIcon(window);
	  //Check for a null icon, and supply one if necessary
	  //if(ico.isNull()){ ico = LXDG::findIcon("preferences-system-windows",""); }
	  return ico;
	}
	
	QString Class(){
	  return LX11::WindowClass(window);
	}
	
	Lumina::STATES status(){
	  if(window==0){ return Lumina::NOSHOW; }
	  LX11::WINDOWSTATE ws = LX11::GetWindowState(window);
	  Lumina::STATES state;
	  switch(ws){
	    case LX11::VISIBLE:
		    state = Lumina::VISIBLE; break;
	    case LX11::INVISIBLE:
		    state = Lumina::INVISIBLE; break;
	    case LX11::ACTIVE:
		    state = Lumina::ACTIVE; break;
	    case LX11::ATTENTION:
		    state = Lumina::NOTIFICATION; break;
	    default:
		    state = Lumina::NOSHOW;
	  }
	  //qDebug() << "Window State:" << ws << state;
	  return state;
	}
};

#endif