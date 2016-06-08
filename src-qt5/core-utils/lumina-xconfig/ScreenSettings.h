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

class ScreenInfo{
  public:	
	QString ID;
	QRect geom; //screen geometry
        bool isprimary;
	bool isactive;
	bool isavailable;
	int order; //left to right
	QStringList resList;

	//Initial Defaults
	ScreenInfo(){
	  order = -1; //initial value is invalid	
          isprimary = false;
          isactive = false;
	  isavailable = false;
	}
	~ScreenInfo(){}
};

class RRSettings{
public:
	//Reset current screen config to match previously-saved settings
	static void ApplyPrevious(); //generally performed on startup of the desktop

	//Read the current screen config from xrandr
	static QList<ScreenInfo> CurrentScreens(); //reads xrandr information

	//Save the screen config for later
	static bool SaveScreens(QList<ScreenInfo> screens);
	
	//Apply screen configuration
	static void Apply(QList<ScreenInfo> screens);
}; 

#endif
