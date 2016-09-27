//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "Browser.h"

Browser::Browser(QObject *parent) : QObject(parent){
  watcher = new QFileSystemWatcher(this);
  connect(watcher, SIGNAL(fileChanged(const QString&)), this, SLOT(fileChanged(QString)) );
  connect(watcher, SIGNAL(directoryChanged(const QString&)), this, SLOT(dirChanged(QString)) );
  showHidden = false;
}

Browser::~Browser(){
  watcher->deleteLater();
}

QString Browser::currentDirectory(){ return currentDir; }

void Browser::showHiddenFiles(bool show){
  if(show !=showHidden){
    showHidden = show;
    QTimer::singleShot(0, this, SLOT(loadDirectory()) );
  }
}
bool Browser::showingHiddenFiles(){
  return showHidden;
}

//   PRIVATE
void Browser::loadItem(QFileInfo info){
  LFileInfo linfo(info);
  QIcon ico;
  if(linfo.isImage()){
    QPixmap pix;
    if(pix.load(info.absoluteFilePath()) ){
      if(pix.height()>128){ pix = pix.scaled(128, 128, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation); }
      ico.addPixmap(pix);
    }
  }else if(linfo.isDirectory()){
    ico = LXDG::findIcon("folder","inode/directory");
  }
  if(ico.isNull()){ ico = LXDG::findIcon(linfo.mimetype(), "unknown"); }
  emit ItemDataAvailable(ico, linfo);
}

// PRIVATE SLOTS
void Browser::fileChanged(QString file){
  if(file.startsWith(currentDir+"/")){ emit itemUpdated(file); }
  else if(file==currentDir){ QTimer::singleShot(0, this, SLOT(loadDirectory()) ); }
}

void Browser::dirChanged(QString dir){
  if(dir==currentDir){ QTimer::singleShot(0, this, SLOT(loadDirectory()) ); }
  else if(dir.startsWith(currentDir)){ emit itemUpdated(dir); }
}

// PUBLIC SLOTS
QString Browser::loadDirectory(QString dir){
  if(dir.isEmpty()){ dir = currentDir; } //reload current directory
  if(dir.isEmpty()){ return; } //nothing to do - nothing previously loaded
  //clean up the watcher first
  QStringList watched; watched << watcher->files() << watcher->directories();
  if(!watched.isEmpty()){ watcher->removePaths(watched); }
  emit clearItems(); //let the main widget know to clear all current items
  //QApplication::processEvents();
  // read the given directory
  QDir directory(dir);
  if(directory.exists()){
    QFileInfoList files;
    if(showHidden){ files = directory.entryInfoList( QDir::Dirs | QDir::Files | QDir::Hidden | QDir::NoDotOrDotDot, QDir::NoSort); }
    else{ files = directory.entryInfoList( QDir::Dirs | QDir::Files | QDir::NoDotOrDotDot, QDir::NoSort); }
    for(int i=0; i<files.length(); i++){
      watcher->addPath(files[i].absoluteFilePath());
      QtConcurrent::run(this, &Browser::loadDirectory, files[i]);
    }
    watcher->addPath(directory.absolutePath());
  }
}
