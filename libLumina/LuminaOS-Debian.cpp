//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifdef __linux__
#include <QDebug>
#include "LuminaOS.h"
#include <unistd.h>
#include <stdio.h> // Needed for BUFSIZ

//can't read xbrightness settings - assume invalid until set
static int screenbrightness = -1;

QString LOS::OSName(){ return "Debian GNU/Linux"; }

//OS-specific prefix(s)
QString LOS::AppPrefix(){ return "/usr/"; } //Prefix for applications
QString LOS::SysPrefix(){ return "/"; } //Prefix for system

//OS-specific application shortcuts (*.desktop files)
QString LOS::ControlPanelShortcut(){ return ""; } //system control panel
QString LOS::AppStoreShortcut(){ return LOS::AppPrefix() + "/share/applications/synaptic.desktop"; } //graphical app/pkg manager
QString LOS::QtConfigShortcut(){ return LOS::AppPrefix() + "/bin/qt5ct"; } //qtconfig binary (NOT *.desktop file)

// ==== ExternalDevicePaths() ====
QStringList LOS::ExternalDevicePaths(){
  /* Returns: QStringList[<type>::::<filesystem>::::<path>]
     Note: <type> = [USB, HDRIVE, DVD, SDCARD, UNKNOWN]
     df is much better for this than mount, because it skips
     all non-physical devices (like bind-mounts from schroot)
     so they are not listed in lumina-fm */
  QStringList devs = LUtils::getCmdOutput("df --output=source,fstype,target");
  //Now check the output
  for(int i=0; i<devs.length(); i++){
    if(devs[i].startsWith("/dev/")){
      devs[i] = devs[i].simplified();
      QString type = devs[i].section(" on ",0,0);
      type.remove("/dev/");
      //Determine the type of hardware device based on the dev node
      if(type.startsWith("sd")){ type = "HDRIVE"; }
      else if(type.startsWith("sr")){ type="DVD"; }
      else if(type.contains("mapper")){ type="LVM"; }
      else{ type = "UNKNOWN"; }
      //Now put the device in the proper output format
      devs[i] = type + "::::" + devs[i].section(" ",1,1) + "::::" + devs[i].section(" ",2,2);
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
QString info = LUtils::getCmdOutput("amixer get Master").join("").simplified();;
  int out = -1;
  int start_position, end_position;
  QString current_volume;
  if(!info.isEmpty()){
     start_position = info.indexOf("[");
     start_position++;
     end_position = info.indexOf("%");
     current_volume = info.mid(start_position, end_position - start_position);
     out = current_volume.toInt();
  }
  return out;

}

//Set the current volume
void LOS::setAudioVolume(int percent){
  if(percent<0){percent=0;}
  else if(percent>100){percent=100;}
  QString info = "amixer -c 0 sset Master,0 " + QString::number(percent) + "%";
  if(!info.isEmpty()){
    //Run Command
    LUtils::runCmd(info);
  }

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
  QProcess::startDetached(LOS::AppPrefix() + "bin/pavucontrol");
}

//Check for user system permission (shutdown/restart)
bool LOS::userHasShutdownAccess(){
  QProcess::startDetached("dbus-send --system --print-reply=literal \
  --type=method_call --dest=org.freedesktop.login1 \
  /org/freedesktop/login1 org.freedesktop.login1.Manager.CanPowerOff");
}

//System Shutdown
void LOS::systemShutdown(){ //start poweroff sequence
  QProcess::startDetached("dbus-send --system --print-reply \
  --dest=org.freedesktop.login1 /org/freedesktop/login1 \
  org.freedesktop.login1.Manager.PowerOff boolean:true");
}

//System Restart
void LOS::systemRestart(){ //start reboot sequence
  QProcess::startDetached("dbus-send --system --print-reply \
  --dest=org.freedesktop.login1 /org/freedesktop/login1 \
  org.freedesktop.login1.Manager.Reboot boolean:true");
}

//Check for suspend support
bool LOS::systemCanSuspend(){
  QProcess::startDetached("dbus-send --system --print-reply=literal \
  --type=method_call --dest=org.freedesktop.login1 \
  /org/freedesktop/login1 org.freedesktop.login1.Manager.CanSuspend");
}

//Put the system into the suspend state
void LOS::systemSuspend(){
  QProcess::startDetached("dbus-send --system --print-reply \
  --dest=org.freedesktop.login1 /org/freedesktop/login1 \
  org.freedesktop.login1.Manager.Suspend boolean:true");
}

//Battery Availability
bool LOS::hasBattery(){
  QString my_status = LUtils::getCmdOutput("acpi -b").join("");
  bool no_battery = my_status.contains("No support");
  if (no_battery) return false;
  return true;
}

//Battery Charge Level
int LOS::batteryCharge(){ //Returns: percent charge (0-100), anything outside that range is counted as an error
  QString my_status = LUtils::getCmdOutput("acpi -b").join("");
  int my_start = my_status.indexOf("%");
  // get the number right before the % sign
  int my_end = my_start;
  my_start--;
  while ( (my_status[my_start] != ' ') && (my_start > 0) )
      my_start--;
  my_start++;
  int my_charge = my_status.mid(my_start, my_end - my_start).toInt();
  if ( (my_charge < 0) || (my_charge > 100) ) return -1;
  return my_charge;
}

//Battery Charging State
// Many possible values are returned if the laptop is plugged in
// these include "Unknown, Full and No support.
// However, it seems just one status is returned when running
// on battery and that is "Discharging". So if the value we get
// is NOT Discharging then we assume the battery is charging.
bool LOS::batteryIsCharging(){
  QString my_status = LUtils::getCmdOutput("acpi -b").join("");
  bool discharging = my_status.contains("Discharging");
  if (discharging) return false;
  return true;
}

//Battery Time Remaining
int LOS::batterySecondsLeft(){ //Returns: estimated number of seconds remaining
  return 0; //not implemented yet for Linux
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
QString LOS::FileSystemCapacity(QString dir) { //Return: percentage capacity as give by the df command
  QStringList mountInfo = LUtils::getCmdOutput("df -h \"" + dir + "\"");
  QString::SectionFlag skipEmpty = QString::SectionSkipEmpty;
  //output: 200G of 400G available on /mount/point
  QString capacity = mountInfo[1].section(" ",3,3, skipEmpty) + " of " + mountInfo[1].section(" ",1,1, skipEmpty) + " available on " +  mountInfo[1].section(" ",5,5, skipEmpty);
  return capacity;
}

#endif
