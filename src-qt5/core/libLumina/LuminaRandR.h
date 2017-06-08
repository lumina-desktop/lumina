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

// XCB
#include "xcb/randr.h"
#include "xcb/xcb_atom.h"

struct p_objects{
//public:
	xcb_atom_t monitor_atom; //This is the index used to identify particular monitors (unique ID)

	//Cached Information
	bool primary, automatic;
	QRect geometry;
	QList<QSize> resolutions;
	QSize physicalSizeMM;
	QString name;
	QList<xcb_randr_output_t> outputs;

	/*p_objects(){
          // Set the defaults for non-default-constructed variables
	  primary = automatic = false;
	  monitor_atom = 0;
	}*/
	
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
	bool isAutomatic();
	QList<QSize> availableResolutions();
	QSize currentResolution(); //could be different from geometry.size() if things like panning/rotation are enabled
	QRect currentGeometry();
	
	//Modification
	bool setAsPrimary(bool);
	bool disable();
	void enable(QRect geom = QRect()); //if no geom provided, will add as the right-most screen at optimal resolution
	void changeResolution(QSize);
	void changeGeometry(QRect); //move a currently-enabled screen to another place

	void updateInfoCache(); //Run this after all modification functions to refresh the current info for this device

	//Now define a simple public_objects class so that each implementation 
	//  has a storage container for placing semi-private objects as needed
	//class p_objects; //forward declaration - defined in the .cpp file
	p_objects p_obj;
};

class OutputDeviceList : public QList<OutputDevice>{
public:
	OutputDeviceList();
	~OutputDeviceList();

	//Simplification functions for dealing with multiple monitors
	void setPrimaryMonitor(QString id);
	void disableMonitor(QString id);
	//void enableMonitor(QString id, 

};
#endif
