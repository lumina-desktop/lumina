//===========================================
//  Lumina desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// Internal, OS-agnostic functionality for managing the object itself
//===========================================
#include <framework-OSInterface.h>
#include <QFile>
#include <QDir>
#include <QVariant>

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
  connect(netman, SIGNAL(requestFinished(QNetworkReply*)), this, SLOT(netRequestFinished(QNetworkReply*)) );
  connect(netman, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)), this, SLOT(netSslErrors(QNetworkReply*, const QList<QSslError>&)) );
}

void OSInterface::connectTimer(){
  if(timer==0){ return; }
  connect(timer, SIGNAL(timeout()), this, SLOT(timerUpdate()) );
}

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

// Qt-based NetworkAccessManager usage
void OSInterface::setupNetworkManager(){
  if(netman==0){
    netman = new QNetworkAccessManager(this);
    connectNetman();
  }
  //Load the initial state of the network accessibility
  netAccessChanged(netman->networkAccessible());
}
