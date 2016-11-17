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

// LInputDevice Class
LInputDevice::LInputDevice(unsigned int id, unsigned int type){
  devID = id;
  devType = type;
  //devName = name;
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

QStringList LInputDevice::listProperties(){
  xcb_input_list_device_properties_cookie_t cookie = xcb_input_list_device_properties_unchecked(QX11Info::connection(), devID);
  xcb_input_list_device_properties_reply_t *reply = xcb_input_list_device_properties_reply(QX11Info::connection(), cookie, NULL);
  qDebug() << "Property List:";
  qDebug() << " - response_type:" << reply->response_type;
  qDebug() << " - num atoms:" << reply->num_atoms;
  qDebug() << " - length:" << reply->length;
  qDebug() << " - sequence:" << reply->sequence;
  //Done with data structure
  ::free(reply);
  //Return info
  return QStringList();
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
