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


class OutputDevice{
public:
  QString id; //output ID
  bool enabled;
  bool isPrimary;
  //Monitor Geometry
  QPoint geom; //geometry of monitor within session
  //Monitor Resolution
  QSize cRes; //current resolution of the monitor (could be different from geom.size() if panning is enabled)
  QList<QSize> availRes; //available resolutions supported by the monitor
  //Refresh Rate
  //int cHz; //current refresh rate
  //QList<int> availHz; //available refresh rates
  //Expand this later to include:
  // panning (current/possible)
  // rotation (current/possible)

	//Global Listing of Devices
	static QList<OutputDevice> availableMonitors();

	//FUNCTIONS (do not use directly - use the static list function instead)
	OutputDevice();
	~OutputDevice();

	//Modification
	bool setAsPrimary();
	bool disable();
	void enable(QRect geom = QRect()); //if no geom provided, will add as the right-most screen at optimal resolution
	void changeResolution(QSize);
	
	//Now define a simple public_objects class so that each implementation 
	//  has a storage container for placing private objects as needed
	class p_objects;
	p_objects* p_obj;
};

class OutputDeviceList : public QList<OutputDevice>{
public:
	OutputDeviceList();
	~OutputDeviceList();

	//Simplification functions for dealing with multiple monitors
	void setPrimaryMonitor(QString id);
	void disableMonitor(QString id);
	//void enableMonitor(QString id, 

private:

};
#endif
