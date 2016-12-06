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

//Internal data structure for storing the property information
struct propData{
  int id;
  QString name;
  QVariant value;
  xcb_atom_t atom;
};

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
	QList<int> listProperties();
	QString propertyName(int prop);
	QVariant getPropertyValue(int prop);
	bool setPropertyValue(int prop, QVariant value);

private:
	unsigned int devID; //device ID number - assigned at class creation
	unsigned int devType; //device "use" identifier - assigned at class creation
	QHash<int, propData> devProps; //Known device properties <id#, properties struct>

	void getProperties();
	void readProperties();
	QVariant valueToVariant(QString value); //xinput output to QVariant
	QString variantToString(QVariant value); //QVariant to xinput input string

	//QString devName; //device name - use this for cross-session management (id #'s can get changed every session)
};

//Static functions for overall management
class LInput{
  public:
  static QList<LInputDevice*> listDevices(); //NOTE: Make sure you "free()" all the LInputDevice objects when finished

};

#endif
