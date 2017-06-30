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
  showThumbs = false;
  imageFormats = LUtils::imageExtensions(false); //lowercase suffixes
  connect(this, SIGNAL(threadDone(QString, QImage)), this, SLOT(futureFinished(QString, QImage))); //will always be between different threads
}

Browser::~Browser(){
  //watcher->deleteLater();
}

QString Browser::currentDirectory(){ return currentDir; }

void Browser::showHiddenFiles(bool show){
  if(show !=showHidden){
    showHidden = show;
    if(!currentDir.isEmpty()){ QTimer::singleShot(0, this, SLOT(loadDirectory()) ); }
  }
}
bool Browser::showingHiddenFiles(){
  return showHidden;
}

void Browser::showThumbnails(bool show){
  if(show != showThumbs){
    showThumbs = show;
    if(!currentDir.isEmpty()){ QTimer::singleShot(0, this, SLOT(loadDirectory()) ); }
  }
}

bool Browser::showingThumbnails(){
  return showThumbs;
}

//   PRIVATE
void Browser::loadItem(QString info, Browser *obj){
  QImage pix;
  if(imageFormats.contains(info.section(".",-1).toLower()) ){
    QFile file(info);
    if(file.open(QIODevice::ReadOnly)){
      QByteArray bytes = file.readAll();
      file.close();
      pix.loadFromData(bytes);
      if(pix.width() > 256 || pix.height() > 256 ){
        pix = pix.scaled(256,256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
      }
    }
  }

  //qDebug() << " - done with item:" << info;
  obj->emit threadDone(info, pix);
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
    if(QFile::exists(file) ){ QtConcurrent::run(this, &Browser::loadItem, file, this); } //file modified but not removed
    else{ QTimer::singleShot(0, this, SLOT(loadDirectory()) ); } //file removed - need to update entire dir
  }else if(file==currentDir){ QTimer::singleShot(0, this, SLOT(loadDirectory()) ); }
}

void Browser::dirChanged(QString dir){

  if(dir==currentDir){ QTimer::singleShot(500, this, SLOT(loadDirectory()) ); }
  else if(dir.startsWith(currentDir)){ QtConcurrent::run(this, &Browser::loadItem, dir, this ); }
}

void Browser::futureFinished(QString name, QImage icon){
  //Note: this will be called once for every item that loads
     QIcon ico;
     //LFileInfo info(name);
     LFileInfo *info = new LFileInfo(name);
      if(!icon.isNull()){
        //qDebug() << " -- Data:";
        QPixmap pix = QPixmap::fromImage(icon);
        ico.addPixmap(pix);
       }else if(info->isDir()){
        //qDebug() << " -- Folder:";
        ico = loadIcon("folder");
      }
      if(ico.isNull()){
	//qDebug() << " -- MimeType:" << info.fileName() << info.mimetype();
        ico = loadIcon(info->iconfile());
      }
      this->emit itemDataAvailable( ico, info);
     //qDebug() << " -- done:" << name;
}

// PUBLIC SLOTS
void Browser::loadDirectory(QString dir){
  if(dir.isEmpty()){ dir = currentDir; } //reload current directory
  if(dir.isEmpty()){ return; } //nothing to do - nothing previously loaded
  //qDebug() << "Load Directory" << dir;
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
    for(int i=0; i<files.length(); i++){
      watcher->addPath(directory.absoluteFilePath(files[i]));
      //qDebug() << "Future Starting:" << files[i];
      QString path = directory.absoluteFilePath(files[i]);
      if(old.contains(path)){ old.removeAll(path); }
      oldFiles << path; //add to list for next time
      if(showThumbs && imageFormats.contains(path.section(".",-1).toLower())){
        QtConcurrent::run(this, &Browser::loadItem, path, this);
      }else{
        //No special icon loading - just skip the file read step
        futureFinished(path, QImage()); //loadItem(path, this);
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
