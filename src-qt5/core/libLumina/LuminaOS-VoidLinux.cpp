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
#include <QTime>

//can't read xbrightness settings - assume invalid until set
static int screenbrightness = -1;

QString LOS::OSName(){ return "VoidLinux"; }

//OS-specific prefix(s)
// NOTE: PREFIX, L_ETCDIR, L_SHAREDIR are defined in the OS-detect.pri project file and passed in
QString LOS::LuminaShare(){ return (L_SHAREDIR+"/lumina-desktop/"); } //Install dir for Lumina share files
QString LOS::AppPrefix(){ return "/usr/"; } //Prefix for applications
QString LOS::SysPrefix(){ return "/usr/"; } //Prefix for system

//OS-specific application shortcuts (*.desktop files)
QString LOS::ControlPanelShortcut(){ return "lumina-config.desktop"; } //system control panel
QString LOS::AppStoreShortcut(){ return ""; } //graphical app/pkg manager
//OS-specific RSS feeds (Format: QStringList[ <name>::::<url> ]; )
QStringList LOS::RSSFeeds(){
  QStringList list;
  list << "Void News::::https://voidlinux.org/atom.xml";
  list << "Void Packages::::https://github.com/void-linux/void-packages/commits/master.atom";
  list << "Void XBPS::::https://github.com/void-linux/xbps/commits/master.atom";
  return list;
}

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
    if(screenbrightness == -1 && QFile::exists("/usr/bin/xbacklight")){
      screenbrightness = 100;
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
  LUtils::runCmd(cmd);
  //Save the result for later
  //qDebug() << "Got brightness change retcode:" << ret;
  screenbrightness = percent;
  LUtils::writeFile(QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/.currentxbrightness", QStringList() << QString::number(screenbrightness), true);
}

//Read the current volume
int LOS::audioVolume(){ //Returns: audio volume as a percentage (0-100, with -1 for errors)
  static int out = -1;
  if(out < 0){
    //First time session check: Load the last setting for this user
    QString info = LUtils::readFile(QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/.currentvolume").join("");
    if(!info.isEmpty()){
      out = info.simplified().toInt();
      return out;
    }
  }
  QStringList info = LUtils::getCmdOutput("pactl list sinks");
  if(info.isEmpty()){ return -1; }
  bool isRunning = false;
  int prevval = out;
  out = -1; //reset back to unknown and read it dynamically from the system
  for(int i=0; i<info.length(); i++){
    if(info[i].simplified().startsWith("State:")){
      isRunning = (info[i].section(":",-1).simplified()=="RUNNING");
    }else if(info[i].simplified().startsWith("Volume:")){
      out = info[i].section(" / ",1,1).section("%",0,0).simplified().toInt();
      if(isRunning){ break; } //currently-running volume output
    }
  }
  //qDebug() << "Current Audio Volume:" << out;
  if(prevval != out){
    //Some external tool changed the audio volume - update the cache file
    LUtils::writeFile(QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/.currentvolume", QStringList() << QString::number(out), true);
  }
  return out;
}

//Set the current volume
void LOS::setAudioVolume(int percent){
  if(percent<0){percent=0;}
  else if(percent>100){percent=100;}
  QString info = QString("pactl set-sink-volume @DEFAULT_SINK@ ")+QString::number(percent)+"%";
  //Run Command
  //qDebug() << "Set Audio Volume:" << percent;
  LUtils::runCmd(info);
  LUtils::writeFile(QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/.currentvolume", QStringList() << QString::number(percent), true);
}

//Change the current volume a set amount (+ or -)
void LOS::changeAudioVolume(int percentdiff){
  int old_volume = audioVolume();
  int new_volume = old_volume + percentdiff;
  if (new_volume < 0)
      new_volume = 0;
  if (new_volume > 100)
      new_volume = 100;
  //qDebug() << "Setting new volume to: " << new_volume;
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
  return QFile::exists(LOS::AppPrefix()+"bin/qsudo");
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
  QProcess::startDetached("qsudo poweroff");
}

//System Restart
void LOS::systemRestart(bool){ //start reboot sequence
  //INPUT: skip updates (true/false)
  QProcess::startDetached("qsudo reboot");
}

//Check for suspend support
bool LOS::systemCanSuspend(){
  bool ok = QFile::exists(LOS::AppPrefix()+"bin/qsudo");
  return ok;
}

//Put the system into the suspend state
void LOS::systemSuspend(){
  QProcess::startDetached("qsudo zzz");
}

//Battery Availability
bool LOS::hasBattery(){
  QString my_status = LUtils::getCmdOutput("acpi -b").join("");
  bool no_battery = my_status.contains("No support");
  return !no_battery;
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
  QString my_status = LUtils::getCmdOutput("acpi -b").join("\n");
  static QRegExp timefind(" [0-9]{2}:[0-9]{2}:[0-9]{2} ");
  int tmp = timefind.indexIn(my_status);
  //qDebug() << "Battery Secs:" << tmp << my_status << timefind.cap(0);
  if(tmp>=0){
    QTime time = QTime::fromString(timefind.cap(0).simplified(),"HH:mm:ss");
    //qDebug() << "Got Time:" << time;
    tmp = time.hour()*3600 + time.minute()*60 + time.second();
  }
  return tmp;
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
  QStringList mountInfo = LUtils::getCmdOutput("df \"" + dir+"\"");
  QString::SectionFlag skipEmpty = QString::SectionSkipEmpty;
  //we take the 5th word on the 2 line
  QString capacity = mountInfo[1].section(" ",4,4, skipEmpty) + " used";
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
