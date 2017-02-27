//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifdef __DragonFly__
#include "LuminaOS.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/sensors.h>

//can't read xbrightness settings - assume invalid until set
static int screenbrightness = -1;
static int audiovolume = -1;

static bool get_sysctlbyname_int(const char *name, int *res) {
    int r = 0;
    size_t len = sizeof(r);
    if (sysctlbyname(name, &r, &len, NULL, 0) == 0) {
        *res = r;
        return true;
    }
    return false;
}

#if 0
static bool get_sysctlbyname_qstr(const char *name, QString &str) {
    size_t len = 0;
    sysctlbyname(name, NULL, &len, NULL, 0);
    if (len > 0) {
      void *buf = malloc(len);
      if (buf) {
        int res = sysctlbyname(name, buf, &len, NULL, 0);
        if (res == 0) {
          str = QString((char*) buf);
        }
        free(buf);
        return (res == 0);
      }
    }
    return false;
}
#endif

// returns -1 on error.
static int get_sysctlbyname_int(const char *name) {
  int res = -1;
  if (get_sysctlbyname_int(name, &res)) {
    return res;
  }
  return -1;
}

static bool get_sysctlbyname_uint(const char *name, unsigned int *res) {
    unsigned int r = 0;
    size_t len = sizeof(r);
    if (sysctlbyname(name, &r, &len, NULL, 0) == 0) {
        *res = r;
        return true;
    }
    return false;
}

QString LOS::OSName(){ return "DragonFly BSD"; }

//OS-specific prefix(s)
// NOTE: PREFIX, L_ETCDIR, L_SHAREDIR are defined in the OS-detect.pri project file and passed in
QString LOS::LuminaShare(){ return (L_SHAREDIR+"/lumina-desktop/"); } //Install dir for Lumina share files
QString LOS::AppPrefix(){ return "/usr/local/"; } //Prefix for applications
QString LOS::SysPrefix(){ return "/usr/"; } //Prefix for system

//OS-specific application shortcuts (*.desktop files)
QString LOS::ControlPanelShortcut(){ return ""; } //system control panel
QString LOS::AppStoreShortcut(){ return ""; } //graphical app/pkg manager
//OS-specific RSS feeds (Format: QStringList[ <name>::::<url> ]; )
QStringList LOS::RSSFeeds(){ 
  QStringList feeds;
  feeds << "DragonFly BSD Feed::::http://www.dragonflybsd.org/recentchanges/index.rss";
  return feeds; 
} 

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
    if(QFile::exists(QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/.currentxbrightness")){
      int val = LUtils::readFile(QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/.currentxbrightness").join("").simplified().toInt();
      screenbrightness = val;
    }
  }
  //If it gets to this point, then we have a valid (but new) installation
  if(screenbrightness<0){ screenbrightness = 100; } //default value for systems

  return screenbrightness;	
}

//Set screen brightness
void LOS::setScreenBrightness(int percent){
  if(percent == -1){ return; } //This is usually an invalid value passed directly to the setter
  //ensure bounds
  if(percent<0){percent=0;}
  else if(percent>100){ percent=100; }
  //Run the command(s)
  bool success = false;
  float pf = percent/100.0; //convert to a decimel
  //Run the command
  QString cmd = "xbrightness  %1";
  cmd = cmd.arg( QString::number( int(65535*pf) ) );
  success = (0 == LUtils::runCmd(cmd) );
  //Save the result for later
  if(!success){ screenbrightness = -1; }
  else{ screenbrightness = percent; }
  LUtils::writeFile(QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/.currentxbrightness", QStringList() << QString::number(screenbrightness), true);
}

//Read the current volume
int LOS::audioVolume(){ //Returns: audio volume as a percentage (0-100, with -1 for errors)
  int out = audiovolume;
  if(out < 0){
    //First time session check: Load the last setting for this user
    QString info = LUtils::readFile(QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/.currentvolume").join("");
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
      LUtils::writeFile(QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/.currentvolume", QStringList() << QString::number(out), true);
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
    LUtils::writeFile(QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/.currentvolume", QStringList() << QString::number(percent), true);
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
  return false; //not implemented yet for DragonFly
}

//Launch the graphical audio mixer utility
void LOS::startMixerUtility(){
  //Not implemented yet for DragonFly
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
  return (get_sysctlbyname_int("hw.acpi.battery.units") >= 1);
}

//Battery Charge Level
int LOS::batteryCharge(){ //Returns: percent charge (0-100), anything outside that range is counted as an error
  int charge = get_sysctlbyname_int("hw.acpi.battery.life");
  if(charge > 100){ charge = -1; } //invalid charge 
  return charge;	
}

//Battery Charging State
bool LOS::batteryIsCharging(){
  return (get_sysctlbyname_int("hw.acpi.battery.state") == 0);
}

//Battery Time Remaining
int LOS::batterySecondsLeft(){ //Returns: estimated number of seconds remaining
  int time = get_sysctlbyname_int("hw.acpi.battery.time");
  if (time > 0) {
    // time is in minutes
    time *= 60;
  }
  return time;
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

static float sensor_value_to_degC(int64_t value) {
    return (value - 273150000) / 1000000.0;
}

//Returns: List containing the temperature of any CPU's ("50C" for example)
QStringList LOS::CPUTemperatures(){
  QStringList temps;

  int mib[5];
  mib[0] = CTL_HW;
  mib[1] = HW_SENSORS;

  for (int dev=0; dev < MAXSENSORDEVICES; ++dev) {
      struct sensordev sensordev;
      size_t sdlen = sizeof(sensordev);

      mib[2] = dev;
      if (sysctl(mib, 3, &sensordev, &sdlen, NULL, 0) == -1) {
        continue;
      }
      mib[3] = SENSOR_TEMP;
      for (int numt=0; numt < sensordev.maxnumt[SENSOR_TEMP]; ++numt) {
          mib[4] = numt;
          struct sensor sensor;
          size_t slen = sizeof(sensor);
          if (sysctl(mib, 5, &sensor, &slen, NULL, 0) == -1) {
            continue;
          }

          // XXX: Filter out non-cpu temperatures

          int degC = (int)sensor_value_to_degC(sensor.value);
          temps << QString::number(degC) + "C" + "(" + QString(sensordev.xname) + ")";
      }
  }

  return temps;
}

int LOS::CPUUsagePercent(){ //Returns: Overall percentage of the amount of CPU cycles in use (-1 for errors)
  return -1; //not implemented yet
}

int LOS::MemoryUsagePercent(){
  //SYSCTL: vm.stats.vm.v_<something>_count
  unsigned int v_page_count = 0;
  unsigned int v_wire_count = 0;
  unsigned int v_active_count = 0;

  if (!get_sysctlbyname_uint("vm.stats.vm.v_page_count", &v_page_count)) return -1;
  if (!get_sysctlbyname_uint("vm.stats.vm.v_wire_count", &v_wire_count)) return -1;
  if (!get_sysctlbyname_uint("vm.stats.vm.v_active_count", &v_active_count)) return -1;

  //List output: [total, wired, active]
  double perc = 100.0 * ((long)v_wire_count+(long)v_active_count)/((double)v_page_count);
  return qRound(perc);
}

QStringList LOS::DiskUsage(){ //Returns: List of current read/write stats for each device
  return QStringList(); //not implemented yet
}
#endif
