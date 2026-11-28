//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_SCREEN_SETTINGS_BACKEND_H
#define _LUMINA_SCREEN_SETTINGS_BACKEND_H

#include <QString>
#include <QList>
#include <QRect>
#include <QStringList>

class ScreenInfo{
  public:
	QString ID;
	QRect geom; //screen geometry
        bool isprimary;
	bool isactive;
	bool isavailable;
	int applyChange; //[<=0: do nothing, 1: deactivate, 2: activate]
	QStringList resList;
	int rotation; //possible values: [-90, 0, 90, 180]


	//Initial Defaults
	ScreenInfo(){
	  applyChange = -1; //initial value is invalid
          isprimary = false;
          isactive = false;
	  isavailable = false;
	  rotation = 0; //no rotation by default
	}
	~ScreenInfo(){}
};

class RRSettings{
public:
	//Reset current screen config to match previously-saved settings
	static void ApplyPrevious();
	static void ApplyProfile(QString profile);
	//Setup all the connected monitors as a single mirror
	static void MirrorAll();

	//Read the current screen config from xrandr
	static QList<ScreenInfo> CurrentScreens(); //reads xrandr information
	static QList<ScreenInfo> PreviousSettings(QString profile="");
	static QStringList savedProfiles();
	static void removeProfile(QString profile);

	//Save the screen config for later
	static bool SaveScreens(QList<ScreenInfo> screens, QString profile = "");

	//Apply screen configuration
	static void Apply(QList<ScreenInfo> screens);
};

#endif
