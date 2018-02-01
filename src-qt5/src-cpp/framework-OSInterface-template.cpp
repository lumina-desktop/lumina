//===========================================
//  Lumina desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// USEFUL INTERNAL FUNCTIONS: (See framework-OSInterface.h for all possibilities);
//----------------------------------------------
//	bool verifyAppOrBin(QString chk) : Returns true is the check is a valid binary or application (*.desktop)
//	int runCmd(QString command, QStringList arguments) : return code of command is returned
//	QStringList getCmdOutput(QString command, QStringList arguments) : returns standard output of command
//===========================================
#include <framework-OSInterface.h>

// = Battery =
bool OSInterface::OS_batteryAvailable(){ return false; }
float OSInterface::OS_batteryCharge(){ return -1; }
bool OSInterface::OS_batteryCharging(){ return false; }
double OSInterface::OS_batterySecondsLeft(){ return -1; }

// = Volume =
bool OSInterface::OS_volumeSupported(){ return false; }
int OSInterface::OS_volume(){ return -1; }
bool OSInterface::OS_setVolume(int){ return false;}

// = Network Information =
QString OSInterface::OS_networkTypeFromDeviceName(QString name){
  //Return options: wifi, wired, cell, cell-2G, cell-3G, cell-4G
  return "";
}

float OSInterface::OS_networkStrengthFromDeviceName(QString name){
  //NOTE: This will only run for non-wired devices (wifi, cell[-*])
  return -1; //percentage
}

// = Media Shortcuts =
QStringList OSInterface::OS_mediaDirectories(){ return QStringList() << "/media"; } //directory where XDG shortcuts are placed for interacting with media (local/remote)

// = Updates =
bool OSInterface::OS_updatesSupported(){ return false; }
bool OSInterface::OS_updatesAvailable(){ return false; }
QString OSInterface::OS_updateDetails(){ return QString(); }	//Information about any available updates
bool OSInterface::OS_updatesRunning(){ return false; }
QString OSInterface::OS_updateLog(){ return QString(); }		//Information about any currently-running update
bool OSInterface::OS_updatesFinished(){ return false; }
QString OSInterface::OS_updateResults(){ return QString(); }	//Information about any finished update
void OSInterface::OS_startUpdates(){}
bool OSInterface::OS_updateOnlyOnReboot(){ return false; } //Should the startUpdates function be called only when rebooting the system?
bool OSInterface::OS_updateCausesReboot(){ return false; }
QDateTime OSInterface::OS_lastUpdate(){ return QDateTime(); }	//The date/time of the previous updates
QString OSInterface::OS_lastUpdateResults(){ return QString(); } //Information about the previously-finished update

// = System Power =
bool OSInterface::OS_canReboot(){ return false; }
void OSInterface::OS_startReboot(){}
bool OSInterface::OS_canShutdown(){ return false; }
void OSInterface::OS_startShutdown(){}
bool OSInterface::OS_canSuspend(){ return false; }
void OSInterface::OS_startSuspend(){}

// = Screen Brightness =
bool OSInterface::OS_brightnessSupported(){ return false; }
int OSInterface::OS_brightness(){ return -1; } //percentage: 0-100 with -1 for errors
bool OSInterface::OS_setBrightness(int){ return false; }

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
QString OSInterface::controlPanelShortcut(){ return QString(); } //relative *.desktop shortcut name (Example: "some_utility.desktop")
QString OSInterface::audioMixerShortcut(){ return QString(); } //relative *.desktop shortcut name (Example: "some_utility.desktop")
QString OSInterface::appStoreShortcut(){ return QString(); } //relative *.desktop shortcut name (Example: "some_utility.desktop")
QString OSInterface::networkManagerUtility(){ return QString(); } //relative *.desktop shortcut name (Example: "some_utility.desktop")

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
