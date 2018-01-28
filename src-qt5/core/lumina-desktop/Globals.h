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
	    return QString::null;
	  }
	  return QString::fromLocal8Bit(name,count);
	}
	/*//Shutdown the system
#ifdef __linux
	static void shutdown(){ system("(shutdown -h now) &"); }
#else // #ifdef __linux
	static void shutdown(){ system("(shutdown -p now) &"); }
#endif // #ifdef __linux
	//Restart the system
	static void restart(){ system("(shutdown -r now) &"); }

	//Determine if there is battery support
	static bool hasBattery(){
	  int val = LUtils::getCmdOutput("apm -l").join("").toInt();
	  return (val >= 0 && val <= 100);
	}

	//Get the current battery charge percentage
	static int batteryCharge(){
	  int charge = LUtils::getCmdOutput("apm -l").join("").toInt();
	  if(charge > 100){ charge = -1; } //invalid charge
	  return charge;
	}

	//Get the current battery charge percentage
	static bool batteryIsCharging(){
	  return (LUtils::getCmdOutput("apm -a").join("").simplified() == "1");
	}

	//Get the amount of time remaining for the battery
	static int batterySecondsLeft(){
	  return LUtils::getCmdOutput("apm -t").join("").toInt();
	}*/
};

#endif
