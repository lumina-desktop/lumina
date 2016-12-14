//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LInputDevice.h"

//Qt Library includes
#include <QString>
#include <QX11Info>
#include <QDebug>

//XCB Library includes
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/xinput.h>
#include <xcb/xproto.h>

#include <LUtils.h>

//===================
//    LInputDevice Class
//===================
// === PUBLIC ===
LInputDevice::LInputDevice(unsigned int id, unsigned int type){
  devID = id;
  devType = type;
  //ATOM_FLOAT = 0; //init this when needed later
  //devName = name;
  getProperties(); //need to populate the name/atom correlations for properties
  readProperties(); //populate the hash with the current values of the properties
}

LInputDevice::~LInputDevice(){

}

unsigned int LInputDevice::devNumber(){
  return devID;
}

bool LInputDevice::isPointer(){
  return (devType==XCB_INPUT_DEVICE_USE_IS_X_POINTER \
	|| devType==XCB_INPUT_DEVICE_USE_IS_X_EXTENSION_POINTER);
}

bool LInputDevice::isKeyboard(){
  return (devType==XCB_INPUT_DEVICE_USE_IS_X_KEYBOARD \
	|| devType==XCB_INPUT_DEVICE_USE_IS_X_EXTENSION_KEYBOARD);
}

bool LInputDevice::isExtension(){
  return (devType==XCB_INPUT_DEVICE_USE_IS_X_EXTENSION_DEVICE \
	|| devType==XCB_INPUT_DEVICE_USE_IS_X_EXTENSION_KEYBOARD \
	|| devType==XCB_INPUT_DEVICE_USE_IS_X_EXTENSION_POINTER);
}

// Property Management
QList<int> LInputDevice::listProperties(){
  return devProps.keys();
}

QString LInputDevice::propertyName(int prop){
  if(devProps.contains(prop)){ return devProps[prop].name; }
  else{ return ""; }
}

QVariant LInputDevice::getPropertyValue(int prop){
  if(devProps.contains(prop)){ return devProps[prop].value; }
  else{ return QVariant(); }
}

bool LInputDevice::setPropertyValue(int prop, QVariant value){
  if(!devProps.contains(prop)){ return false; }
  //Need the float atom for some properties - make sure we have that first
  /*if(ATOM_FLOAT==0){
    xcb_intern_atom_reply_t *ar = xcb_intern_atom_reply(QX11Info::connection(), \
			xcb_intern_atom(QX11Info::connection(), 0, 1, "FLOAT"), NULL);
    if(ar!=0){
      ATOM_FLOAT = ar->atom;
      free(ar);
    }
  }*/
  //Now setup the argument
  bool ok = false;
  QStringList args;
   args << "--set-prop";
   args << QString::number(devID);
   args << QString::number(prop); //prop ID
   args << variantToString(value);
  ok = (0 == LUtils::runCmd("xinput", args) );
  if(ok){ 
    //Need to update the value in the hash as well
    propData dat = devProps[prop];
      dat.value = value;
    devProps.insert(prop, dat);
  }
  return ok;
}

// === PRIVATE ===
void LInputDevice::getProperties(){
  devProps.clear();
  xcb_input_list_device_properties_cookie_t cookie = xcb_input_list_device_properties_unchecked(QX11Info::connection(), devID);
  xcb_input_list_device_properties_reply_t *reply = xcb_input_list_device_properties_reply(QX11Info::connection(), cookie, NULL);
  //Get the atoms
  xcb_atom_t *atoms = xcb_input_list_device_properties_atoms(reply);
  //qDebug() << "Property Response Type:" << reply->response_type; //Always seems to be "1"
  QList<xcb_get_atom_name_cookie_t> cookies;
  for(int i=0; i<reply->num_atoms; i++){  cookies <<  xcb_get_atom_name(QX11Info::connection(), atoms[i]);  }
  for(int i=0; i<reply->num_atoms; i++){
    xcb_get_atom_name_reply_t *nr = xcb_get_atom_name_reply(QX11Info::connection(), cookies[i], NULL);
    propData DATA;
      DATA.name = QString::fromUtf8( xcb_get_atom_name_name(nr), xcb_get_atom_name_name_length(nr) );
      DATA.atom = atoms[i];
      DATA.id = (int)(atoms[i]);
    devProps.insert(DATA.id,DATA);
    ::free(nr);
  }
  //Done with data structure
  ::free(reply);
}

void LInputDevice::readProperties(){
  QList<int> props = devProps.keys();
  //XINPUT UTILITY USAGE (alternative to XCB which actually works right now)
  QStringList info = LUtils::getCmdOutput("xinput list-props "+QString::number(devID));
  for(int i=0; i<props.length(); i++){
    propData PROP = devProps[props[i]];
    QStringList filter = info.filter(" ("+QString::number(PROP.id)+"):");
    if(filter.length()==1){
      QString val = filter.first().section("):",1,-1).simplified();
      //Now figure out what type of value this is and save it into the QVariant
      QVariant variant;
      if(val.split(", ").length()>1){
        //some kind of array
        QList<QVariant> list;
        QStringList valList = val.split(", ");
        for(int j=0; j<valList.length(); j++){ list << valueToVariant(valList[j]); }
        variant = QVariant(list);
      }else{
	variant = valueToVariant(val);
      }
      PROP.value = variant;
    }
    devProps.insert(props[i], PROP);
  }

//XCB Code (non-functional - issue with library itself? 12/6/16 - Ken Moore)
  /*QVariant result;
  if(!devProps.contains(prop)){qDebug() << "Invalid Property"; return result; }
  //Now generate the property request
  xcb_input_get_device_property_cookie_t cookie = xcb_input_get_device_property_unchecked( QX11Info::connection(), devProps.value(prop).atom, \
		XCB_ATOM_ATOM, 0, 1000, devID, 0);
  xcb_input_get_device_property_reply_t *reply = xcb_input_get_device_property_reply(QX11Info::connection(), cookie, NULL);
  if(reply==0){ qDebug() << "Could not get reply!"; return result; }
  //Now read off the value of the property
  if(ATOM_FLOAT==0){
    xcb_intern_atom_reply_t *ar = xcb_intern_atom_reply(QX11Info::connection(), \
			xcb_intern_atom(QX11Info::connection(), 0, 1, "FLOAT"), NULL);
    if(ar!=0){
      ATOM_FLOAT = ar->atom;
      free(ar);
    }
  }
  //Turn the reply into the proper items array (depends on format of the return data)
  xcb_input_get_device_property_items_t items;
  qDebug() <<QByteArray::fromRawData( (char*)(xcb_input_get_device_property_items(reply) ) , reply->num_items);
  void *buffer = xcb_input_get_device_property_items(reply);
  xcb_input_get_device_property_items_serialize( &buffer, reply->num_items, reply->format, &items);

  //if(reply->num_items > 0){
  //qDebug() << "Format:" << reply->format << "Length:" << length;
  //qDebug() << "Response Type:" << reply->response_type << "Pads:" << reply->pad0 << reply->pad1;
    switch(reply->type){
	case XCB_ATOM_INTEGER:
	  //qDebug() << "Got Integer";

	  break;
	case XCB_ATOM_CARDINAL:
	  //qDebug() << "Got Cardinal";

	  break;
	case XCB_ATOM_STRING:
	  qDebug() << "Got String:";
	  if(reply->format==8){
	    result.setValue( QByteArray::fromRawData( (char*) xcb_input_get_device_property_items_data_8(&items), sizeof(xcb_input_get_device_property_items_data_8(&items))/sizeof(char)) );
	  }
	  break;
	case XCB_ATOM_ATOM:
	  //qDebug() << "Got Atom";

	  break;
	default:
	    qDebug() << "Other Type:" << reply->type;
    }
  //}
  free(reply); //done with this structure
  return result;*/
}

QVariant LInputDevice::valueToVariant(QString value){
  //Read through the string and see what type of value it is
  if(value.count("\"")==2){
    //String value or atom
    if(value.endsWith(")")){
      //ATOM (name string +(atomID))
      return QVariant(value); //don't strip off the atom number -- keep that within the parenthesis
    }else{
      //String
      value = value.section("\"",1,-2); //everything between the quotes
      return QVariant(value);
    }
  }else if(value.contains(".")){
    //float/double number
    return QVariant( value.toDouble() );
  }else{
    //integer or boolian (no way to tell right now - assume all int)
    bool ok = false;
    int intval = value.toInt(&ok);
    if(ok){ return QVariant(intval); }
  }
  return QVariant();
}

QString LInputDevice::variantToString(QVariant value){
  if( value.canConvert< QList<QVariant> >() ){
    //List of variants
    QStringList out;
    QList<QVariant> list = value.toList();
    for(int i=0; i<list.length(); i++){ out << variantToString(list[i]); }
    return out.join(", ");
  }else{
    //Single value
    if(value.canConvert<double>() ){
      return QString::number(value.toDouble());
    }else if(value.canConvert<int>() ){
     return QString::number(value.toInt());
    }else if( value.canConvert<QString>() ){
      //See if this is an atom first
      QString val = value.toString();
      if(val.contains("(")){ val = val.section("(",1,-1).section(")",0,0); }
      return val;
    }
  }
  return ""; //nothing to return
}

//======================
//  LInput Static Functions
//======================
QList<LInputDevice*> LInput::listDevices(){
  QList<LInputDevice*> devices;
  xcb_input_list_input_devices_cookie_t cookie = xcb_input_list_input_devices_unchecked(QX11Info::connection());
  xcb_input_list_input_devices_reply_t *reply = xcb_input_list_input_devices_reply(QX11Info::connection(), cookie, NULL);
  if(reply==0){ return devices; } //error - nothing returned
  //Use the iterator for going through the reply
  //qDebug() << "Create iterator";
  xcb_input_device_info_iterator_t iter = xcb_input_list_input_devices_devices_iterator(reply);
  //xcb_str_iterator_t nameiter = xcb_input_list_input_devices_names_iterator(reply);

  //Now step through the reply
  while(iter.data != 0 ){
    devices << new LInputDevice(iter.data->device_id, iter.data->device_use);
    //qDebug() << "Found Input Device:" << iter.data->device_id;
    //qDebug() << "  - num_class_info:" << iter.data->num_class_info;
    if(iter.rem>0){ xcb_input_device_info_next(&iter); }
    else{ break; }
  }
  //Free the reply (done with it)
  ::free(reply);
  //return the information
  return devices;
}
