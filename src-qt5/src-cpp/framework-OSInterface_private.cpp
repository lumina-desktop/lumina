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
  setupNetworkManager(60000, 1); //will create/connect the network monitor automatically
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
  connect(netman, SIGNAL(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)), this, SLOT(NetworkTimerUpdate()) );
  connect(netman, SIGNAL(finished(QNetworkReply*)), this, SLOT(netRequestFinished(QNetworkReply*)) );
  connect(netman, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)), this, SLOT(netSslErrors(QNetworkReply*, const QList<QSslError>&)) );
}

bool OSInterface::verifyAppOrBin(QString chk){
  bool valid = !chk.isEmpty();
  if(chk.contains(" ")){ chk = chk.section(" ",0,0); }
  if(valid && chk.endsWith(".desktop")){
    if(chk.startsWith("/")){ return QFile::exists(chk); }
    valid = false;
    QStringList paths;
      paths << QString(getenv("XDG_DATA_HOME")) << QString(getenv("XDG_DATA_DIRS")).split(":");
    for(int i=0; i<paths.length() && !valid; i++){
      if(QFile::exists(paths[i]+"/applications")){ valid = findInDirectory(chk, paths[i]+"/applications", true); }
    }
  }else if(valid){
    //Find the absolute path for this binary
    if(!chk.startsWith("/")){
      QStringList paths = QString(getenv("PATH")).split(":");
      for(int i=0; i<paths.length(); i++){
        if(QFile::exists(paths[i]+"/"+chk)){ chk = paths[i]+"/"+chk; break; }
      }
      if(!chk.startsWith("/")){ return false; } //could not find the file
    }else if(!QFile::exists(chk)){
      return false; //file does not exist
    }
    //Make sure it is executable by the user
    valid = QFileInfo(chk).isExecutable();
  }
  return valid;
}

QString OSInterface::runProcess(int &retcode, QString command, QStringList arguments, QString workdir, QStringList env){
  QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels); //need output
  //First setup the process environment as necessary
  QProcessEnvironment PE = QProcessEnvironment::systemEnvironment();
    if(!env.isEmpty()){
      for(int i=0; i<env.length(); i++){
    if(!env[i].contains("=")){ continue; }
        PE.insert(env[i].section("=",0,0), env[i].section("=",1,100));
      }
    }
    proc.setProcessEnvironment(PE);
  //if a working directory is specified, check it and use it
  if(!workdir.isEmpty()){
    proc.setWorkingDirectory(workdir);
  }
  //Now run the command (with any optional arguments)
  if(arguments.isEmpty()){ proc.start(command); }
  else{ proc.start(command, arguments); }
  //Wait for the process to finish (but don't block the event loop)
  for(int i=0; i<10 && !proc.waitForFinished(500); i++){ //maximum of 5 seconds for command to finish
    if(proc.state() == QProcess::NotRunning){ break; } //somehow missed the finished signal - go ahead and stop now
  }
  if(proc.state() != QProcess::NotRunning){ proc.terminate(); } //just in case - make sure to kill off the process
  QString info = proc.readAllStandardOutput();
  retcode = proc.exitCode(); //return success/failure
  return info;
}

int OSInterface::runCmd(QString command, QStringList args){
  int retcode;
  runProcess(retcode, command, args);
  return retcode;
}

QStringList OSInterface::getCmdOutput(QString command, QStringList args){
  int retcode;
  return runProcess(retcode, command, args).split("\n");
}

bool OSInterface::findInDirectory(QString file, QString dirpath, bool recursive){
  bool found = QFile::exists(dirpath+"/"+file);
  if(!found && recursive){
    QDir dir(dirpath);
    QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for(int i=0; i<dirs.length() && !found; i++){ found = findInDirectory(file, dir.absoluteFilePath(dirs[i]), recursive); }
  }
  return found;
}

QString OSInterface::readFile(QString path){
  QFile file(path);
  QString info;
  if(file.open(QIODevice::ReadOnly)){
    QTextStream out(&file);
    info = out.readAll();
    file.close();
  }
  return info;
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
void OSInterface::setupNetworkManager(int update_ms, int delay_ms){
  if(netman==0){
    netman = new QNetworkAccessManager(this);
    connectNetman();
  }
  networkTimer = new QTimer(this);
    networkTimer->setSingleShot(true);
    networkTimer->setInterval(update_ms);
    connect(networkTimer, SIGNAL(timeout()), this, SLOT(NetworkTimerUpdate()) );
  QTimer::singleShot(delay_ms, this, SLOT(NetworkTimerUpdate()) );
}

bool OSInterface::networkAvailable(){
  if(INFO.contains("netaccess/available")){ return INFO.value("netaccess/available").toBool(); }
  return false;
}

QString OSInterface::networkType(){
  if(INFO.contains("netaccess/type")){ return INFO.value("netaccess/type").toString(); } //"wifi", "wired", or "cell"
  return "";
}

float OSInterface::networkStrength(){
  if(INFO.contains("netaccess/strength")){ return INFO.value("netaccess/strength").toFloat(); } //percentage
  return -1;
}

QString OSInterface::networkIcon(){
  if(INFO.contains("netaccess/icon")){ return INFO.value("netaccess/icon").toString(); }
  return "";
}

QString OSInterface::networkHostname(){
  return QHostInfo::localHostName();
}

QStringList OSInterface::networkAddress(){
  QString addr;
  if(INFO.contains("netaccess/address")){ addr = INFO.value("netaccess/address").toString(); }
  return addr.split(", ");
}

bool OSInterface::hasNetworkManager(){
  return verifyAppOrBin(networkManagerUtility());
}

QString OSInterface::networkStatus(){
  QString stat = "<b>%1</b><br>%2<br>%3";
  return stat.arg(networkHostname(), networkType(), networkAddress().join("<br>"));
}

//NetworkAccessManager slots
void OSInterface::syncNetworkInfo(OSInterface *os, QHash<QString, QVariant> *hash, QTimer *timer){
  //qDebug() << "[DEBUG] Got Net Access Changed";
  hash->insert("netaccess/available", netman->networkAccessible()== QNetworkAccessManager::Accessible);
  //Update all the other network status info at the same time
  QNetworkConfiguration active;
  QList<QNetworkConfiguration> netconfigL = netman->configuration().children();
  for(int i=0; i<netconfigL.length(); i++){
    if(!netconfigL[i].state().testFlag(QNetworkConfiguration::Discovered) ){ continue; } //skip this interface
    QList<QNetworkAddressEntry> addressList = QNetworkInterface::interfaceFromName(netconfigL[i].name()).addressEntries();
    //NOTE: There are often 2 addresses, IPv4 and IPv6
    bool ok = false;
    for(int j=0; j<addressList.length() && !ok; j++){
      if( addressList[j].ip().isLoopback() ){ continue; }
      addressList[j].ip().toIPv4Address(&ok);
    }
    if(ok){ active = netconfigL[i]; break; } //found a good one with a valid IPv4
    //else if(!active.isValid()){
  }
  if(!active.isValid()){ active = netman->activeConfiguration(); } //use the default Qt-detected interface
  //Type of connection
  QString type;
  switch(active.bearerTypeFamily()){
    case QNetworkConfiguration::BearerEthernet: type="wired"; break;
    case QNetworkConfiguration::BearerWLAN: type="wifi"; break;
    case QNetworkConfiguration::Bearer2G: type="cell-2G"; break;
    case QNetworkConfiguration::Bearer3G: type="cell-3G"; break;
    case QNetworkConfiguration::Bearer4G: type="cell-4G"; break;
    default: type=OS_networkTypeFromDeviceName(active.name()); //could not be auto-determined - run the OS-specific routine
  }
  hash->insert("netaccess/type", type);
  float strength = 100;
  if(type!="wired"){ strength = OS_networkStrengthFromDeviceName(active.name()); }
  hash->insert("netaccess/strength", strength);

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
  hash->insert("netaccess/address", address.join(", "));

  //Figure out the icon used for this type/strnegth
  QString icon;
  if(type.startsWith("cell")){
    if(address.isEmpty()){ icon = "network-cell-off"; }
    else if(strength>80){ icon = "network-cell-connected-100"; }
    else if(strength>60){ icon = "network-cell-connected-75"; }
    else if(strength>40){ icon = "network-cell-connected-50"; }
    else if(strength>10){ icon = "network-cell-connected-25"; }
    else if(strength >=0){ icon = "network-cell-connected-00"; }
    else{ icon = "network-cell"; } //unknown strength - just use generic icon so we at least get off/on visibility
  }else if(type=="wifi"){
    if(address.isEmpty()){ icon = "network-wireless-off"; }
    else if(strength>80){ icon = "network-wireless-100"; }
    else if(strength>60){ icon = "network-wireless-75"; }
    else if(strength>40){ icon = "network-wireless-50"; }
    else if(strength>10){ icon = "network-wireless-25"; }
    else if(strength >=0){ icon = "network-wireless-00"; }
    else{ icon = "network-wireless"; } //unknown strength - just use generic icon so we at least get off/on visibility
  }else if(type=="wired"){
    if(strength==100 && !address.isEmpty()){ icon = "network-wired-connected"; }
    else if(strength==100){ icon = "network-wired-pending"; }
    else{ icon = "network-wired-disconnected"; }
  }else{
    icon = "network-workgroup"; //failover to a generic "network" icon
  }
  hash->insert("netaccess/icon",icon);
  //qDebug() << "[DEBUG] Emit NetworkStatusChanged";
  os->emit networkStatusChanged();
  QTimer::singleShot(0, timer, SLOT(start()));
}


// ========================
//     TIMER-BASED MONITORS
// ========================
//Timer slots

void OSInterface::NetworkTimerUpdate(){
  if(networkTimer->isActive()){ networkTimer->stop(); } //just in case this was manually triggered
  QtConcurrent::run(this, &OSInterface::syncNetworkInfo, this, &INFO, networkTimer);
}

void OSInterface::BatteryTimerUpdate(){
  if(batteryTimer->isActive()){ batteryTimer->stop(); } //just in case this was manually triggered
  QtConcurrent::run(this, &OSInterface::syncBatteryInfo, this, &INFO, batteryTimer);
}

void OSInterface::UpdateTimerUpdate(){
  if(updateTimer->isActive()){ updateTimer->stop(); } //just in case this was manually triggered
  QtConcurrent::run(this, &OSInterface::syncUpdateInfo, this, &INFO, updateTimer);
}

void OSInterface::BrightnessTimerUpdate(){
  if(brightnessTimer->isActive()){ brightnessTimer->stop(); } //just in case this was manually triggered
  QtConcurrent::run(this, &OSInterface::syncBrightnessInfo, this, &INFO, brightnessTimer);
}

void OSInterface::VolumeTimerUpdate(){
  if(volumeTimer->isActive()){ volumeTimer->stop(); } //just in case this was manually triggered
  QtConcurrent::run(this, &OSInterface::syncVolumeInfo, this, &INFO, volumeTimer);
}

void OSInterface::CpuTimerUpdate(){
  if(cpuTimer->isActive()){ cpuTimer->stop(); } //just in case this was manually triggered
  QtConcurrent::run(this, &OSInterface::syncCpuInfo, this, &INFO, cpuTimer);
}

void OSInterface::MemTimerUpdate(){
  if(memTimer->isActive()){ memTimer->stop(); } //just in case this was manually triggered
  QtConcurrent::run(this, &OSInterface::syncMemoryInfo, this, &INFO, memTimer);
}

void OSInterface::DiskTimerUpdate(){
  if(diskTimer->isActive()){ diskTimer->stop(); } //just in case this was manually triggered
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
  float charge = OS_batteryCharge();
  bool charging = OS_batteryCharging();
  double secs = OS_batterySecondsLeft();
  //Check for any alert generations
  if(charging && hash->value("battery/percent",100).toFloat() <= 99 && charge>99){ os->emit BatteryFullAlert(); }
  else if(!charging && hash->value("battery/percent", 50).toFloat()>10 && charge<10){ os->emit BatteryEmptyAlert(); }

  hash->insert("battery/percent",charge);
  hash->insert("battery/charging",charging);
  //Convert the seconds to human-readable
  QString time;
    if(secs>3600){
      time = QString::number( qRound(secs/360.0)/10.0 )+" h";
    }else if(secs>60){
      time = QString::number( qRound(secs/6.0)/10.0 )+" m";
    }else if(secs>0){
      time = QString::number(secs)+" s";
    }
  hash->insert("battery/time", time);
  //Determine the icon which should be used for this status
  QString icon;
  if(charging){
    if(charge>=99){ icon="battery-charging"; }
    else if(charge>80){ icon="battery-charging-80"; }
    else if(charge >60){ icon="battery-charging-60"; }
    else if(charge >30){ icon="battery-charging-40"; }
    else if(charge >0){ icon="battery-charging-20"; }
    else{ icon="battery-unknown"; }
  }else{
    if(charge>90){ icon="battery"; }
    else if(charge>80){ icon="battery-80"; }
    else if(charge >60){ icon="battery-60"; }
    else if(charge >30){ icon="battery-40"; }
    else if(charge >10){ icon="battery-20"; }
    else if(charge >0){ icon="battery-alert"; }
    else{ icon="battery-unknown"; }
  }
  hash->insert("battery/icon",icon);
  //Now emit the change signal and restart the timer
  os->emit batteryChanged();
  QTimer::singleShot(0, timer, SLOT(start()));
}

void OSInterface::syncUpdateInfo(OSInterface *os, QHash<QString, QVariant> *hash, QTimer *timer){
  //Get the current status
  QString status, icon;
  if(OS_updatesRunning()){
    status="running"; icon="sync";
  }else if(OS_updatesFinished()){
    status="finished"; icon="security-high";
  }else if(OS_updatesAvailable()){
    status="available"; icon="security-medium";
  }
  //Save the current info into the hash (if different)
  if(status != updateStatus()){
    hash->insert("updates/status", status);
    hash->insert("updates/icon", icon);
    os->emit updateStatusChanged();
  }
  QTimer::singleShot(0, timer, SLOT(start()));
}

void OSInterface::syncBrightnessInfo(OSInterface *os, QHash<QString, QVariant> *hash, QTimer *timer){

  QTimer::singleShot(0, timer, SLOT(start()));
}

void OSInterface::syncVolumeInfo(OSInterface *os, QHash<QString, QVariant> *hash, QTimer *timer){
  int oldvol = volume();
  int newvol = OS_volume();
  if(oldvol!=newvol && newvol>=0){
    hash->insert("volume/current",newvol);
    QString icon;
    if(newvol>66){ icon = "audio-volume-high"; }
    else if(newvol>33){ icon = "audio-volume-medium"; }
    else if(newvol>0){ icon = "audio-volume-low"; }
    else{ icon = "audio-volume-muted"; }
    hash->insert("volume/icon",icon);
    os->emit volumeChanged();
  }
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

// = Battery =
bool OSInterface::batteryAvailable(){ return OS_batteryAvailable(); }
float OSInterface::batteryCharge(){
  if(INFO.contains("battery/percent")){ return INFO.value("battery/percent").toFloat(); }
  return -1;
}
bool OSInterface::batteryCharging(){
  if(INFO.contains("battery/charging")){ return INFO.value("battery/charging").toBool(); }
  return false;
}
QString OSInterface::batteryRemaining(){
  if(INFO.contains("battery/time")){ return INFO.value("battery/time").toString(); }
  return "";
}
QString OSInterface::batteryIcon(){
  if(INFO.contains("battery/icon")){ return INFO.value("battery/icon").toString(); }
  return "";
}

QString OSInterface::batteryStatus(){
  QString text = QString::number(batteryCharge())+"%";
  if(!batteryCharging()){
    QString time = batteryRemaining();
    if(!time.isEmpty()){
      text.append(" ("+time+")");
    }
  }
  return text;
}

// = Volume =
bool OSInterface::volumeSupported(){ return OS_volumeSupported(); }
int OSInterface::volume(){
  if(INFO.contains("volume/current")){ return INFO.value("volume/current").toInt(); }
  return 0;
}

void OSInterface::setVolume(int vol){
  OS_setVolume(vol);
  VolumeTimerUpdate(); //update the internal cache
}

QString OSInterface::volumeIcon(){
  if(INFO.contains("volume/icon")){ return INFO.value("volume/icon").toString(); }
  return "";
}

// = Media =
QStringList OSInterface::mediaDirectories(){ return OS_mediaDirectories(); }
QStringList OSInterface::mediaShortcuts(){ return autoHandledMediaFiles(); } //List of currently-available XDG shortcut file paths

// = Updates =
bool OSInterface::updatesSupported(){ return OS_updatesSupported(); }
QString OSInterface::updateStatus(){
  if(INFO.contains("updates/status")){ return INFO.value("updates/status").toString(); }
  return "";
}
bool OSInterface::updateInfoAvailable(){
  return !updateStatus().isEmpty();
}

QString OSInterface::updateIcon(){
  if(INFO.contains("updates/icon")){ return INFO.value("updates/icon").toString(); }
  return "";
}

QString OSInterface::updateStatusInfo(){
  QString status = updateStatus();
  if(status=="available"){ return updateDetails(); }
  else if(status=="running"){ return updateLog(); }
  else if(status=="finished"){ return updateResults(); }
  return "";
}

QString OSInterface::updateDetails(){
  return OS_updateDetails(); //don't cache these types of logs - too large
}

QString OSInterface::updateLog(){
  return OS_updateLog(); //don't cache these types of logs - too large and change too often
}

QString OSInterface::updateResults(){
  return OS_updateResults(); //don't cache these types of logs - too large
}

void OSInterface::startUpdates(){ OS_startUpdates(); }
bool OSInterface::updateOnlyOnReboot(){ return OS_updateOnlyOnReboot(); }
bool OSInterface::updateCausesReboot(){ return OS_updateCausesReboot(); }

QDateTime OSInterface::lastUpdate(){ return OS_lastUpdate(); }
QString OSInterface::lastUpdateResults(){ return OS_lastUpdateResults(); }

// = System Power =
bool OSInterface::canReboot(){ return OS_canReboot(); }
void OSInterface::startReboot(){ OS_startReboot(); }
bool OSInterface::canShutdown(){ return OS_canShutdown(); }
void OSInterface::startShutdown(){ OS_startShutdown(); }
bool OSInterface::canSuspend(){ return OS_canSuspend(); }
void OSInterface::startSuspend(){ OS_startSuspend(); }

// = Screen Brightness =
bool OSInterface::brightnessSupported(){ return OS_brightnessSupported(); }
int OSInterface::brightness(){
  if(INFO.contains("brightness/percent")){ return INFO.value("brightness/percent").toInt(); }
  return 100;
}
void OSInterface::setBrightness(int percent){
  OS_setBrightness(percent);
  BrightnessTimerUpdate(); //update internal cache ASAP
}

// = System Status Monitoring
bool OSInterface::cpuSupported(){ return OS_cpuSupported(); }
QList<int> OSInterface::cpuPercentage(){ return QList<int>(); } // (one per CPU) percentage: 0-100 with empty list for errors
QStringList OSInterface::cpuTemperatures(){ return QStringList(); } // (one per CPU) Temperature of CPU ("50C" for example)

bool OSInterface::memorySupported(){ return false; }
int OSInterface::memoryUsedPercentage(){ return -1; } //percentage: 0-100 with -1 for errors
QString OSInterface::memoryTotal(){ return QString(); } //human-readable form - does not tend to change within a session
QStringList OSInterface::diskIO(){ return QStringList(); } //Returns list of current read/write stats for each device

bool OSInterface::diskSupported(){ return false; }
int OSInterface::fileSystemPercentage(QString dir){ return -1; } //percentage of capacity used: 0-100 with -1 for errors
QString OSInterface::fileSystemCapacity(QString dir){ return QString(); } //human-readable form - total capacity
