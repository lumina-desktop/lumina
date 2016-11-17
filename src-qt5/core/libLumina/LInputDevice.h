//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This class governs all the XCB input device interactions
//  and provides simpler Qt-based functions for use elsewhere
//===========================================
#ifndef _LUMINA_XCB_INPUT_DEVICES_H
#define _LUMINA_XCB_INPUT_DEVICES_H

#include <QList>
#include <QString>
#include <QStringList>

class LInputDevice{
public:
	LInputDevice(unsigned int id, unsigned int type); //don't use this directly - use the "listDevices()" function instead
	~LInputDevice();

	//QString name(); //Return the name of this device
	unsigned int devNumber();
	bool isPointer();
	bool isKeyboard();
	bool isExtension();

	//List Properties of device
	QStringList listProperties();

private:
  unsigned int devID; //device ID number - assigned at class creation
  unsigned int devType; //device "use" identifier - assigned at class creation
  //QString devName; //device name - use this for cross-session management (id #'s can get changed every session)
};

//Static functions for overall management
class LInput{
  public:
  static QList<LInputDevice*> listDevices(); //NOTE: Make sure you "free()" all the LInputDevice objects when finished

};

#endif
