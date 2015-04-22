//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifdef __FreeBSD__
#include "LuminaOS.h"
#include <unistd.h>

#include <QDebug>
//can't read xbrightness settings - assume invalid until set
static int screenbrightness = -1;
static int audiovolume = -1;

QString LOS::OSName(){ return "FreeBSD"; }

//OS-specific prefix(s)
QString LOS::AppPrefix(){ return "/usr/local/"; } //Prefix for applications
QString LOS::SysPrefix(){ return "/usr/"; } //Prefix for system

//OS-specific application shortcuts (*.desktop files)
QString LOS::ControlPanelShortcut(){ return "/usr/local/share/applications/pccontrol.desktop"; } //system control panel
QString LOS::AppStoreShortcut(){ return "/usr/local/share/applications/softmanager.desktop"; } //graphical app/pkg manager
QString LOS::QtConfigShortcut(){ return "/usr/local/bin/qtconfig-qt4"; } //qtconfig binary (NOT *.desktop file)

// ==== ExternalDevicePaths() ====
QStringList LOS::ExternalDevicePaths(){
    //Returns: QStringList[<type>::::<filesystem>::::<path>]
      //Note: <type> = [USB, HDRIVE, DVD, SDCARD, UNKNOWN]
  QStringList devs = LUtils::getCmdOutput("mount");
  //Now check the output
  for(int i=0; i<devs.length(); i++){
    if(devs[i].startsWith("/dev/")){
      devs[i].replace("\t"," ");
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
  //Make sure we are not running in VirtualBox (does not work in a VM)
  QStringList info = LUtils::getCmdOutput("pciconf -lv");
  if( !info.filter("VirtualBox", Qt::CaseInsensitive).isEmpty() ){ return -1; }
  //Now perform the standard brightness checks
  if(screenbrightness==-1){ //memory value
    if(QFile::exists(QDir::homePath()+"/.lumina/.currentxbrightness")){ //saved file value
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
  //Run the command(s)
  bool success = false;
  // - try hardware setting first (PC-BSD only)
  if(QFile::exists("/usr/local/bin/pc-sysconfig")){
    QString ret = LUtils::getCmdOutput("pc-sysconfig", QStringList() <<"setscreenbrightness "+QString::number(percent)).join("");
    success = ret.toLower().contains("success");
    qDebug() << "Set hardware brightness:" << percent << success;
  }
  // - if hardware brightness does not work, use software brightness
  if(!success){
    QString cmd = "xbrightness  %1";
    float pf = percent/100.0; //convert to a decimel
    cmd = cmd.arg( QString::number( int(65535*pf) ) );
    success = (0 == LUtils::runCmd(cmd) );
  }
  //Save the result for later
  if(!success){ screenbrightness = -1; }
  else{ screenbrightness = percent; }
  LUtils::writeFile(QDir::homePath()+"/.lumina/.currentxbrightness", QStringList() << QString::number(screenbrightness), true);
}

//Read the current volume
int LOS::audioVolume(){ //Returns: audio volume as a percentage (0-100, with -1 for errors)
  int out = audiovolume;
  if(out < 0){
    //First time session check: Load the last setting for this user
    QString info = LUtils::readFile(QDir::homePath()+"/.lumina/.currentvolume").join("");
    if(!info.isEmpty()){ 
      out = info.simplified().toInt(); 
      audiovolume = out; //unset this internal flag
      return out; 
    }
  }
  
    //probe the system for the current volume (other utils could be changing it)
      QString info = LUtils::getCmdOutput("mixer -S vol").join(":").simplified(); //ignores any other lines
      if(!info.isEmpty()){
        int L = info.section(":",1,1).toInt();
        int R = info.section(":",2,2).toInt();
        if(L>R){ out = L; }
        else{ out = R; }
	if(out != audiovolume){
	  //Volume changed by other utility: adjust the saved value as well
	  LUtils::writeFile(QDir::homePath()+"/.lumina/.currentvolume", QStringList() << QString::number(out), true);
	}
	audiovolume = out; 
      }

  return out;
}

//Set the current volume
void LOS::setAudioVolume(int percent){
  if(percent<0){percent=0;}
  else if(percent>100){percent=100;}
  QString info = LUtils::getCmdOutput("mixer -S vol").join(":").simplified(); //ignores any other lines
  if(!info.isEmpty()){
    int L = info.section(":",1,1).toInt();
    int R = info.section(":",2,2).toInt();
    int diff = L-R;
    if((percent == L) && (L==R)){ return; } //already set to that volume
    if(diff<0){ R=percent; L=percent+diff; } //R Greater
    else{ L=percent; R=percent-diff; } //L Greater or equal
    //Check bounds
    if(L<0){L=0;}else if(L>100){L=100;}
    if(R<0){R=0;}else if(R>100){R=100;}
    //Run Command
    audiovolume = percent; //save for checking later
    LUtils::runCmd("mixer vol "+QString::number(L)+":"+QString::number(R));
    LUtils::writeFile(QDir::homePath()+"/.lumina/.currentvolume", QStringList() << QString::number(percent), true);
  }	
}

//Change the current volume a set amount (+ or -)
void LOS::changeAudioVolume(int percentdiff){
  QString info = LUtils::getCmdOutput("mixer -S vol").join(":").simplified(); //ignores any other lines
  if(!info.isEmpty()){
    int L = info.section(":",1,1).toInt() + percentdiff;
    int R = info.section(":",2,2).toInt() + percentdiff;
    //Check bounds
    if(L<0){L=0;}else if(L>100){L=100;}
    if(R<0){R=0;}else if(R>100){R=100;}
    //Run Command
    LUtils::runCmd("mixer vol "+QString::number(L)+":"+QString::number(R));
  }	
}

//Check if a graphical audio mixer is installed
bool LOS::hasMixerUtility(){
  return QFile::exists("/usr/local/bin/pc-mixer");
}

//Launch the graphical audio mixer utility
void LOS::startMixerUtility(){
  QProcess::startDetached("pc-mixer -notray");
}

//Check for user system permission (shutdown/restart)
bool LOS::userHasShutdownAccess(){
  //User needs to be a part of the operator group to be able to run the shutdown command
  QStringList groups = LUtils::getCmdOutput("id -Gn").join(" ").split(" ");
  bool ok = groups.contains("operator"); //not implemented yet
  if(ok){
    //If a PC-BSD system, also disable the shutdown/restart options if the system is in the middle of upgrading
    ok = (QProcess::execute("pgrep -F /tmp/.updateInProgress")!=0); //this is 0 if updating right now
  }
  return ok;
}

//System Shutdown
void LOS::systemShutdown(){ //start poweroff sequence
  QProcess::startDetached("shutdown -po now");
}

//System Restart
void LOS::systemRestart(){ //start reboot sequence
  QProcess::startDetached("shutdown -ro now");
}

//Check for suspend support
bool LOS::systemCanSuspend(){
  //This will only function on PC-BSD 
  //(permissions issues on standard FreeBSD unless setup a special way)
  bool ok = QFile::exists("/usr/local/bin/pc-sysconfig");
  if(ok){
    ok = LUtils::getCmdOutput("pc-sysconfig systemcansuspend").join("").toLower().contains("true");
  }
  return ok;
}

//Put the system into the suspend state
void LOS::systemSuspend(){
  QProcess::startDetached("pc-sysconfig suspendsystem");
}

//Battery Availability
bool LOS::hasBattery(){
  int val = LUtils::getCmdOutput("apm -l").join("").toInt();
  return (val >= 0 && val <= 100);
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
  return LUtils::getCmdOutput("apm -t").join("").toInt();
}

//File Checksums
QStringList LOS::Checksums(QStringList filepaths){ //Return: checksum of the input file
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
QString LOS::FileSystemCapacity(QString dir) { //Return: percentage capacity as give by the df command
  QStringList mountInfo = LUtils::getCmdOutput("df \"" + dir+"\"");
  QString::SectionFlag skipEmpty = QString::SectionSkipEmpty;
  //we take the 5th word on the 2 line
  QString capacity = mountInfo[1].section(" ",4,4, skipEmpty);
  return capacity;
}

#endif
