//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Copyright (c) 2014, Antoine Jacoutot <ajacoutot@openbsd.org>
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifdef __OpenBSD__
#include "LuminaOS.h"
#include <unistd.h>

//can't read xbrightness settings - assume invalid until set
static int screenbrightness = -1;

QString LOS::OSName(){ return "OpenBSD"; }

//OS-specific prefix(s)
QString LOS::AppPrefix(){ return "/usr/local/"; } //Prefix for applications
QString LOS::SysPrefix(){ return "/usr/"; } //Prefix for system

//OS-specific application shortcuts (*.desktop files)
QString LOS::ControlPanelShortcut(){ return ""; } //system control panel
QString LOS::AppStoreShortcut(){ return ""; } //graphical app/pkg manager
QString LOS::QtConfigShortcut(){ return "/usr/local/bin/qtconfig4"; } //qtconfig binary (NOT *.desktop file)

// ==== ExternalDevicePaths() ====
QStringList LOS::ExternalDevicePaths(){
    //Returns: QStringList[<type>::::<filesystem>::::<path>]
      //Note: <type> = [USB, HDRIVE, DVD, SDCARD, UNKNOWN]
  QStringList devs = LUtils::getCmdOutput("mount");
  //Now check the output
  for(int i=0; i<devs.length(); i++){
      QString type = devs[i].section(" ",0,0);
      type.remove("/dev/");
      //Determine the type of hardware device based on the dev node
      if(type.startsWith("sd")||type.startsWith("wd")){ type = "HDRIVE"; }
      else if(type.startsWith("cd")){ type="DVD"; }
      else{ type = "UNKNOWN"; }
      //Now put the device in the proper output format
      QString fs = devs[i].section(" ", 4, 4);
      QString path = devs[i].section(" ",2, 2);
      if (!fs.isEmpty() ) {   //we not found a filesystem, most probably this is an invalid row
          devs[i] = type+"::::"+fs+"::::"+path;
      }
      else {
          devs.removeAt(i);
          i--; 
      }
  }
  return devs;
}

//Read screen brightness information
int LOS::ScreenBrightness(){
  //Returns: Screen Brightness as a percentage (0-100, with -1 for errors)
  //Make sure we are not running in a VM (does not work)
  QStringList info = LUtils::getCmdOutput("sysctl -n hw.product");
  if( !info.filter(QRegExp("VirtualBox|KVM")).isEmpty() ){ return -1; }
  //Now perform the standard brightness checks
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
  //Run the command
  QString cmd = "xbacklight -time 0 -steps 1 -set %1";
  cmd = cmd.arg( QString::number(percent) );
  int ret = LUtils::runCmd(cmd);
  //Save the result for later
  if(ret!=0){ screenbrightness = -1; }
  else{ screenbrightness = percent; }
  LUtils::writeFile(QDir::homePath()+"/.lumina/.currentxbrightness", QStringList() << QString::number(screenbrightness), true);
}

//Read the current volume
int LOS::audioVolume(){ //Returns: audio volume as a percentage (0-100, with -1 for errors)
  QString info = LUtils::getCmdOutput("mixerctl -n outputs.master").join(",").simplified(); //ignores any other lines
  int out = -1;
  if(!info.isEmpty()){
    int L = info.section(",",0,0).toInt();
    int R = info.section(",",1,1).toInt();
    L = (L*100)/255; //percent
    R = (R*100)/255; //percent
    if(L>R){ out = L; }
    else{ out = R; }
  }
  return out;
}

//Set the current volume
void LOS::setAudioVolume(int percent){
  if(percent<0){percent=0;}
  else if(percent>100){percent=100;}
  QString info = LUtils::getCmdOutput("mixerctl -n outputs.master").join(",").simplified(); //ignores any other lines
  if(!info.isEmpty()){
    int L = info.section(",",0,0).toInt();
    int R = info.section(",",1,1).toInt();
    L = (L*100)/255; //percent
    R = (R*100)/255; //percent
    int diff = L-R;
    if(diff<0){ R=percent; L=percent+diff; } //R Greater
    else{ L=percent; R=percent-diff; } //L Greater or equal
    //Check bounds
    if(L<0){L=0;}else if(L>100){L=100;}
    if(R<0){R=0;}else if(R>100){R=100;}
    //Run Command
    L = (L*255)/100; //0-255
    R = (R*255)/100; //0-255
    LUtils::runCmd("mixerctl -q outputs.master="+QString::number(L)+","+QString::number(R));
  }    
}

//Change the current volume a set amount (+ or -)
void LOS::changeAudioVolume(int percentdiff){
  QString info = LUtils::getCmdOutput("mixerctl -n outputs.master").join(",").simplified(); //ignores any other lines
  if(!info.isEmpty()){
    int L = info.section(",",0,0).toInt();
    int R = info.section(",",1,1).toInt();
    L = (L*100)/255; //percent
    R = (R*100)/255; //percent
    L = L + percentdiff;
    R = R + percentdiff;
    //Check bounds
    if(L<0){L=0;}else if(L>100){L=100;}
    if(R<0){R=0;}else if(R>100){R=100;}
    //Run Command
    L = (L*255)/100; //0-255
    R = (R*255)/100; //0-255
    LUtils::runCmd("mixerctl -q outputs.master="+QString::number(L)+","+QString::number(R));
  }
}

//Check if a graphical audio mixer is installed
bool LOS::hasMixerUtility(){
  return false; //not implemented yet for OpenBSD
}

//Launch the graphical audio mixer utility
void LOS::startMixerUtility(){
  //Not implemented yet for OpenBSD
}

//Check for user system permission (shutdown/restart)
bool LOS::userHasShutdownAccess(){
  //User needs to be a part of the operator group to be able to run the shutdown command
  QStringList groups = LUtils::getCmdOutput("id -Gn").join(" ").split(" ");
  return groups.contains("operator");
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
  int val = LUtils::getCmdOutput("apm -b").join("").toInt();
  return (val < 4);
}

//Battery Charge Level
int LOS::batteryCharge(){ //Returns: percent charge (0-100), anything outside that range is counted as an error
  int charge = LUtils::getCmdOutput("apm -l").join("").toInt();
  if(charge > 100){ charge = -1; } //invalid charge 
  return charge;	
}

//Battery Charging State
bool LOS::batteryIsCharging(){
  return (LUtils::getCmdOutput("apm -a").join("").simplified() == "1");
}

//Battery Time Remaining
int LOS::batterySecondsLeft(){ //Returns: estimated number of seconds remaining
  int min = LUtils::getCmdOutput("apm -m").join("").toInt();
  return (min * 60);
}

#endif
