//===========================================
//  Lumina desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// FreeBSD/TrueOS specific OS Interactions
//===========================================
// USEFUL INTERNAL FUNCTIONS:
//----------------------------------------------
//	bool verifyAppOrBin(QString chk)
//===========================================
#include <framework-OSInterface.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/unistd.h>

// = Battery =
bool OSInterface::OS_batteryAvailable(){
  static int bat_avail = -1; //this will not change during a single session - keep later calls fast
  if(bat_avail < 0){
    int val = getCmdOutput("apm -l").join("").toInt();
    bat_avail = ((val >= 0 && val <= 100) ? 1 : 0 );
  }
  return (bat_avail==1);
}

float OSInterface::OS_batteryCharge(){
  int charge = getCmdOutput("apm -l").join("").toInt();
  if(charge > 100){ charge = -1; } //invalid charge
  return charge;
}

bool OSInterface::OS_batteryCharging(){
  return (getCmdOutput("apm -a").join("").simplified() == "1");
}

double OSInterface::OS_batterySecondsLeft(){ //Returns: estimated number of seconds remaining
  return getCmdOutput("apm -t").join("").toDouble();
}

// = Volume =
bool OSInterface::OS_volumeSupported(){ return true; }
int OSInterface::OS_volume(){
  int out = -1;
  /*bool remoteSession = !QString(getenv("PICO_CLIENT_LOGIN")).isEmpty();
  if(remoteSession){
     QStringList info = getCmdOutput("pactl list short sinks");
     qDebug() << "Got PA sinks:" << info;
     out = 50; //TEMPORARY - still need to write up the info parsing
  }else{*/
    //probe the system for the current volume (other utils could be changing it)
      QString info = getCmdOutput("mixer -S vol").join(":").simplified(); //ignores any other lines
      if(!info.isEmpty()){
        int L = info.section(":",1,1).toInt();
        int R = info.section(":",2,2).toInt();
        if(L>R){ out = L; }
        else{ out = R; }
      }
  //} //end of Remote Session check
  return out;
}

bool OSInterface::OS_setVolume(int percent){
  if(percent<0){percent=0;}
  else if(percent>100){percent=100;}
  /*bool remoteSession = !QString(getenv("PICO_CLIENT_LOGIN")).isEmpty();
  if(remoteSession){
    runCmd(QString("pactl set-sink-volume @DEFAULT_SINK@ ")+QString::number(percent)+"%");
  }else{*/
    QString info = getCmdOutput("mixer -S vol").join(":").simplified(); //ignores any other lines
    if(!info.isEmpty()){
      int L = info.section(":",1,1).toInt();
      int R = info.section(":",2,2).toInt();
      int diff = L-R;
      if((percent == L) && (L==R)){ return false; } //already set to that volume
      if(diff<0){ R=percent; L=percent+diff; } //R Greater
      else{ L=percent; R=percent-diff; } //L Greater or equal
      //Check bounds
      if(L<0){L=0;}else if(L>100){L=100;}
      if(R<0){R=0;}else if(R>100){R=100;}
      //Run Command
      return (0==runCmd("mixer vol "+QString::number(L)+":"+QString::number(R)) );
    }
  //} //end of Remote Session check
  return false;
}

// = Network Information =
QString OSInterface::OS_networkTypeFromDeviceName(QString name){
  //Return options: wifi, wired, cell, cell-2G, cell-3G, cell-4G
  QString type = "wired";
  if(name.startsWith("wlan")){ type = "wifi"; }
  //Not sure about cell connections . Probably still treated as wifi devices (wlan*)
  return type;
}

float OSInterface::OS_networkStrengthFromDeviceName(QString name){
  //NOTE: This will only run for non-wired devices (wifi, cell[-*])
  // Step 1 : Figure out which access point is currently connected
  QStringList info = getCmdOutput("ifconfig", QStringList() << name).filter("bssid");
  if(info.isEmpty()){ return -1; }
  QString bssid = info.first().section("bssid ",1,-1).section(" ",0,0);
  // Step 2: Scan access point to get signal/noise
  info = getCmdOutput("ifconfig", QStringList() << name << "list" << "scan").filter(bssid);
  if(info.isEmpty()){ return -1; }
  QString signoise =info.first().section(" ", 4,4, QString::SectionSkipEmpty).simplified();
  int sig = signoise.section(":",0,0).toInt();
  int noise = signoise.section(":",1,1).toInt();
  // Step 3: Turn signal/noise ratio into a percentage
  int perc = qAbs(sig - noise) * 4;
  return perc; //percentage
}

// = Media Shortcuts =
QStringList OSInterface::OS_mediaDirectories(){ return QStringList() << "/media"; } //directory where XDG shortcuts are placed for interacting with media (local/remote)

// = Updates =
bool OSInterface::OS_updatesSupported(){ return verifyAppOrBin("pc-updatemanager"); }

bool OSInterface::OS_updatesAvailable(){ return QFile::exists("/tmp/.trueos-update-staged"); }
QString OSInterface::OS_updateDetails(){ return readFile("/tmp/.trueos-update-staged"); }	//Information about any available updates

bool OSInterface::OS_updatesRunning(){ return (runCmd("pgrep -F /tmp/.updateInProgress")==0); }
QString OSInterface::OS_updateLog(){ return QString(); }		//Information about any currently-running update

//Note: Because the second-stage updates on TrueOS actually happen on reboot, we never see a "finished" update
bool OSInterface::OS_updatesFinished(){ return false; }
QString OSInterface::OS_updateResults(){ return QString(); }	//Information about any finished update

void OSInterface::OS_startUpdates(){ runCmd("pc-updatemanager", QStringList() << "startupdate"); } //start stage 2 on reboot
bool OSInterface::OS_updateOnlyOnReboot(){ return true; } //Should the startUpdates function be called only when rebooting the system?
bool OSInterface::OS_updateCausesReboot(){ return true; }

QDateTime OSInterface::OS_lastUpdate(){ return QDateTime(); }	//The date/time of the previous updates
QString OSInterface::OS_lastUpdateResults(){ return QString(); } //Information about the previously-finished update

// = System Power =
bool OSInterface::OS_canReboot(){
  int ret = eaccess("/sbin/shutdown", X_OK);
  return (ret==0);
}
void OSInterface::OS_startReboot(){ runCmd("/sbin/shutdown", QStringList() << "-ro" << "now"); }

bool OSInterface::OS_canShutdown(){
  int ret = eaccess("/sbin/shutdown", X_OK);
  return (ret==0);
}
void OSInterface::OS_startShutdown(){ runCmd("/sbin/shutdown", QStringList() << "-po" << "now"); }

bool OSInterface::OS_canSuspend(){
  int ret = eaccess("/usr/sbin/acpiconf", X_OK);
  return (ret==0);
}
void OSInterface::OS_startSuspend(){ runCmd("zzz"); } //zzz runs "acpiconf -s <suspend state>"

// = Screen Brightness =
bool OSInterface::OS_brightnessSupported(){
//First run a quick check to ensure this is not a VirtualBox VM (no brightness control)
  static int goodsys = -1; //This will not change over time - only check/set once
  if(goodsys<0){
      //Make sure we are not running in VirtualBox (does not work in a VM)
      QStringList info = getCmdOutput("pciconf -lv");
      if( info.filter("VirtualBox", Qt::CaseInsensitive).isEmpty() ){ goodsys = 1; }
      else{ goodsys = 0; } //not a good system
  }
  if(goodsys!=1){ return false; } //go ahead and stop here - not a good system
  QStringList tools; tools << "intel_backlight" << "xbrightness";
  bool ok = false;
  for(int i=0; i<tools.length() && !ok; i++){ ok = verifyAppOrBin(tools[i]); }
  return ok;
}

int OSInterface::OS_brightness(){
  //return percentage: 0-100 with -1 for errors
  QStringList tools; tools << "intel_backlight" << "xbrightness";
  //NOTE: xbacklight does not have a way to return the current brightness
  int num = -1;
  for(int i=0; i<tools.length() && num<0; i++){
    if(!verifyAppOrBin(tools[i])){ continue; }
    switch(i){
	case 0: //intel_backlight
	  num = getCmdOutput("intel_backlight").join("").section("%",0,0).section(":",1,1).simplified().toInt();
	  break;
	default:
	  num = -1;
    }
  }
  if(num>100){ num=100; } //quick verification of upper limit
  else if(num<-1){ num = -1; } //something really messed up - return the error code
  return num;
}

bool  OSInterface::OS_setBrightness(int percent){
  QStringList tools; tools << "intel_backlight" << "xbrightness";
  for(int i=0; i<tools.length(); i++){
    if(!verifyAppOrBin(tools[i])){ continue; }
    QStringList args;
    switch(i){
      case 0: //intel_backlight
	args << QString::number(percent);
	break;
      case 1: //xbrightness
	args << QString::number( qRound( (percent/100.0)*65535) ); //xbrightness has a scale of 0-65535
	break;
    }
    return (0 == runCmd(tools[i], args) );
  }
  return false;
}

// = System Status Monitoring
bool OSInterface::OS_cpuSupported(){ return false; }
QList<int> OSInterface::OS_cpuPercentage(){ return QList<int>(); } // (one per CPU) percentage: 0-100 with empty list for errors
QStringList OSInterface::OS_cpuTemperatures(){ return QStringList(); } // (one per CPU) Temperature of CPU ("50C" for example)

bool OSInterface::OS_memorySupported(){ return false; }
int OSInterface::OS_memoryUsedPercentage(){ return -1; } //percentage: 0-100 with -1 for errors
QString OSInterface::OS_memoryTotal(){ return QString(); } //human-readable form - does not tend to change within a session
QStringList OSInterface::OS_diskIO(){ return QStringList(); } //Returns list of current read/write stats for each device

bool OSInterface::OS_diskSupported(){ return false; }
int OSInterface::OS_fileSystemPercentage(QString dir){ return -1; } //percentage of capacity used: 0-100 with -1 for errors
QString OSInterface::OS_fileSystemCapacity(QString dir){ return QString(); } //human-readable form - total capacity

// = OS-Specific Utilities =
QString OSInterface::controlPanelShortcut(){ return "pccontrol.desktop"; } //relative *.desktop shortcut name (Example: "some_utility.desktop")
QString OSInterface::audioMixerShortcut(){ return "pc-mixer -notray"; } //relative *.desktop shortcut name (Example: "some_utility.desktop")
QString OSInterface::appStoreShortcut(){ return "appcafe.desktop"; } //relative *.desktop shortcut name (Example: "some_utility.desktop")
QString OSInterface::networkManagerUtility(){ return "pc-netmanager.desktop"; } //relative *.desktop shortcut name (Example: "some_utility.desktop")

//FileSystemWatcher slots (optional - re-implement only if needed/used by this OS)
void OSInterface::watcherFileChanged(QString){} //any additional parsing for files that are watched
void OSInterface::watcherDirChanged(QString dir){ //any additional parsing for watched directories
  if(handleMediaDirChange(dir)){ return; } //auto-handled media directories
}

//IO Device slots (optional - implement only if needed/used by this OS)
void OSInterface::iodeviceReadyRead(){}
void OSInterface::iodeviceAboutToClose(){}

void OSInterface::netRequestFinished(QNetworkReply*){}
void OSInterface::netSslErrors(QNetworkReply*, const QList<QSslError>&){}
