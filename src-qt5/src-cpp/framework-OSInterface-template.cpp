//===========================================
//  Lumina desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include <framework-OSInterface.h>
#include <QNetworkConfiguration>
#include <QNetworkInterface>

//Start/stop interface watchers/notifications
void OSInterface::start(){
  setupMediaWatcher(); //will create/connect the filesystem watcher automatically
  setupNetworkManager(); //will create/connect the network monitor automatically

}

void OSInterface::stop(){
  if(isRunning()){
    watcher->deleteLater();
    watcher = 0;
  }
}

bool OSInterface::isRunning(){ return (watcher!=0); } //status of the object - whether it has been started yet

// = Battery =
bool OSInterface::batteryAvailable(){ return false; }
float OSInterface::batteryCharge(){ return -1; }
bool OSInterface::batteryCharging(){ return false; }
double OSInterface::batterySecondsLeft(){ return -1; }

// = Volume =
bool OSInterface::volumeAvailable(){ return false; }
int OSInterface::volume(){ return -1; }
void OSInterface::setVolume(int){}

// = Network Information =
QString OSInterface::networkTypeFromDeviceName(QString name){
  //Return options: wifi, wired, cell, cell-2G, cell-3G, cell-4G
  return "";
}

float OSInterface::networkStrength(){
  //QString device = INFO.value("netaccess/devicename");
  return -1; //percentage. ("wired" type should always be 100%)
}

// = Media Shortcuts =
QStringList OSInterface::mediaDirectories(){ return QStringList() << "/media"; } //directory where XDG shortcuts are placed for interacting with media (local/remote)
QStringList OSInterface::mediaShortcuts(){ return autoHandledMediaFiles(); } //List of currently-available XDG shortcut file paths

// = Updates =
bool OSInterface::updatesAvailable(){ return false; }
QString OSInterface::updateDetails(){ return QString(); }	//Information about any available updates
bool OSInterface::updatesRunning(){ return false; }
QString OSInterface::updateLog(){ return QString(); }		//Information about any currently-running update
bool OSInterface::updatesFinished(){ return false; }
QString OSInterface::updateResults(){ return QString(); }	//Information about any finished update
void OSInterface::startUpdates(){}
bool OSInterface::updateOnlyOnReboot(){ return false; } //Should the startUpdates function be called only when rebooting the system?
QDateTime OSInterface::lastUpdate(){ return QDateTime(); }	//The date/time of the previous updates
QString OSInterface::lastUpdateResults(){ return QString(); } //Information about the previously-finished update

// = System Power =
bool OSInterface::canReboot(){ return false; }
void OSInterface::startReboot(){}
bool OSInterface::canShutdown(){ return false; }
void OSInterface::startShutdown(){}
bool OSInterface::canSuspend(){ return false; }
void OSInterface::startSuspend(){}

// = Screen Brightness =
int OSInterface::brightness(){ return -1; } //percentage: 0-100 with -1 for errors
void OSInterface::setBrightness(int){}

// = System Status Monitoring
QList<int> OSInterface::cpuPercentage(){ return QList<int>(); } // (one per CPU) percentage: 0-100 with empty list for errors
QStringList OSInterface::cpuTemperatures(){ return QStringList(); } // (one per CPU) Temperature of CPU ("50C" for example)
int OSInterface::memoryUsedPercentage(){ return -1; } //percentage: 0-100 with -1 for errors
QString OSInterface::memoryTotal(){ return QString(); } //human-readable form - does not tend to change within a session
QStringList OSInterface::diskIO(){ return QStringList(); } //Returns list of current read/write stats for each device
int OSInterface::fileSystemPercentage(QString dir){ return -1; } //percentage of capacity used: 0-100 with -1 for errors
QString OSInterface::fileSystemCapacity(QString dir){ return QString(); } //human-readable form - total capacity

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
void OSInterface::timerUpdate(){}
