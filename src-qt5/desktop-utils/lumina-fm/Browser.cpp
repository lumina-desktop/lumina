//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "Browser.h"

#include <QStringList>
#include <QTimer>
#include <QtConcurrent>
#include <QDebug>

#include <LUtils.h>

Browser::Browser(QObject *parent) : QObject(parent){
  watcher = new QFileSystemWatcher(this);
  connect(watcher, SIGNAL(fileChanged(const QString&)), this, SLOT(fileChanged(QString)) );
  connect(watcher, SIGNAL(directoryChanged(const QString&)), this, SLOT(dirChanged(QString)) );
  showHidden = false;
  imageFormats = LUtils::imageExtensions(false); //lowercase suffixes
  connect(this, SIGNAL(threadDone(QString, QByteArray)), this, SLOT(futureFinished(QString, QByteArray)), Qt::QueuedConnection); //will always be between different threads
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
void Browser::loadItem(QString info){
  //qDebug() << "LoadItem:" << info;
  //FileItem item;
     //itemame = info;
  QByteArray bytes;
  if(imageFormats.contains(info.section(".",-1).toLower()) ){
    QFile file(info);
    if(file.open(QIODevice::ReadOnly)){
      bytes = file.readAll();
      file.close();
    }

    /*QPixmap pix;
    if(pix.load(item.info.absoluteFilePath()) ){
      if(pix.height()>128){ pix = pix.scaled(128, 128, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation); }
      item.icon.addPixmap(pix);
    }*/
  }
  //qDebug() << " - done with item:" << info;
  this->emit threadDone(info, bytes);
}

QIcon Browser::loadIcon(QString icon){
  if(!mimeIcons.contains(icon)){ 
    mimeIcons.insert(icon, LXDG::findIcon(icon, "unknown"));
  }

  return mimeIcons[icon];
}


// PRIVATE SLOTS
void Browser::fileChanged(QString file){
  if(file.startsWith(currentDir+"/") ){ 
    if(QFile::exists(file) ){ QtConcurrent::run(this, &Browser::loadItem, file ); } //file modified but not removed
    else{ QTimer::singleShot(0, this, SLOT(loadDirectory()) ); } //file removed - need to update entire dir
  }else if(file==currentDir){ QTimer::singleShot(0, this, SLOT(loadDirectory()) ); }
}

void Browser::dirChanged(QString dir){
  if(dir==currentDir){ QTimer::singleShot(500, this, SLOT(loadDirectory()) ); }
  else if(dir.startsWith(currentDir)){ QtConcurrent::run(this, &Browser::loadItem, dir ); }
}

void Browser::futureFinished(QString name, QByteArray icon){
  //Note: this will be called once for every item that loads
      QIcon ico;
      LFileInfo info(name);
      if(!icon.isEmpty()){
        QPixmap pix;
        if(pix.loadFromData(icon) ){ ico.addPixmap(pix); }
      }else if(info.isDir()){
        ico = loadIcon("folder"); 
        //LXDG::findIcon("folder","inode/directory");
      }
      if(ico.isNull()){ 
	//qDebug() << "MimeType:" << info.fileName() << info.mimetype();
        ico = loadIcon(info.iconfile());
       //ico = LXDG::findIcon( info.iconfile(), "unknown" ); 
      }
      this->emit itemDataAvailable( ico, info );
}

// PUBLIC SLOTS
void Browser::loadDirectory(QString dir){
  //qDebug() << "Load Directory" << dir;
  if(dir.isEmpty()){ dir = currentDir; } //reload current directory
  if(dir.isEmpty()){ return; } //nothing to do - nothing previously loaded
  if(currentDir != dir){ //let the main widget know to clear all current items (completely different dir)
    oldFiles.clear();
    emit clearItems(); 
  } 
  currentDir = dir; //save this for later
  //clean up the watcher first
  QStringList watched; watched << watcher->files() << watcher->directories();
  if(!watched.isEmpty()){ watcher->removePaths(watched); }
  QStringList old = oldFiles; //copy this over for the moment (both lists will change in a moment)
  oldFiles.clear(); //get ready for re-creating this list
  // read the given directory
  QDir directory(dir);
  if(directory.exists()){
    QStringList files;
    if(showHidden){ files = directory.entryList( QDir::Dirs | QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot, QDir::NoSort); }
    else{ files = directory.entryList( QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::NoSort); }
    emit itemsLoading(files.length());
    //QCoreApplication::processEvents();
    //QCoreApplication::sendPostedEvents();
    for(int i=0; i<files.length(); i++){
      watcher->addPath(directory.absoluteFilePath(files[i]));
      //qDebug() << "Future Starting:" << files[i];
      QString path = directory.absoluteFilePath(files[i]);
      if(old.contains(path)){ old.removeAll(path); }
      oldFiles << path; //add to list for next time
      if(imageFormats.contains(path.section(".",-1).toLower()) || path.endsWith(".desktop")){
        QtConcurrent::run(this, &Browser::loadItem, path );
        //QCoreApplication::sendPostedEvents();
      }else{
        //No special icon loading - do it in-line here
        loadItem(path);
      }
    }
    watcher->addPath(directory.absolutePath());
    if(!old.isEmpty()){
      old.removeAll(directory.absolutePath());
      for(int i=0; i<old.length(); i++){
        emit itemRemoved(old[i]);
      }
    }
  }else{
    emit itemsLoading(0); //nothing to load
  }
}
