//===========================================
//  Lumina desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include <OSInterface.h>

//===========
//    PUBLIC
//===========

//Simple functions used to determine if the current OS supports using this class, and what levels of support
QList<OSInterface::Interface> OSInterface::supportedNotifications(){
  //Which interfaces provide change notifications
  return QList< OSInterface::Interface >();
}

QList<OSInterface::Interface> OSInterface::supportedStatus(){
  //Which interfaces are available for "status" requests
  return QList< OSInterface::Interface >();
}

QList<OSInterface::Interface> OSInterface::supportedModify(){
   //Which interfaces are available for "modify" requests
  return QList< OSInterface::Interface >();
}

//Start/stop interface watchers/notifications (each only called once per session)
void OSInterface::start(){
 //nothing to do
}

void OSInterface::stop(){
 //nothing to do
}
	
//Generic status update
QList<QVariant> OSInterface::status(OSInterface::Interface){
	// ==== Interface status output lists ====
	//  Battery: [ float (percent charge), bool (is Charging), double (seconds remaining) ];
	//  Volume: [int (percent volume) ]
	//  Devices: [ QStringList[ name, mountpoint, type (optional)] ]  (List length depends on number of devices)
	//  Network: [bool (network access available)]
	//  PowerOff: [bool (can power off system)]
	//  Reboot: [bool (can reboot system)]
	//  Suspend: [bool (can suspend system)]
	//  Updates: [bool (is updating), bool (reboot required)]
	// ==========
  return QList<QVariant>();
}

//Individual Interface interactions
bool OSInterface::modify(OSInterface::Interface, QList<QVariant>){ //returns: success/failure
	// ==== Interface modification argument lists ====
	//  Battery: <NO MODIFICATION>
	//  Volume: [int (set percent volume) ]
	//  Devices: <NO MODIFICATION>
	//  Network: <NO MODIFICATION>
	//  PowerOff: [bool (skip updates - optional)]
	//  Reboot: [bool (skip updates - optional)]
	//  Suspend: [] (No input arguments)
	//  Updates: <NO MODIFICATION>
	// ==========
  return false;
}

//=================
//     PRIVATE SLOTS
//=================
//FileSystemWatcher slots
void OSInterface::watcherFileChanged(QString){

}

void OSInterface::watcherDirChanged(QString){

}

//IO Device slots
void OSInterface::iodeviceReadyRead(){

}

void OSInterface::iodeviceAboutToClose(){

}

//NetworkAccessManager slots
void OSInterface::netAccessChanged(QNetworkAccessManager::NetworkAccessibility){

}

void OSInterface::netRequestFinished(QNetworkReply*){

}

void OSInterface::netSslErrors(QNetworkReply*, const QList<QSslError>&){

}
