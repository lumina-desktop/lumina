#include "BackgroundWorker.h"

#include <LuminaXDG.h>
#include <QMediaServiceSupportedFormatsInterface>
#include <QImageReader>

BackgroundWorker::BackgroundWorker() : QObject(){

}

BackgroundWorker::~BackgroundWorker(){
	
}

void BackgroundWorker::startDirChecks(QString path){
  QDir dir(path);
  //First check for image files
  if(imgFilter.isEmpty()){
    //Initial Run - load supported image extensions
    QList<QByteArray> fmt = QImageReader::supportedImageFormats();
    for(int i=0; i<fmt.length(); i++){ imgFilter << "*."+QString(fmt[i]).toLower(); }
    qDebug() << "Supported Image Formats:" << imgFilter;
  }
  QStringList pics = dir.entryList(imgFilter, QDir::Files | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase);	
  if(!pics.isEmpty() && !imgFilter.isEmpty()){ emit ImagesAvailable(pics); }
	  
  //Now check for multimedia files
  if(multiFilter.isEmpty()){
    //Initial Run - load supported multimedia extensions 
    // (Auto-detection of supported types broken in Qt5 - just use everythings for now)
    multiFilter = LXDG::findAVFileExtensions();
    multiFilter.removeDuplicates();
    qDebug() << "Supported Multimedia Formats:" << multiFilter;
  }
  QStringList files = dir.entryList(multiFilter, QDir::Files | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase);
  if(!files.isEmpty() && !multiFilter.isEmpty()){ emit MultimediaAvailable(files); }
  
  //Now check for ZFS snapshots of the directory
  if(!QFileInfo(path).isWritable() ){ return; } //skip ZFS checks if can't restore to this dir
  cdir = path;
  QStringList snapDirs; 
  QString baseSnapDir;
  bool found = false;
  if(cdir == path && QFile::exists(csnapdir) ){ 
     //no need to re-search for it - just found it recently
    baseSnapDir= csnapdir; found=true;
  }else{
    while(dir.absolutePath()!="/" && !found){
      if(dir.exists(".zfs/snapshot")){ 
        baseSnapDir = dir.canonicalPath()+"/.zfs/snapshot";
	found = true;
      }else{ dir.cdUp(); }
    }
  }
  cdir = path; csnapdir = baseSnapDir;
  //Now find the snapshots that contain this directory and save them
  if(found){
    QString reldir = path;
	  reldir.remove(baseSnapDir.section("/.zfs/snapshot",0,0)); //convert to a relative path
    dir.cd(baseSnapDir);
    snapDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time);
    //Check that the current directory exists in each snapshot
    for(int i=0; i<snapDirs.length(); i++){
      if( !dir.exists(snapDirs[i]+"/"+reldir) ){
        snapDirs.removeAt(i);
	i--;
      }else{
	snapDirs[i] = QFileInfo(dir, snapDirs[i]+"/"+reldir).created().toString("yyyyMMddhhmmsszzz")+"::::"+snapDirs[i];
      }
    }
    snapDirs.sort();
    //Sort the snapshots by time (newest last) and format them
    for(int i=0; i<snapDirs.length(); i++){
      snapDirs[i] = dir.absolutePath()+"/"+snapDirs[i].section("::::",1,50)+"/"+reldir;
    }
    if(!snapDirs.isEmpty()){ emit SnapshotsAvailable(baseSnapDir, snapDirs); }
    //qDebug() << "Found snapshots:" << snapDirs;
  }
}