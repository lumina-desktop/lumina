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
  connect(this, SIGNAL(threadDone(QString, const QImage*)), this, SLOT(futureFinished(QString, const QImage*))); //will always be between different threads
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
void Browser::loadItem(QString info, Browser *obj){
  QImage* pix = 0; //this needs to return 0 if a standard icon is to be used
  if(imageFormats.contains(info.section(".",-1).toLower()) ){
    QFile file(info);
    if(file.open(QIODevice::ReadOnly)){
      QByteArray bytes = file.readAll();
      file.close();
      QImage *tmppix = new QImage();
      tmppix->loadFromData(bytes);
      if(tmppix->width() > 256 || tmppix->height() > 256 ){
        *pix = tmppix->scaled(256,256, Qt::KeepAspectRatio, Qt::FastTransformation);
      }
      delete tmppix;
    }
  }
  //qDebug() << " - done with item:" << info;
  obj->emit threadDone(info, pix);
}

QIcon* Browser::loadIcon(QString icon){
  if(!mimeIcons.contains(icon)){
    mimeIcons.insert(icon, LXDG::findIcon(icon, "unknown"));
  }
  return &mimeIcons[icon];
}


// PRIVATE SLOTS
void Browser::fileChanged(QString file){
  //qDebug() << "Got File Changed:" << file;
  if(file.section("/",0,-2) == currentDir){
    if(QFile::exists(file) ){
      updateList << file;
      if(!updateTimer->isActive()){ updateTimer->start(); }
      //QtConcurrent::run(this, &Browser::loadItem, file, this); //file modified but not removed

    }else if(oldFiles.contains(file) ){
      oldFiles.removeAll(file);
      emit itemRemoved(file);
    }
  }//else if(file==currentDir){ QTimer::singleShot(0, this, SLOT(loadDirectory()) ); }
}

void Browser::dirChanged(QString dir){
  //qDebug() << "Got Dir Changed:" << dir;
  updateList << dir;
  if(!updateTimer->isActive()){ updateTimer->start(); }
  //if(dir==currentDir){ QTimer::singleShot(10, this, SLOT(loadDirectory()) ); }
  //else if(dir.startsWith(currentDir)){ QtConcurrent::run(this, &Browser::loadItem, dir, this ); }
}

void Browser::futureFinished(QString name, const QImage* icon){
  //Note: this will be called once for every item that loads
    //Haven't added the extra files in a directory fix, but that should be easy to do
    //Try to load a file with multiple videos and lots of other stuff before any other directory. It crashes for some reason
    //qDebug() << "Finished:" << name;
    QIcon *ico = new QIcon();
    LFileInfo *info = new LFileInfo(name);
    if(icon != nullptr && showThumbs){
      QPixmap pix = QPixmap::fromImage(*icon);
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
      ico = loadIcon(info->iconfile());
    }
    this->emit itemDataAvailable( ico, info);
    // We are done with processing received image (copied to pixmap above) so now clean it up
    delete icon;
    icon = nullptr;
    //qDebug() << " -- done:" << name;
}

void Browser::updateRequested(){
  //Clear the cache list ASAP
  QStringList list = updateList;
  updateList.clear();
  list.removeDuplicates();
  //Now look to see if an all-dir update is needed
  if(list.contains(currentDir)){ QTimer::singleShot(10, this, SLOT(loadDirectory()) ); }
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
  bool dirupdate = true;
  if(currentDir != dir){ //let the main widget know to clear all current items (completely different dir)
    //videoImages.clear();
    oldFiles.clear();
    lastcheck = QDateTime(); //null time
    emit clearItems();
    dirupdate = false;
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
    //qDebug() << "Files Found:" << files.length();
    for(int i=0; i<files.length(); i++){
      watcher->addPath(directory.absoluteFilePath(files[i]));
      QString path = directory.absoluteFilePath(files[i]);
      oldFiles << path; //add to list for next time
      bool reloaditem = !dirupdate || lastcheck.isNull() || (QFileInfo(path).lastModified() > lastcheck || QFileInfo(path).created() > lastcheck);
      //if(dirupdate){ qDebug() << "Reload Item:" << reloaditem << path.section("/",-1); }
      //reloaditem = true;
      if(old.contains(path)){ old.removeAll(path); } //still in existance
      //if(showThumbs && imageFormats.contains(path.section(".",-1).toLower())){
      //qDebug() << "Future Starting:" << files[i];
      if(reloaditem){ QtConcurrent::run(this, &Browser::loadItem, path, this); }
      /*}else{
        //No special icon loading - just skip the file read step
        futureFinished(path, QImage()); //loadItem(path, this);
      }*/
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
