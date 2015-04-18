#include "BackgroundWorker.h"

#include <LuminaXDG.h>
#include <QMediaServiceSupportedFormatsInterface>
#include <QImageReader>
#include <QDir>
#include <QFileInfo>

BackgroundWorker::BackgroundWorker() : QObject(){

}

BackgroundWorker::~BackgroundWorker(){
	
}

void BackgroundWorker::startDirChecks(QString path){
  QDir dir(path);
  //Make sure to remove any symlinks or redundency in the path
  if(dir.canonicalPath()!=path){ path = dir.canonicalPath(); dir.cd(path); }
  qDebug() << "Starting Dir Checks:" << path;
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
  qDebug() << " - done with audio/multimedia checks";
  //Now check for ZFS snapshots of the directory
  if(!QFileInfo(path).isWritable() ){ return; } //skip ZFS checks if can't restore to this dir
  cdir = path;
  QStringList snapDirs; 
  QString baseSnapDir;
  bool found = false;
  qDebug() << " - start searching for base snapshot directory";
  if(cdir == path && QFile::exists(csnapdir) ){ 
     //no need to re-search for it - just found it recently
    baseSnapDir= csnapdir; found=true;
  }else{
    while(dir.canonicalPath()!="/" && !found){
      if(dir.exists(".zfs/snapshot")){ 
        baseSnapDir = dir.canonicalPath()+"/.zfs/snapshot";
	found = true;
      }else{ dir.cdUp(); }
    }
  }
  qDebug() << " - done with base snapshot directory";
  cdir = path; csnapdir = baseSnapDir;
  //Now find the snapshots that contain this directory and save them
  if(found){
    qDebug() << " - start fetching snapshots";
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
    qDebug() << " - done fetching snapshots";
    snapDirs.sort();
    //Sort the snapshots by time (newest last) and format them
    for(int i=0; i<snapDirs.length(); i++){
      snapDirs[i] = dir.absolutePath()+"/"+snapDirs[i].section("::::",1,50)+"/"+reldir;
    }
    if(!snapDirs.isEmpty()){ emit SnapshotsAvailable(baseSnapDir, snapDirs); }
    qDebug() << "Found snapshots";
  }
}


void BackgroundWorker::createStatusBarMsg(QFileInfoList fileList, QString path, QString messageFolders, QString messageFiles){

  //collect some statistics of dir and display them in statusbar
  //Get the total size of the items
  double totalSizes = 0;
  int numberFolders = 0;
  int numberFiles = 0;
  for(int i=0; i<fileList.length(); i++){
    if(!fileList[i].isDir()){
      numberFiles++;
      totalSizes += fileList[i].size(); //in Bytes
    }
    else { numberFolders++; }
  }
  //Convert the size into display units
  static QStringList units = QStringList() << tr("B") << tr("KB") << tr("MB") << tr("GB") << tr("TB");
  int cunit = 0;
  while(cunit < units.length() && totalSizes > 1024){
    cunit++;
    totalSizes = totalSizes/1024;
  }
  //Assemble the message
  QString msgStatusBar = QString(tr("%1: %2 / %3: %4")).arg(messageFolders).arg(numberFolders).arg(messageFiles).arg(numberFiles);

  if(totalSizes > 0){
    totalSizes = qRound(totalSizes*100)/100.0; //round to 2 decimel places
    msgStatusBar += "    "+QString(tr("Total size: %1 %2")).arg(QString::number(totalSizes), units[cunit]);
  }
  //If a path given, get the total capacity of it (percantage)
  if (!path.isEmpty()) { //path could be empty when fileList is based on user's selection
	  QString capacity = LOS::FileSystemCapacity(path) ;
      if (msgStatusBar.isEmpty()) msgStatusBar += QString(tr("Capacity: %1")).arg(capacity);
      else msgStatusBar += "   "+QString(tr("Capacity: %1")).arg(capacity);
  }
  //Emit the signal to show this on the UI
  if (!msgStatusBar.isEmpty()){
    emit Si_DisplayStatusBar(msgStatusBar);
  }
}
