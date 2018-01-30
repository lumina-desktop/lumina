//===========================================
//  Lumina desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// Internal, OS-agnostic functionality for managing the object itself
//===========================================
#include <framework-OSInterface.h>
#include <QtConcurrent>

#include <QQmlEngine>

OSInterface::OSInterface(QObject *parent) : QObject(parent){
  watcher = 0;
  iodevice = 0;
  netman = 0;
}

OSInterface::~OSInterface(){
  if(watcher!=0){
    QStringList paths; paths << watcher->files() << watcher->directories();
    if(!paths.isEmpty()){ watcher->removePaths(paths); }
    watcher->deleteLater();
  }
  if(iodevice!=0){
    if(iodevice->isOpen()){ iodevice->close(); }
    iodevice->deleteLater();
  }
  if(netman!=0){
    netman->deleteLater();
  }
}

OSInterface* OSInterface::instance(){
  static OSInterface* m_os_object = 0;
  if(m_os_object==0){
    m_os_object = new OSInterface();
  }
  return m_os_object;
}

void OSInterface::RegisterType(){
  static bool done = false;
  if(done){ return; }
  done=true;
  qmlRegisterType<OSInterface>("Lumina.Backend.OSInterface", 2, 0, "OSInterface");
}

//Start/stop interface systems
void OSInterface::start(){
  if(!mediaDirectories().isEmpty()){ setupMediaWatcher(); }//will create/connect the filesystem watcher automatically
  setupNetworkManager(); //will create/connect the network monitor automatically
  if(batteryAvailable()){ setupBatteryMonitor(30000, 1); } //30 second updates, 1 ms init delay
  if(brightnessSupported()){ setupBrightnessMonitor(60000, 1); } //1 minute updates, 1 ms init delay
  if(volumeSupported()){ setupVolumeMonitor(60000, 2); } //1 minute updates, 2 ms init delay
  if(updatesSupported()){ setupUpdateMonitor(12*60*60*1000, 5*60*1000); } //12-hour updates, 5 minute delay
  if(cpuSupported()){ setupCpuMonitor(2000, 20); } //2 second updates, 20 ms init delay
  if(memorySupported()){ setupMemoryMonitor(2000, 21); } //2 second updates, 21 ms init delay
  if(diskSupported()){ setupDiskMonitor(60000, 25); } //1 minute updates, 25 ms init delay
}

void OSInterface::stop(){
  if(watcher!=0){ watcher->deleteLater();  watcher=0; }
  if(batteryTimer!=0){ batteryTimer->stop(); disconnect(batteryTimer); }
  if(brightnessTimer!=0){ brightnessTimer->stop(); disconnect(brightnessTimer); }
  if(volumeTimer!=0){ volumeTimer->stop(); disconnect(volumeTimer); }
  if(updateTimer!=0){ updateTimer->stop(); disconnect(updateTimer); }
  if(cpuTimer!=0){ cpuTimer->stop(); disconnect(cpuTimer); }
  if(memTimer!=0){ memTimer->stop(); disconnect(memTimer); }
  if(diskTimer!=0){ diskTimer->stop(); disconnect(diskTimer); }
  if(netman!=0){ disconnect(netman); netman->deleteLater(); netman = 0; }
}

bool OSInterface::isRunning(){ return _started; } //status of the object - whether it has been started yet

void OSInterface::connectWatcher(){
  if(watcher==0){ return; }
  connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(watcherFileChanged(QString)) );
  connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(watcherDirChanged(QString)) );
}

void OSInterface::connectIodevice(){
  if(iodevice==0){ return; }
  connect(iodevice, SIGNAL(readyRead()), this, SLOT(iodeviceReadyRead()) );
}

void OSInterface::connectNetman(){
  if(netman==0){ return; }
  connect(netman, SIGNAL(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)), this, SLOT(netAccessChanged(QNetworkAccessManager::NetworkAccessibility)) );
  connect(netman, SIGNAL(finished(QNetworkReply*)), this, SLOT(netRequestFinished(QNetworkReply*)) );
  connect(netman, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)), this, SLOT(netSslErrors(QNetworkReply*, const QList<QSslError>&)) );
}

bool OSInterface::verifyAppOrBin(QString chk){
  bool valid = !chk.isEmpty();
  if(valid && chk.endsWith(".desktop")){
    chk  = LUtils::AppToAbsolute(chk);
    valid = QFile::exists(chk);
  }else if(valid){
    valid = LUtils::isValidBinary(chk);
  }
  return valid;
}

// ===========================
//  OS SPECIFIC EXISTANCE CHECKS
// ===========================
bool OSInterface::hasControlPanel(){
  return verifyAppOrBin(controlPanelShortcut());
}

bool OSInterface::hasAudioMixer(){
  return verifyAppOrBin(audioMixerShortcut());
}

bool OSInterface::hasAppStore(){
  return verifyAppOrBin(appStoreShortcut());
}

// ========================
//        MEDIA DIRECTORIES
// ========================

// External Media Management (if system uses *.desktop shortcuts)
void OSInterface::setupMediaWatcher(){
  //Create/connect the watcher if needed
  if(watcher == 0){ watcher = new QFileSystemWatcher(); connectWatcher(); }
  QStringList dirs = this->mediaDirectories();
  if(dirs.isEmpty()){ return; } //nothing to do
  //Make sure each directory is scanned **right now** (if it exists)
  for(int i=0; i<dirs.length(); i++){
    if(QFile::exists(dirs[i])){
      handleMediaDirChange(dirs[i]);
    }
  }
}

bool OSInterface::handleMediaDirChange(QString dir){ //returns true if directory was handled
  if( !this->mediaDirectories().contains(dir) ){ return false; } //not a media directory
  QDir qdir(dir);
  QStringList files = qdir.entryList(QStringList() << "*.desktop", QDir::Files, QDir::Name);
  for(int i=0; i<files.length(); i++){ files[i]  = qdir.absoluteFilePath(files[i]); }
  QString key = "media_files/"+dir;
  if(files.isEmpty() && INFO.contains(key)){ INFO.remove(key); emit mediaShortcutsChanged(); } //no files for this directory at the moment
  else{ INFO.insert("media_files/"+dir, files); emit mediaShortcutsChanged(); } //save these file paths for later
  //Make sure the directory is still watched (sometimes the dir is removed/recreated on modification)
  if(!watcher->directories().contains(dir)){ watcher->addPath(dir); }
  return true;
}

QStringList OSInterface::autoHandledMediaFiles(){
  QStringList files;
  QStringList keys = INFO.keys().filter("media_files/");
  for(int i=0; i<keys.length(); i++){
    if(keys[i].startsWith("media_files/")){ files << INFO[keys[i]].toStringList(); }
  }
  return files;
}

// =============================
//  NETWORK INTERFACE FUNCTIONS
// =============================
// Qt-based NetworkAccessManager usage
void OSInterface::setupNetworkManager(){
  if(netman==0){
    netman = new QNetworkAccessManager(this);
    connectNetman();
  }
  //Load the initial state of the network accessibility
  netAccessChanged(netman->networkAccessible());
}

bool OSInterface::networkAvailable(){
  if(INFO.contains("netaccess/available")){ return INFO.value("netaccess/available").toBool(); }
  return false;
}

QString OSInterface::networkType(){
  if(INFO.contains("netaccess/type")){ return INFO.value("netaccess/type").toString(); } //"wifi", "wired", or "cell"
  return "";
}

QString OSInterface::networkHostname(){
  return QHostInfo::localHostName();
}

QHostAddress OSInterface::networkAddress(){
  QString addr;
  if(INFO.contains("netaccess/address")){ addr = INFO.value("netaccess/address").toString(); }
  return QHostAddress(addr);
}

bool OSInterface::hasNetworkManager(){
  return verifyAppOrBin(networkManagerUtility());
}

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
    default: type=networkTypeFromDeviceName(active.name()); //could not be auto-determined - run the OS-specific routine
  }
  INFO.insert("netaccess/type", type);
  //qDebug() << "Detected Device Status:" << active.identifier() << type << stat;
  QNetworkInterface iface = QNetworkInterface::interfaceFromName(active.name());
  //qDebug() << " - Configuration: Name:" << active.name() << active.bearerTypeName() << active.identifier();
  //qDebug() << " - Interface: MAC Address:" << iface.hardwareAddress() << "Name:" << iface.name() << iface.humanReadableName() << iface.isValid();
  QList<QNetworkAddressEntry> addressList = iface.addressEntries();
  QStringList address;
  //NOTE: There are often 2 addresses, IPv4 and IPv6
  for(int i=0; i<addressList.length(); i++){
    address << addressList[i].ip().toString();
  }
  //qDebug() << " - IP Address:" << address;
  //qDebug() << " - Hostname:" << networkHostname();
  INFO.insert("netaccess/address", address.join(", "));
  emit networkStatusChanged();
}


// ========================
//     TIMER-BASED MONITORS
// ========================
//Timer slots
void OSInterface::BatteryTimerUpdate(){
  QtConcurrent::run(this, &OSInterface::syncBatteryInfo, this, &INFO, batteryTimer);
}

void OSInterface::UpdateTimerUpdate(){
  QtConcurrent::run(this, &OSInterface::syncUpdateInfo, this, &INFO, updateTimer);
}

void OSInterface::BrightnessTimerUpdate(){
  QtConcurrent::run(this, &OSInterface::syncBrightnessInfo, this, &INFO, brightnessTimer);
}

void OSInterface::VolumeTimerUpdate(){
  QtConcurrent::run(this, &OSInterface::syncVolumeInfo, this, &INFO, volumeTimer);
}

void OSInterface::CpuTimerUpdate(){
  QtConcurrent::run(this, &OSInterface::syncCpuInfo, this, &INFO, cpuTimer);
}

void OSInterface::MemTimerUpdate(){
  QtConcurrent::run(this, &OSInterface::syncMemoryInfo, this, &INFO, memTimer);
}

void OSInterface::DiskTimerUpdate(){
  QtConcurrent::run(this, &OSInterface::syncDiskInfo, this, &INFO, diskTimer);
}

// Timer Setup functions
void OSInterface::setupBatteryMonitor(int update_ms, int delay_ms){
  batteryTimer = new QTimer(this);
    batteryTimer->setSingleShot(true);
    batteryTimer->setInterval(update_ms);
    connect(batteryTimer, SIGNAL(timeout()), this, SLOT(BatteryTimerUpdate()) );
  QTimer::singleShot(delay_ms, this, SLOT(BatteryTimerUpdate()) );
}
void OSInterface::setupUpdateMonitor(int update_ms, int delay_ms){
  updateTimer = new QTimer(this);
    updateTimer->setSingleShot(true);
    updateTimer->setInterval(update_ms);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(UpdateTimerUpdate()) );
  QTimer::singleShot(delay_ms, this, SLOT(UpdateTimerUpdate()) );
}
void OSInterface::setupBrightnessMonitor(int update_ms, int delay_ms){
  brightnessTimer = new QTimer(this);
    brightnessTimer->setSingleShot(true);
    brightnessTimer->setInterval(update_ms);
    connect(brightnessTimer, SIGNAL(timeout()), this, SLOT(BrightnessTimerUpdate()) );
  QTimer::singleShot(delay_ms, this, SLOT(BrightnessTimerUpdate()) );
}
void OSInterface::setupVolumeMonitor(int update_ms, int delay_ms){
  volumeTimer = new QTimer(this);
    volumeTimer->setSingleShot(true);
    volumeTimer->setInterval(update_ms);
    connect(volumeTimer, SIGNAL(timeout()), this, SLOT(VolumeTimerUpdate()) );
  QTimer::singleShot(delay_ms, this, SLOT(VolumeTimerUpdate()) );
}
void OSInterface::setupCpuMonitor(int update_ms, int delay_ms){
  cpuTimer = new QTimer(this);
    cpuTimer->setSingleShot(true);
    cpuTimer->setInterval(update_ms);
    connect(cpuTimer, SIGNAL(timeout()), this, SLOT(CpuTimerUpdate()) );
  QTimer::singleShot(delay_ms, this, SLOT(CpuTimerUpdate()) );
}
void OSInterface::setupMemoryMonitor(int update_ms, int delay_ms){
  memTimer = new QTimer(this);
    memTimer->setSingleShot(true);
    memTimer->setInterval(update_ms);
    connect(memTimer, SIGNAL(timeout()), this, SLOT(MemTimerUpdate()) );
  QTimer::singleShot(delay_ms, this, SLOT(MemTimerUpdate()) );
}
void OSInterface::setupDiskMonitor(int update_ms, int delay_ms){
  diskTimer = new QTimer(this);
    diskTimer->setSingleShot(true);
    diskTimer->setInterval(update_ms);
    connect(diskTimer, SIGNAL(timeout()), this, SLOT(DiskTimerUpdate()) );
  QTimer::singleShot(delay_ms, this, SLOT(DiskTimerUpdate()) );
}

// Timer-based monitor update routines (NOTE: these are all run in a separate thread!!)
void OSInterface::syncBatteryInfo(OSInterface *os, QHash<QString, QVariant> *hash, QTimer *timer){

  QTimer::singleShot(0, timer, SLOT(start()));
}

void OSInterface::syncUpdateInfo(OSInterface *os, QHash<QString, QVariant> *hash, QTimer *timer){

  QTimer::singleShot(0, timer, SLOT(start()));
}

void OSInterface::syncBrightnessInfo(OSInterface *os, QHash<QString, QVariant> *hash, QTimer *timer){

  QTimer::singleShot(0, timer, SLOT(start()));
}

void OSInterface::syncVolumeInfo(OSInterface *os, QHash<QString, QVariant> *hash, QTimer *timer){

  QTimer::singleShot(0, timer, SLOT(start()));
}

void OSInterface::syncCpuInfo(OSInterface *os, QHash<QString, QVariant> *hash, QTimer *timer){

  QTimer::singleShot(0, timer, SLOT(start()));
}

void OSInterface::syncMemoryInfo(OSInterface *os, QHash<QString, QVariant> *hash, QTimer *timer){

  QTimer::singleShot(0, timer, SLOT(start()));
}

void OSInterface::syncDiskInfo(OSInterface *os, QHash<QString, QVariant> *hash, QTimer *timer){

  QTimer::singleShot(0, timer, SLOT(start()));
}
