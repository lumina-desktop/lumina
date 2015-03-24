//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifdef __FreeBSD_kernel__
#ifndef __FreeBSD__
// The above two checks should make sure that we are on a
// operating system using the FreeBSD kernel without actually being
// on FreeBSD. That probably means Debian's kFreeBSD port.
#include <QDebug>
#include "LuminaOS.h"
#include <unistd.h>
#include <stdio.h> // Needed for BUFSIZ

//can't read xbrightness settings - assume invalid until set
static int screenbrightness = -1;

QString LOS::OSName(){ return "Debian GNU/kFreeBSD"; }

//OS-specific prefix(s)
QString LOS::AppPrefix(){ return "/usr/"; } //Prefix for applications
QString LOS::SysPrefix(){ return "/usr/"; } //Prefix for system

//OS-specific application shortcuts (*.desktop files)
QString LOS::ControlPanelShortcut(){ return ""; } //system control panel
QString LOS::AppStoreShortcut(){ return ""; } //graphical app/pkg manager
QString LOS::QtConfigShortcut(){ return "/usr/bin/qtconfig-qt4"; } //qtconfig binary (NOT *.desktop file)

// ==== ExternalDevicePaths() ====
QStringList LOS::ExternalDevicePaths(){
    //Returns: QStringList[<type>::::<filesystem>::::<path>]
      //Note: <type> = [USB, HDRIVE, DVD, SDCARD, UNKNOWN]
   QStringList devs = LUtils::getCmdOutput("mount");
  //Now check the output
  for(int i=0; i<devs.length(); i++){
    if(devs[i].startsWith("/dev/")){
      QString type = devs[i].section(" on ",0,0);
        type.remove("/dev/");
      //Determine the type of hardware device based on the dev node
      if(type.startsWith("da")){ type = "USB"; }
      else if(type.startsWith("ada")){ type = "HDRIVE"; }
      else if(type.startsWith("mmsd")){ type = "SDCARD"; }
      else if(type.startsWith("cd")||type.startsWith("acd")){ type="DVD"; }
      else{ type = "UNKNOWN"; }
      //Now put the device in the proper output format
      devs[i] = type+"::::"+devs[i].section("(",1,1).section(",",0,0)+"::::"+devs[i].section(" on ",1,50).section("(",0,0).simplified();
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
    if(QFile::exists(QDir::homePath()+"/.lumina/.currentxbrightness")){
      int val = LUtils::readFile(QDir::homePath()+"/.lumina/.currentxbrightness").join("").simplified().toInt();
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
  LUtils::writeFile(QDir::homePath()+"/.lumina/.currentxbrightness", QStringList() << QString::number(screenbrightness), true);

}

//Read the current volume
int LOS::audioVolume(){ //Returns: audio volume as a percentage (0-100, with -1 for errors)
   return -1;    // not available on kFreeBSD yet
}

//Set the current volume
void LOS::setAudioVolume(int percent){
   return;
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
  return QFile::exists("/usr/bin/pavucontrol");
}

//Launch the graphical audio mixer utility
void LOS::startMixerUtility(){
  QProcess::startDetached("/usr/bin/pavucontrol");
}

//Check for user system permission (shutdown/restart)
bool LOS::userHasShutdownAccess(){
  return true; //not implemented yet
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
  return false;
}

//Battery Charge Level
int LOS::batteryCharge(){ //Returns: percent charge (0-100), anything outside that range is counted as an error
  return -1;
}

//Battery Charging State
bool LOS::batteryIsCharging(){
  return false;
}

//Battery Time Remaining
int LOS::batterySecondsLeft(){ //Returns: estimated number of seconds remaining
  return 0; //not implemented yet for Linux
}

//File Checksums
QStringList LOS::Checksums(QStringList filepaths){ //Return: checksum of the input file
 return QStringList();
}

#endif
#endif
