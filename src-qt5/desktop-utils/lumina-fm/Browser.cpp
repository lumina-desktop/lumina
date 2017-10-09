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
  videoFormats = LUtils::videoExtensions(); //lowercase suffixes
  //connect(surface, SIGNAL(frameReceived(QImage)), this, SLOT(captureFrame(QImage)));
  //connect(player, &QMediaPlayer::mediaStatusChanged, this, [&]{ stopVideo(player, player->mediaStatus()); });
  connect(this, SIGNAL(threadDone(QString, QImage)), this, SLOT(futureFinished(QString, QImage))); //will always be between different threads
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
  QImage pix;
  if(imageFormats.contains(info.section(".",-1).toLower()) ){
    QFile file(info);
    if(file.open(QIODevice::ReadOnly)){
      QByteArray bytes = file.readAll();
      file.close();
      pix.loadFromData(bytes);
      if(pix.width() > 256 || pix.height() > 256 ){
        pix = pix.scaled(256,256, Qt::KeepAspectRatio);
      }
    }
  }else if(videoFormats.contains(info.section(".",-1).toLower()) ){
    videoList.push_back(info);
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
    if(QFile::exists(file) ){ QtConcurrent::run(this, &Browser::loadItem, file, this); } //file modified but not removed
    else if(oldFiles.contains(file) ){
      oldFiles.removeAll(file);
      emit itemRemoved(file);
    }
  }//else if(file==currentDir){ QTimer::singleShot(0, this, SLOT(loadDirectory()) ); }
}

void Browser::dirChanged(QString dir){
  //qDebug() << "Got Dir Changed:" << dir;
  if(dir==currentDir){ QTimer::singleShot(10, this, SLOT(loadDirectory()) ); }
  else if(dir.startsWith(currentDir)){ QtConcurrent::run(this, &Browser::loadItem, dir, this ); }
}

void Browser::stopVideo(QMediaPlayer *player, QMediaPlayer::MediaStatus status) {
  //qDebug() << status;
  if(status == QMediaPlayer::BufferedMedia) {
    //qDebug() << "stoppingVideo" << player << player->currentMedia().canonicalUrl();
    player->setPosition(player->duration() / 2);
    player->pause();
  }
}

void Browser::captureFrame(QPixmap pix, QIcon *ico) {
  static int received = 0;
  //qDebug() << "grabbing frame" << received+1;
  *ico = pix/*.scaledToHeight(64)*/;
  if(++received == videoList.size()) {
    emit frameChanged();
    received = 0;
  }
}

void Browser::futureFinished(QString name, QImage icon){
  //Note: this will be called once for every item that loads
    QIcon *ico = new QIcon();
    LFileInfo *info = new LFileInfo(name);
    if(!icon.isNull() && showThumbs){
      QPixmap pix = QPixmap::fromImage(icon);
      ico->addPixmap(pix);
    }
    if(ico->isNull()){
      if(videoFormats.contains(name.section(".",-1).toLower())) {
        QElapsedTimer loadingTime;
        //qDebug() << videoList;
        //videoList.add(name);
        //qDebug() << "Loading Video for" << name;
        //qDebug() << "VIDEO" << info;
        QMediaPlayer *player = new QMediaPlayer(0, QMediaPlayer::VideoSurface);
        //qDebug() << " - created player" << player;
        LVideoSurface *surface = new LVideoSurface();
        //qDebug() << " - Create objects";
        connect(surface, &LVideoSurface::frameReceived, this, [&] (QPixmap pix) { captureFrame(pix, ico); });
        connect(player, &QMediaPlayer::mediaStatusChanged, this, [&]{ stopVideo(player, player->mediaStatus()); });
        player->setVideoOutput(surface);
        player->setMuted(true);
        QMediaResource video = QMediaResource(QUrl("file://"+info->absoluteFilePath()));
        video.setResolution(QSize(64, 64));
        player->setMedia(video);
        //player->setMedia(QUrl("file://"+info->absoluteFilePath()));
        player->play();
        player->pause();

        //qDebug() << "Started loop";
        loadingTime.start();
        QTimer timeout;
        timeout.setSingleShot(true);
        timeout.setInterval(5000);
        QEventLoop loop;
        connect(this, SIGNAL(frameChanged()), &loop, SLOT(quit()), Qt::DirectConnection);
        connect(&timeout, SIGNAL(timeout()), &loop, SLOT(quit()));
        loop.exec();
        //qDebug() << "Exited loop";

        qDebug() << loadingTime.elapsed();
        delete player;
        delete surface;
      }else {
        ico = loadIcon(info->iconfile());
      }
    }
    this->emit itemDataAvailable( *ico, info);
    //qDebug() << " -- done:" << name;
}

// PUBLIC SLOTS
void Browser::loadDirectory(QString dir, bool force){
  if(force){ lastcheck = QDateTime(); } //reset check time to force reloads
  videoList.clear();
  if(dir.isEmpty()){ dir = currentDir; } //reload current directory
  if(dir.isEmpty()){ return; } //nothing to do - nothing previously loaded
  //qDebug() << "Load Directory" << dir;
  bool dirupdate = true;
  if(currentDir != dir){ //let the main widget know to clear all current items (completely different dir)
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
    if(dirupdate){ emit itemsLoading(files.length()); }
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
