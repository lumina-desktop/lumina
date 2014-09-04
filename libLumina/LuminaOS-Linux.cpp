//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifdef __linux__
#include "LuminaOS.h"
#include <unistd.h>
#include <stdio.h> // Needed for BUFSIZ

// ==== ExternalDevicePaths() ====
QStringList LOS::ExternalDevicePaths(){
    //Returns: QStringList[<type>::::<filesystem>::::<path>]
      //Note: <type> = [USB, HDRIVE, DVD, SDCARD, UNKNOWN]
	
  //Not implemented yet for Linux
  return QStringList();
}

//Read screen brightness information
int LOS::ScreenBrightness(){
  //Returns: Screen Brightness as a percentage (0-100, with -1 for errors)
  return -1;  //not implemented yet for Linux
}

//Set screen brightness
void LOS::setScreenBrightness(int percent){
  //not implemented yet for Linux
}

//Read the current volume
int LOS::audioVolume(){ //Returns: audio volume as a percentage (0-100, with -1 for errors)
  return -1; //Not implemented yet for Linux
}

//Set the current volume
void LOS::setAudioVolume(int percent){
  //not implemented yet for Linux
}

//Change the current volume a set amount (+ or -)
void LOS::changeAudioVolume(int percentdiff){
  //not implemented yet for Linux	
}

//Check if a graphical audio mixer is installed
bool LOS::hasMixerUtility(){
  return false; //not implemented yet for Linux	
}

//Launch the graphical audio mixer utility
void LOS::startMixerUtility(){
  //not implemented yet for Linux	
}

//System Shutdown
void LOS::systemShutdown(){ //start poweroff sequence
  QProcess::startDetached("shutdown -h now");
}

//System Restart
void LOS::systemRestart(){ //start reboot sequence
  QProcess::startDetached("shutdown -r now");
}

//Battery Availability
bool LOS::hasBattery(){
  return false; //not implemented yet for Linux
}

//Battery Charge Level
int LOS::batteryCharge(){ //Returns: percent charge (0-100), anything outside that range is counted as an error
  return -1;  //not implemented yet for Linux
}

//Battery Charging State
bool LOS::batteryIsCharging(){
  return false; //not implemented yet for Linux
}

//Battery Time Remaining
int LOS::batterySecondsLeft(){ //Returns: estimated number of seconds remaining
  return 0; //not implemented yet for Linux
}

#endif
