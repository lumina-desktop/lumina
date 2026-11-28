//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include <QDebug>
#include "LuminaOS.h"
#include "LuminaOS-common.h"
#include <QStorageInfo>
#include <unistd.h>
#include <stdio.h> // Needed for BUFSIZ

//can't read xbrightness settings - assume invalid until set
static int screenbrightness = -1;

QString LOS::OSName(){ return "Linux"; }

//OS-specific prefix(s)
// NOTE: PREFIX, L_ETCDIR, L_SHAREDIR are defined in the OS-detect.pri project file and passed in
QString LOS::LuminaShare(){ return (L_SHAREDIR+"/lumina-desktop/"); } //Install dir for Lumina share files
QString LOS::AppPrefix(){ return "/usr/"; } //Prefix for applications
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
  QStringList devs = LUtils::getCmdOutput("mount");
  //Now check the output
  for(int i=0; i<devs.length(); i++){
    if(devs[i].startsWith("/dev/")){
      devs[i] = devs[i].simplified();
      QString type = devs[i].section(" ",0,0);
      type.remove("/dev/");
      //Determine the type of hardware device based on the dev node
      if(type.startsWith("sd") || type.startsWith("nvme")){ type = "HDRIVE"; }
      else if(type.startsWith("sr")){ type="DVD"; }
      else if(type.contains("mapper")){ type="LVM"; }
      else{ type = "UNKNOWN"; }
      //Now put the device in the proper output format
      devs[i] = type+"::::"+devs[i].section(" ",4,4)+"::::"+devs[i].section(" ",2,2);
    }else{
      //invalid device - remove it from the list
      devs.removeAt(i);
      i--;
    }
  }
  return devs;
}

//Read screen brightness information
int LOS::ScreenBrightness(){
   //Returns: Screen Brightness as a percentage (0-100, with -1 for errors)
  if(screenbrightness==-1){
    if(QFile::exists(QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/.currentxbrightness")){
      int val = LUtils::readFile(QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/.currentxbrightness").join("").simplified().toInt();
      screenbrightness = val;
    }
  }
  return screenbrightness;

}

//Set screen brightness
void LOS::setScreenBrightness(int percent){
  //ensure bounds
  if(percent<0){percent=0;}
  else if(percent>100){ percent=100; }
  // float pf = percent/100.0; //convert to a decimel
  //Run the command
  QString cmd = "xbacklight -set %1";
  // cmd = cmd.arg( QString::number( int(65535*pf) ) );
  cmd = cmd.arg( QString::number( percent ) );
  int ret = LUtils::runCmd(cmd);
  //Save the result for later
  if(ret!=0){ screenbrightness = -1; }
  else{ screenbrightness = percent; }
  LUtils::writeFile(QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/.currentxbrightness", QStringList() << QString::number(screenbrightness), true);

}

//Read the current volume
int LOS::audioVolume(){ //Returns: audio volume as a percentage (0-100, with -1 for errors)
QString info = LUtils::getCmdOutput("amixer get Master").join("").simplified();;
  int out = -1;
  int start_position, end_position;
  QString current_volume;
  if(!info.isEmpty()){
     //(no "[..%]" in the output - e.g. no Master control - used to give 0 instead of the -1 error value)
     end_position = info.indexOf("%");
     start_position = (end_position<0) ? -1 : info.lastIndexOf("[", end_position);
     if(start_position>=0){
       start_position++;
       current_volume = info.mid(start_position, end_position - start_position);
       bool ok = false;
       int vol = current_volume.toInt(&ok);
       if(ok){ out = vol; }
     }
  }
  return out;


}

//Set the current volume
void LOS::setAudioVolume(int percent){
  if(percent<0){percent=0;}
  else if(percent>100){percent=100;}
  QString info = "amixer set Master " + QString::number(percent) + "%";
  //Run Command
  LUtils::runCmd(info);
}

//Change the current volume a set amount (+ or -)
void LOS::changeAudioVolume(int percentdiff){
  int old_volume = audioVolume();
  int new_volume = old_volume + percentdiff;
  if (new_volume < 0)
      new_volume = 0;
  if (new_volume > 100)
      new_volume = 100;
  qDebug() << "Setting new volume to: " << new_volume;
  setAudioVolume(new_volume);
}

//Check if a graphical audio mixer is installed
bool LOS::hasMixerUtility(){
  return QFile::exists(LOS::AppPrefix() + "bin/pavucontrol");
}

//Launch the graphical audio mixer utility
void LOS::startMixerUtility(){
  LUtils::startDetached(LOS::AppPrefix() + "bin/pavucontrol");
}

//Check for user system permission (shutdown/restart)
bool LOS::userHasShutdownAccess(){
  return true; //not implemented yet
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
  LUtils::startDetached("shutdown -P -h now");
}

//System Restart
void LOS::systemRestart(bool){ //start reboot sequence
  //INPUT: skip updates (true/false)
  LUtils::startDetached("shutdown -r now");
}

//Check for suspend support
bool LOS::systemCanSuspend(){
  return false;
}

//Put the system into the suspend state
void LOS::systemSuspend(){

}

//Battery information is read straight from the kernel (sysfs) - shared with the other Linux backends in LuminaOS-common.h
//Battery Availability
bool LOS::hasBattery(){
  return LOSLinuxBattery::hasBattery(); //(sysfs - see LuminaOS-common.h)
}

//Battery Charge Level
int LOS::batteryCharge(){ //Returns: percent charge (0-100), anything outside that range is counted as an error
  return LOSLinuxBattery::charge();
}

//Battery Charging State
// While plugged in, batteries report "Charging", "Full", "Not charging" or "Unknown";
// on battery power they report "Discharging". So anything that is NOT Discharging is
// treated as charging/plugged in (same behaviour as the previous acpi-based check).
bool LOS::batteryIsCharging(){
  return LOSLinuxBattery::isCharging();
}

//Battery Time Remaining
int LOS::batterySecondsLeft(){ //Returns: estimated number of seconds remaining
  return LOSLinuxBattery::secondsLeft(); //(-1 when unknown or charging)
}

//File Checksums
QStringList LOS::Checksums(QStringList filepaths){ //Return: checksum of the input file
  QStringList info = LUtils::getCmdOutput("md5sum \""+filepaths.join("\" \"")+"\"");
  for(int i=0; i<info.length(); i++){
    // first: md5sum: = error ; second: there's always one empty entry generated by getCmdOutput
    if( info[i].startsWith("md5sum:") || info[i].isEmpty()){ info.removeAt(i); i--; }
    else{
      //Strip out the extra information
      info[i] = info[i].section(" ",0,0);
    }
  }
 return info;
}

//file system capacity
QString LOS::FileSystemCapacity(QString dir) { //Return: percentage capacity, same value as the "Use%" column of df
  //(same value as the "Use%" column of df)
  QString pct = LOSCommon::capacityPercent(dir);
  if(pct.isEmpty()){ return QString(); }
  return pct + " used";
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
