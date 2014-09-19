//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifdef __OSNAME__
#include "LuminaOS.h"
#include <unistd.h>
#include <stdio.h> // Needed for BUFSIZ

//OS-specific prefix(s)
QString LOS::AppPrefix(){ return "/usr/local/"; } //Prefix for applications
QString LOS::SysPrefix(){ return "/usr/"; } //Prefix for system

//OS-specific application shortcuts (*.desktop files)
QString LOS::ControlPanelShortcut(){ return ""; } //system control panel
QString LOS::AppStoreShortcut(){ return ""; } //graphical app/pkg manager
QString LOS::QtConfigShortcut(){ return ""; } //qtconfig binary (NOT *.desktop file)
	
// ==== ExternalDevicePaths() ====
QStringList LOS::ExternalDevicePaths(){
    //Returns: QStringList[<type>::::<filesystem>::::<path>]
      //Note: <type> = [USB, HDRIVE, DVD, SDCARD, UNKNOWN]
	
  //Not implemented yet
  return QStringList();
}

//Read screen brightness information
int LOS::ScreenBrightness(){
  //Returns: Screen Brightness as a percentage (0-100, with -1 for errors)
  return -1;  //not implemented yet
}

//Set screen brightness
void LOS::setScreenBrightness(int percent){
  //not implemented yet
}

//Read the current volume
int LOS::audioVolume(){ 
  //Returns: audio volume as a percentage (0-100, with -1 for errors)
  return -1; //Not implemented yet
}

//Set the current volume
void LOS::setAudioVolume(int percent){
  //not implemented yet
}

//Change the current volume a set amount (+ or -)
void LOS::changeAudioVolume(int percentdiff){
  //not implemented yet
}

//Check if a graphical audio mixer is installed
bool LOS::hasMixerUtility(){
  return false; //not implemented yet
}

//Launch the graphical audio mixer utility
void LOS::startMixerUtility(){
  //not implemented yet
}

//Check for user system permission (shutdown/restart)
bool LOS::userHasShutdownAccess(){
  return false; //not implemented yet
}
	
//System Shutdown
void LOS::systemShutdown(){ //start poweroff sequence
  QProcess::startDetached("shutdown -p now");
}

//System Restart
void LOS::systemRestart(){ //start reboot sequence
  QProcess::startDetached("shutdown -r now");
}

//Battery Availability
bool LOS::hasBattery(){
  return false; //not implemented yet
}

//Battery Charge Level
int LOS::batteryCharge(){ //Returns: percent charge (0-100), anything outside that range is counted as an error
  return -1;  //not implemented yet
}

//Battery Charging State
bool LOS::batteryIsCharging(){
  return false; //not implemented yet
}

//Battery Time Remaining
int LOS::batterySecondsLeft(){ //Returns: estimated number of seconds remaining
  return 0; //not implemented yet
}

#endif
