//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_GLOBALS_H
#define _LUMINA_DESKTOP_GLOBALS_H

#include <LUtils.h>
#include <LuminaXDG.h>
#include <LuminaOS.h>
#include <LDesktopUtils.h>
#include <LFileInfo.h>

#include <QWidgetAction>
#include <QMenu>
#include <QString>
#include <QIcon>
#include <QWidget>
#include <QAction>
#include <QToolButton>

#include <unistd.h>
#include <stdio.h>

class Lumina{
public:
  enum STATES {NONE, VISIBLE, INVISIBLE, ACTIVE, NOTIFICATION, NOSHOW};

};

class SYSTEM{
public:
	//Installation location for finding default files
	//static QString installDir(){ return PREFIX + "/share/Lumina-DE/"; }
	//Current Username
	static QString user(){ return QString::fromLocal8Bit(getlogin()); }
	//Current Hostname
	static QString hostname(){
	  char name[BUFSIZ];
	  int count = gethostname(name,sizeof(name));
	  if (count < 0) {
	    return QString();
	  }
	  return QString::fromLocal8Bit(name,count);
	}

};

#endif
