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
  updateTimer = new QTimer(this);
    updateTimer->setInterval(500);
    updateTimer->setSingleShot(true);
  connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateRequested()) );

  showHidden = false;
  showThumbs = false;
  imageFormats = LUtils::imageExtensions(false); //lowercase suffixes
  //connect(surface, SIGNAL(frameReceived(QImage)), this, SLOT(captureFrame(QImage)));
  //connect(player, &QMediaPlayer::mediaStatusChanged, this, [&]{ stopVideo(player, player->mediaStatus()); });
  connect(this, SIGNAL(threadDone(const QString, const QImage)), this, SLOT(futureFinished(const QString, const QImage))); //will always be between different threads
}

Browser::~Browser(){
  //watcher->deleteLater();
}

QString Browser::currentDirectory(){ return currentDir; }

void Browser::showHiddenFiles(bool show){
  if(show !=showHidden){
    showHidden = show;
    lastcheck = QDateTime(); //reset this timestamp - need to reload all
    if(!currentDir.isEmpty()){ QTimer::singleShot(0, this, SLOT(loadDirectory()) ); }
  }
}
bool Browser::showingHiddenFiles(){
  return showHidden;
}

void Browser::showThumbnails(bool show){
  if(show != showThumbs){
    showThumbs = show;
    lastcheck = QDateTime(); //reset this timestamp - need to reload all
    if(!currentDir.isEmpty()){ QTimer::singleShot(0, this, SLOT(loadDirectory()) ); }
  }
}

bool Browser::showingThumbnails(){
  return showThumbs;
}

//   PRIVATE
void Browser::loadItem(const QString info, Browser *obj){
  //qDebug() << "Load Thumbnail in separate thread:" << info;
  QImage pix(info);
  //Make sure we only keep a small version in memory - not the full thing
  if(pix.width() > 256 || pix.height() > 256 ){
    pix = pix.scaled(256,256, Qt::KeepAspectRatio, Qt::FastTransformation);
  }
  obj->emit threadDone(info, pix);
  //qDebug() << " - Done:" << info;
 /* QImage* pix = new QImage();
  qDebug() << "Load Thumbnail";
  QFile file(info);
  if(file.open(QIODevice::ReadOnly)){
    QByteArray bytes = file.readAll();
    pix->loadFromData(bytes);
    if(pix->width() > 256 || pix->height() > 256 ){
      *pix = pix->scaled(256,256, Qt::KeepAspectRatio, Qt::FastTransformation);
    }
    file.close();
  }
  qDebug() << "Loading Thumbnail Done:" << info;
  obj->emit threadDone(info, pix);*/
}

QIcon* Browser::loadIcon(QString icon){
  hashMutex.lock();
  if(!mimeIcons.contains(icon)){
    mimeIcons.insert(icon, LXDG::findIcon(icon, "unknown"));
  }
  QIcon *tmp = &mimeIcons[icon];
  hashMutex.unlock();
  return tmp;
}


// PRIVATE SLOTS
void Browser::fileChanged(QString file){
  //qDebug() << "Got File Changed:" << file;
  if(file.section("/",0,-2) == currentDir){
    if(QFile::exists(file) ){
      //qDebug() << "File exists:" << file;
      updateList << file;
      if(!updateTimer->isActive()){ updateTimer->start(); }
      //QtConcurrent::run(this, &Browser::loadItem, file, this); //file modified but not removed

    }else if(oldFiles.contains(file) ){
      //qDebug() << "OldFile removed: " << file;
      oldFiles.removeAll(file);
      emit itemRemoved(file);
      updateList << currentDir;
      if(!updateTimer->isActive()){ updateTimer->start(); }
    }else{
      //qDebug() << "File does not exist:" << file;
      updateList << currentDir;
      if(!updateTimer->isActive()){ updateTimer->start(); }
    }
  }
}

void Browser::dirChanged(QString dir){
  //qDebug() << "Got Dir Changed:" << dir;
  updateList << dir;
  if(!updateTimer->isActive()){ updateTimer->start(); }
  //if(dir==currentDir){ QTimer::singleShot(10, this, SLOT(loadDirectory()) ); }
  //else if(dir.startsWith(currentDir)){ QtConcurrent::run(this, &Browser::loadItem, dir, this ); }
}

void Browser::futureFinished(QString name, const QImage icon){
  //Note: this will be called once for every item that loads
    //Haven't added the extra files in a directory fix, but that should be easy to do
    //Try to load a file with multiple videos and lots of other stuff before any other directory. It crashes for some reason
    //qDebug() << "Finished:" << name;
    QIcon *ico = new QIcon();
    LFileInfo *info = new LFileInfo(name);
    if(!icon.isNull() && showThumbs){
      QPixmap pix = QPixmap::fromImage(icon);
      ico->addPixmap(pix);
    /*}else if(info->isVideo() && showThumbs) {
      if(videoImages.find(name) == videoImages.end()) {
        LVideoLabel *mediaLabel = new LVideoLabel(name);
        while(mediaLabel->pixmap()->isNull()) { QCoreApplication::processEvents(QEventLoop::AllEvents, 50); }
        ico->addPixmap(*(mediaLabel->pixmap()));
        videoImages.insert(name, *mediaLabel->pixmap());
        delete mediaLabel;
      }else{
        ico->addPixmap(videoImages[name]);
      }*/
    }else{
      //Standard Icon based on file type
      ico = loadIcon(info->iconfile());
    }
    this->emit itemDataAvailable( ico, info);
    //qDebug() << " -- done:" << name;
}

void Browser::updateRequested(){
  //Clear the cache list ASAP
  QStringList list = updateList;
  updateList.clear();
  //qDebug() << "Got Update List:" << list;
  list.removeDuplicates();
  //Now look to see if an all-dir update is needed
  if(list.contains(currentDir) || list.isEmpty()){ QTimer::singleShot(10, this, SLOT(loadDirectory()) ); }
  else{
    //individual file updates
    for(int i=0; i<list.length(); i++){
      if(QFile::exists(list[i])){ QtConcurrent::run(this, &Browser::loadItem, list[i], this); }
    }
  }
}

// PUBLIC SLOTS
void Browser::loadDirectory(QString dir, bool force){
  if(force){ lastcheck = QDateTime(); } //reset check time to force reloads
  if(dir.isEmpty()){ dir = currentDir; } //reload current directory
  if(dir.isEmpty()){ return; } //nothing to do - nothing previously loaded
  updateList.clear();
  //qDebug() << "Load Directory" << dir;
  if(dir.endsWith("/")){ dir.chop(1); }
  bool dirupdate = true;
  if(currentDir != dir){ //let the main widget know to clear all current items (completely different dir)
    //qDebug() << " - different Directory";
    //videoImages.clear();
    oldFiles.clear();
    lastcheck = QDateTime(); //null time
    emit clearItems();
    dirupdate = false;
    QCoreApplication::processEvents();
  }
  currentDir = dir; //save this for later
  QDateTime now = QDateTime::currentDateTime();
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
    if(dirupdate || old.isEmpty()){ emit itemsLoading(files.length()); }
    //qDebug() << "Files Found:" << files.length() << currentDir;
    for(int i=0; i<files.length(); i++){
      watcher->addPath(directory.absoluteFilePath(files[i]));
      QString path = directory.absoluteFilePath(files[i]);
      oldFiles << path; //add to list for next time
      bool reloaditem = !dirupdate || lastcheck.isNull() || (QFileInfo(path).lastModified() > lastcheck || QFileInfo(path).created() > lastcheck);
      //if(dirupdate){ qDebug() << "Reload Item:" << reloaditem << path.section("/",-1); }
      //reloaditem = true;
      if(old.contains(path)){ old.removeAll(path); } //still in existance
      else{ reloaditem = true; } //new file - make sure this gets loaded
      //qDebug() << "Item:" << files[i] << reloaditem;
      //if(showThumbs && imageFormats.contains(path.section(".",-1).toLower())){
      //qDebug() << "Future Starting:" << path;
      if(reloaditem){
	if(imageFormats.contains(path.section(".",-1).toLower()) ){
          QtConcurrent::run(this, &Browser::loadItem, path, this);
        }else{
          //No special icon loading - just skip the loadItem function
          futureFinished(path, QImage());
        }
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
  lastcheck = now; // save this for later
}
