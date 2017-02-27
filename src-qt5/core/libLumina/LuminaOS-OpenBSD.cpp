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
  //Run the command
  QString cmd = "xbacklight -time 0 -steps 1 -set %1";
  cmd = cmd.arg( QString::number(percent) );
  int ret = LUtils::runCmd(cmd);
  //Save the result for later
  if(ret!=0){ screenbrightness = -1; }
  else{ screenbrightness = percent; }
  LUtils::writeFile(QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/.currentxbrightness", QStringList() << QString::number(screenbrightness), true);
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

//File Checksums
QStringList LOS::Checksums(QStringList filepaths){ //Return: checksum of the input file
  //on OpenBSD md5 has the following layout 
  //>md5 LuminaThemes.o LuminaUtils.o 
  //MD5 (LuminaThemes.o) = 50006505d9d7e54e5154eeb095555055
  //MD5 (LuminaUtils.o) = d490878ee8866e55e5af571b98b4d448

  QStringList info = LUtils::getCmdOutput("md5 \""+filepaths.join("\" \"")+"\"");
  for(int i=0; i<info.length(); i++){
    if( !info[i].contains(" = ") ){ info.removeAt(i); i--; }
    else{
      //Strip out the extra information
      info[i] = info[i].section(" = ",1,1);
    }
  }
 return info;
}

//file system capacity
QString LOS::FileSystemCapacity(QString dir) {
  // on OpenBSD, df has the following layout:
  //>df /home/wi
  //>Filesystem  512-blocks      Used     Avail Capacity  Mounted on
  //>/dev/sd2l     14334588    739900  12877960     5%    /home

  QStringList mountInfo = LUtils::getCmdOutput("df \"" + dir+"\"");
  QString::SectionFlag skipEmpty = QString::SectionSkipEmpty;
  //we take the 5th word on the 2 line
  QString capacity = mountInfo[1].section(" ",4,4, skipEmpty);
  return capacity;
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
