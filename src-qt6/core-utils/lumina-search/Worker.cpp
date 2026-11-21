#include "Worker.h"

#include <QTimer>
#include <LuminaXDG.h>
#include <LUtils.h>

Worker::Worker(QObject *parent) : QObject(parent){
  //(the application list used to be parsed here - every .desktop file, on the GUI thread, before the window
  // was even shown, and also for file-only searches: it is now loaded on the first app search, in the worker)
  appsLoaded = false;
  stopsearch = false;
}

void Worker::loadApps(){
  //Get the list of all applications and save them in an easily-searchable form
  appsLoaded = true;
  XDGDesktopList APPS;
  APPS.updateList();
  QList<XDGDesktop*> apps = APPS.apps(false,false);
  for(int i=0; i<apps.length(); i++){
    applist << ":::1:::"+apps[i]->name+":::2:::"+apps[i]->genericName+":::3:::"+apps[i]->comment+":::4:::"+apps[i]->filePath;
  }
}

void Worker::setStartDir(QString dir){
  QMutexLocker lock(&dirMutex);
  m_startDir = dir;
}

QString Worker::startDir() const{
  QMutexLocker lock(&dirMutex);
  return m_startDir;
}

void Worker::setSkipDirs(QStringList dirs){
  QMutexLocker lock(&dirMutex);
  m_skipDirs = dirs;
}

QStringList Worker::skipDirs() const{
  QMutexLocker lock(&dirMutex);
  return m_skipDirs;
}

Worker::~Worker(){
  stopsearch = true;
}

void Worker::StartSearch(QString term, bool isApp){
  sterm=term; sapp=isApp;
  if(stopsearch){
    //Need to stop the current search first - give it a moment first
    QTimer::singleShot(100,this, SLOT(beginsearch()) );	  
  }else{
    //Start immediately
    QTimer::singleShot(0,this, SLOT(beginsearch()) );
  }
}

void Worker::StopSearch(){
  stopsearch = true;	
}

bool Worker::searchDir(QString dirpath){
  //This is a recursive search algorithm for scanning a directory
  QDir dir(dirpath);
  //First look for files that match the search term	
  if(stopsearch){ return true; }
  emit SearchUpdate( QString(tr("Searching: %1")).arg(dirpath.replace(QDir::homePath(),"~")) );
  QStringList tmp;
  if(sterm.startsWith(".")){ tmp = dir.entryList(QStringList(sterm), QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden, QDir::Name); }
  else{ tmp = dir.entryList(QStringList(sterm), QDir::AllEntries | QDir::NoDotAndDotDot , QDir::Name); }
  for(int i=0; i<tmp.length(); i++){
    if(stopsearch){ return true; }
    emit FoundItem( dir.absoluteFilePath(tmp[i]) );
  }
  if(stopsearch){ return true; }
  //Now recursively scan the sub directories (not through symlinks: a link back up the tree recursed until the
  // stack overflowed, and links into other trees re-scanned them)
  tmp = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::Name);
  for(int i=0; i<tmp.length(); i++){
    if(stopsearch){ return true; }
    if( curSkipDirs.contains(dir.absoluteFilePath(tmp[i])) || tmp[i]=="proc" ){ continue; } //this dir is skipped
       //Special case - skip the "proc" directory heirarchy (highly-recursive layout for *every* process which is running)
    if( searchDir(dir.absoluteFilePath(tmp[i])) ){ return true; }
  }
  return false;
}

void Worker::beginsearch(){
  stopsearch = false; //just starting search - always set this to false initially
  emit SearchUpdate( QString(tr("Starting Search: %1")).arg(sterm) );
  //Now Perform the search
  if(sapp){
    if(!appsLoaded){ loadApps(); }
    //First try to match based on the name
    QStringList tmp = applist.filter(":::1:::"+sterm, Qt::CaseInsensitive);
    tmp.sort();
    for(int i=0; i<tmp.length(); i++){
      if(stopsearch){ return; }
      emit FoundItem( tmp[i].section(":::4:::",1,1) );
    }
    //Check if this is a binary name
    if(stopsearch){ return; }
    if(LUtils::isValidBinary(sterm)){
      emit FoundItem(sterm);
      return;
    }
    //If items found, go ahead and stop now
    if(stopsearch){ return; }
    if(tmp.length()<1){
      //Now try to match based on the generic name
      tmp = applist.filter(":::2:::"+sterm, Qt::CaseInsensitive);
      tmp.sort();
      for(int i=0; i<tmp.length(); i++){
        if(stopsearch){ return; }
        emit FoundItem( tmp[i].section(":::4:::",1,1) );
      }
    }
    //If items found, go ahead and stop now
    if(stopsearch){ return; }
    if(tmp.length()<1){
      //Now try to match based on anything (name/genericname/comment)
      tmp = applist.filter(sterm, Qt::CaseInsensitive);
      tmp.sort();
      for(int i=0; i<tmp.length(); i++){
        if(stopsearch){ return; }
        emit FoundItem( tmp[i].section(":::4:::",1,1) );
      }
    }
    
  }else{
    //Search through the user's home directory and look for a file/dir starting with that term
    if(!sterm.contains("*")){
      sterm.prepend("*"); sterm.append("*"); //make sure it is a search glob pattern
    }
    QString start;
    { QMutexLocker lock(&dirMutex); start = m_startDir; curSkipDirs = m_skipDirs; }
    if(start.isEmpty()){ start = QDir::homePath(); }
    searchDir(start);
    
  }
  emit SearchUpdate( tr("Search Finished") );
  emit SearchDone();
}
