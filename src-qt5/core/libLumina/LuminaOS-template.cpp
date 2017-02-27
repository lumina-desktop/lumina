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

QString LOS::OSName(){ return "Unknown"; }

//OS-specific prefix(s)
// NOTE: PREFIX, L_ETCDIR, L_SHAREDIR are defined in the OS-detect.pri project file and passed in
QString LOS::LuminaShare(){ return (L_SHAREDIR+"/lumina-desktop/"); } //Install dir for Lumina share files
QString LOS::AppPrefix(){ return "/usr/local/"; } //Prefix for applications
QString LOS::SysPrefix(){ return "/usr/"; } //Prefix for system

//OS-specific application shortcuts (*.desktop files)
QString LOS::ControlPanelShortcut(){ return ""; } //system control panel
QString LOS::AppStoreShortcut(){ return ""; } //graphical app/pkg manager
//OS-specific RSS feeds (Format: QStringList[ <name>::::<url> ]; )
QStringList LOS::RSSFeeds(){ return QStringList(); } 

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
void LOS::setScreenBrightness(int){ //percent: 0-100
  //not implemented yet
}

//Read the current volume
int LOS::audioVolume(){ 
  //Returns: audio volume as a percentage (0-100, with -1 for errors)
  return -1; //Not implemented yet
}

//Set the current volume
void LOS::setAudioVolume(int){ //percent: 0-100
  //not implemented yet
}

//Change the current volume a set amount (+ or -)
void LOS::changeAudioVolume(int){ //percent difference from current (+ or -)
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

//Check for whether the system is safe to power off (no updates being perfomed)
bool LOS::systemPerformingUpdates(){
  return false; //Not implemented yet
}
//Return the details of any updates which are waiting to apply on shutdown
QString LOS::systemPendingUpdates(){
  return "";
}

//System Shutdown
void LOS::systemShutdown(bool){ //start poweroff sequence
  //INPUT: skip updates (true/false)
  QProcess::startDetached("shutdown -p now");
}

//System Restart
void LOS::systemRestart(bool){ //start reboot sequence
  //INPUT: skip updates (true/false)
  QProcess::startDetached("shutdown -r now");
}

//Check for suspend support
bool LOS::systemCanSuspend(){
  return false;
}

//Put the system into the suspend state
void LOS::systemSuspend(){

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

//File Checksums
QStringList LOS::Checksums(QStringList){ //QStringList filepaths
  //Return: checksum of the input file
  return QStringList();
}

//file system capacity
QString LOS::FileSystemCapacity(QString) { //QString directory path
  //Return: percentage capacity as give by the df command
  return QString();
}

QStringList LOS::CPUTemperatures(){ //Returns: List containing the temperature of any CPU's ("50C" for example)
  return QStringList(); //not implemented yet
}

int LOS::CPUUsagePercent(){ //Returns: Overall percentage of the amount of CPU cycles in use (-1 for errors)
  return -1; //not implemented yet
}

int LOS::MemoryUsagePercent(){
  return -1; //not implemented yet
}

QStringList LOS::DiskUsage(){ //Returns: List of current read/write stats for each device
  return QStringList(); //not implemented yet
}
#endif
