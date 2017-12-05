//===========================================
//  Lumina desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// Internal, OS-agnostic functionality for managing the object itself
//===========================================
#include <OSInterface.h>

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

OSInterface::OSInterface* instance(){
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
