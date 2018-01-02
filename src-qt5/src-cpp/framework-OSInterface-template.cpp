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
  setupNetworkManager();
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
bool OSInterface::networkAvailable(){
  if(INFO.contains("netaccess/available")){ return INFO.value("netaccess/available").toBool(); }
  return false;
}

QString OSInterface::networkType(){
  if(INFO.contains("netaccess/type")){ return INFO.value("netaccess/type").toString(); } //"wifi", "wired", or "cell"
  return "";
}

float OSInterface::networkStrength(){ return -1; } //percentage. ("wired" type should always be 100%)

QString OSInterface::networkHostname(){
  return QHostInfo::localHostName();
}

QHostAddress OSInterface::networkAddress(){
  QString addr;
  if(INFO.contains("netaccess/address")){ addr = INFO.value("netaccess/address").toString(); }
  return QHostAddress(addr);
}
// = Network Modification =

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
bool OSInterface::hasControlPanel(){ return false; }
QString OSInterface::controlPanelShortcut(){ return QString(); } //relative *.desktop shortcut name (Example: "some_utility.desktop")
bool OSInterface::hasAudioMixer(){ return false; }
QString OSInterface::audioMixerShortcut(){ return QString(); } //relative *.desktop shortcut name (Example: "some_utility.desktop")
bool OSInterface::hasAppStore(){ return false; }
QString OSInterface::appStoreShortcut(){ return QString(); } //relative *.desktop shortcut name (Example: "some_utility.desktop")

//FileSystemWatcher slots
void OSInterface::watcherFileChanged(QString){}
void OSInterface::watcherDirChanged(QString dir){
  if(handleMediaDirChange(dir)){ return; }
}

//IO Device slots
void OSInterface::iodeviceReadyRead(){}
void OSInterface::iodeviceAboutToClose(){}

//NetworkAccessManager slots
void OSInterface::netAccessChanged(QNetworkAccessManager::NetworkAccessibility stat){
  INFO.insert("netaccess/available", stat== QNetworkAccessManager::Accessible);
  //Update all the other network status info at the same time
  QNetworkConfiguration active = netman->activeConfiguration();
  //Type of connection
  QString type;
  switch(active.bearerTypeFamily()){
    case QNetworkConfiguration::BearerEthernet: type="wired"; break;
    case QNetworkConfiguration::BearerWLAN: type="wifi"; break;
    case QNetworkConfiguration::Bearer2G: type="cell-2G"; break;
    case QNetworkConfiguration::Bearer3G: type="cell-3G"; break;
    case QNetworkConfiguration::Bearer4G: type="cell-4G"; break;
    default: type="";
  }
  INFO.insert("netaccess/type", type);
  qDebug() << "Detected Device Status:" << active.identifier() << type << stat;
  QNetworkInterface iface = QNetworkInterface::interfaceFromName(active.name());
  qDebug() << " - Configuration: Name:" << active.name() << active.bearerTypeName() << active.identifier();
  qDebug() << " - Interface: MAC Address:" << iface.hardwareAddress() << "Name:" << iface.name() << iface.humanReadableName() << iface.isValid();
  QList<QNetworkAddressEntry> addressList = iface.addressEntries();
  QStringList address;
  //NOTE: There are often 2 addresses, IPv4 and IPv6
  for(int i=0; i<addressList.length(); i++){
    address << addressList[i].ip().toString();
  }
  qDebug() << " - IP Address:" << address;
  qDebug() << " - Hostname:" << networkHostname();
  INFO.insert("netaccess/address", address.join(", "));
  emit networkStatusChanged();
}

void OSInterface::netRequestFinished(QNetworkReply*){}
void OSInterface::netSslErrors(QNetworkReply*, const QList<QSslError>&){}
void OSInterface::timerUpdate(){}
