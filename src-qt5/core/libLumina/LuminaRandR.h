//===========================================
//  Lumina-DE source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This class governs all the xcb/randr interactions
//  and provides simpler Qt-based functions for use elsewhere
//===========================================
#ifndef _LUMINA_LIBRARY_RANDR_MONITORS_H
#define _LUMINA_LIBRARY_RANDR_MONITORS_H

//Qt includes
#include <QSize>
#include <QString>
#include <QPoint>
#include <QRect>
#include <QList>
#include <QObject>
#include <QDebug>
#include <QX11Info>

// XCB
#include "xcb/randr.h"
#include "xcb/xcb_atom.h"

struct p_objects{
	xcb_randr_output_t output; //This is the index used to identify particular monitors (unique ID)
	xcb_randr_crtc_t crtc; //This is the index used for the current settings/configuration (associated with output)

	//Cached Information
	bool primary;
	QRect geometry;

	QSize physicalSizeMM; //physical size of the display in MM
	QString name;

	xcb_randr_mode_t current_mode;
	QList<xcb_randr_mode_t> modes; //each mode is a combination of resolution + refresh rate
	QList<QSize> resolutions; //smaller subset of modes - just unique resolutions

};

class OutputDevice{
public:

  // EXPANSIONS TO-DO
  //Refresh Rate
  //int cHz; //current refresh rate
  //QList<int> availHz; //available refresh rates
  //Expand this later to include:
  // panning (current/possible)
  // rotation (current/possible)

	//Global Listing of Devices
	static QList<OutputDevice> availableMonitors();

	//FUNCTIONS (do not use directly - use the static list function instead)
	OutputDevice(QString id);
	~OutputDevice();

	//Information
	QString ID();

	bool isEnabled();
	bool isPrimary();
	bool isConnected();
	QList<QSize> availableResolutions();
	QSize currentResolution(); //could be different from geometry.size() if things like panning/rotation are enabled
	QRect currentGeometry();
	QSize physicalSizeMM();
	QSize physicalDPI();

	//Modification
	bool setAsPrimary(bool);
	bool disable();
	bool enable(QRect geom); //if empty resolution is supplied (QSize(0,0)) it will use the highest-available resolution
	bool changeResolution(QSize); //change the resolution (but not position) of a currently-enabled screen
	bool move(QPoint); //move a currently-enabled screen to another place
	bool setGeometry(QRect); //move/resize a currently-enabled screen

	void updateInfoCache(); //Run this after all modification functions to refresh the current info for this device

	//Now define a simple public_objects class so that each implementation
	//  has a storage container for placing semi-private objects as needed
	//class p_objects; //forward declaration - defined in the .cpp file
	p_objects p_obj;
};

class OutputDeviceList{
private:
	QList<OutputDevice> out_devs;

public:
	OutputDeviceList();
	~OutputDeviceList();

	int length(){ return out_devs.length(); }

	OutputDevice* at(int i){
	  if(i<out_devs.length()){ return &out_devs[i]; }
           return 0;
	}

	//Simplification functions for dealing with multiple monitors
	void setPrimaryMonitor(QString id);
	QString primaryMonitor();

	bool disableMonitor(QString id);
	bool enableMonitor(QString id, QRect geom);

};
#endif
