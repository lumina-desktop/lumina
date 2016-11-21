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
#include <QHash>
#include <QVariant>

#include <xcb/xproto.h>

class LInputDevice{
public:
	LInputDevice(unsigned int id, unsigned int type); //don't use this directly - use the "listDevices()" function instead
	~LInputDevice();

	//Device Information
	//QString name(); //Return the name of this device
	unsigned int devNumber();
	bool isPointer();
	bool isKeyboard();
	bool isExtension();

	//Property Management
	QStringList listProperties();
	QVariant propertyValue(QString prop);

private:
  unsigned int devID; //device ID number - assigned at class creation
  unsigned int devType; //device "use" identifier - assigned at class creation
  QHash<QString, xcb_atom_t> devProps; //Known device properties <name, atom>

  void getProperties();
  //QString devName; //device name - use this for cross-session management (id #'s can get changed every session)
};

//Static functions for overall management
class LInput{
  public:
  static QList<LInputDevice*> listDevices(); //NOTE: Make sure you "free()" all the LInputDevice objects when finished

};

#endif
