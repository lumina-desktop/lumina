//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "DesktopSettings.h"
#include <QFile>
#include <QDir>
#include <QDebug>

#include <unistd.h>
#include <pwd.h>
#include <grp.h>

#define FILEPREFIX QString("/lumina-desktop/desktop/")

// === PUBLIC ===
DesktopSettings::DesktopSettings(QObject *parent) : QObject(parent){
  qRegisterMetaType< DesktopSettings::File >("DesktopSettings::File");
  watcher = 0;
  runmode = DesktopSettings::UserFull;
}

DesktopSettings::~DesktopSettings(){
  if(!files.isEmpty()){ stop(); }
}

DesktopSettings* DesktopSettings::instance(){
  static DesktopSettings *set = 0;
  if(set==0){
    //First-time init
    set = new DesktopSettings();
    set->start();
  }
  return set;
}

//Start/stop routines
void DesktopSettings::start(){
  files.clear(); settings.clear(); //clear the internal hashes (just in case)
  if(watcher==0){
    watcher = new QFileSystemWatcher(this);
    connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(fileChanged(QString)) );
    connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(dirChanged(QString)) );
  }
  parseSystemSettings(); //set the runmode appropriately
  locateFiles(); //

}

void DesktopSettings::stop(){
  QStringList watch; watch << watcher->files() << watcher->directories();
  if(!watch.isEmpty()){ watcher->removePaths(watch); }
  files.clear(); //clear the internal hash
  settings.clear();
}

//Main Read/Write functions
QList< DesktopSettings::File > DesktopSettings::writableFiles(){
  QList< DesktopSettings::File > tmp;
  if(runmode!=DesktopSettings::SystemFull){ tmp = filesForRunMode(runmode); }
  return tmp;
}

QVariant DesktopSettings::value(DesktopSettings::File file, QString variable, QVariant defaultvalue){
  if(!files.contains(file)){ return defaultvalue; }
  for(int i=0; i<files[file].length(); i++){
    //qDebug() << "Look for Settings value:" << variable << files[file];
    if( settings.contains(files[file][i])){ //make sure this file is in the settings hash
      if(settings[files[file][i]]->contains(variable)){ //if this file does not have the variable - go to the next one
        //qDebug() << " - Found Setting in File:" << files[file][i];
        return settings[files[file][i]]->value(variable, defaultvalue);
      }
    }
  }
  return defaultvalue; //none of the files contain the variable - return the default
}

bool DesktopSettings::setValue(DesktopSettings::File file, QString variable, QVariant value){
  if(!files.contains(file)){ return false; }
  for(int i=0; i<files[file].length(); i++){
    if( settings.contains(files[file][i])){ //make sure this file is in the settings hash
      if(settings[files[file][i]]->isWritable() ){ //Check write permissions
        settings[files[file][i]]->setValue(variable, value);
	settings[files[file][i]]->sync(); //make sure this is synced to disk ASAP
        return true;
      }
    }
  }
  return false;
}

QStringList DesktopSettings::keys(DesktopSettings::File file){
  if(!files.contains(file)){ return QStringList(); }
  QStringList keyList;
  for(int i=0; i<files[file].length(); i++){
    if( settings.contains(files[file][i])){ //make sure this file is in the settings hash
      keyList << settings[files[file][i]]->allKeys();
    }
  }
  keyList.removeDuplicates();
  return keyList;
}

//Information functions
QStringList DesktopSettings::filePaths(DesktopSettings::File file){
  if(!files.contains(file)){ return QStringList(); }
  return files.value(file);
}

//=== PRIVATE ===
void DesktopSettings::parseSystemSettings(){
  //run at start - determine the RunMode for this user/session
  //This will also load the DesktopSettings::System file into the hashes
  runmode = DesktopSettings::UserFull; //default value unless otherwise specified
  QStringList dirs;
  dirs << QString(getenv("XDG_CONFIG_DIRS")).split(":") << QString(getenv("XDG_DATA_DIRS")).split(":");
  for(int i=0; i<dirs.length(); i++){
    if(dirs[i].endsWith("/xdg")){ dirs[i] = dirs[i].section("/",0,-2); } //Chop off the xdg subdir - need the generic configuration directory
    QString path = dirs[i]+rel_path(DesktopSettings::System);
    if(!QFile::exists(path)){ continue; }
    //Load the system file into the hashes
    files.insert(DesktopSettings::System, QStringList() << path);
    settings.insert(path, new QSettings(path, QSettings::IniFormat, this) );
    //Read off the default mode for the system
    QString defMode = settings[path]->value("default_mode","fulluser").toString().toLower();
    if(defMode=="fullsystem"){ runmode= DesktopSettings::SystemFull; }
    else if(defMode=="staticinterface"){ runmode = DesktopSettings::SystemInterface; }
    else{ runmode = DesktopSettings::UserFull; }
    //Now determine the runmode for this user
    struct passwd *pw = getpwuid(getuid());
    if(pw!=0){
      QString cuser = QString(pw->pw_name);
      free(pw); //done with this structure
      if( settings[path]->value("fulluser_users", QStringList()).toStringList().contains(cuser) ){ runmode = DesktopSettings::UserFull; }
      else if( settings[path]->value("fullsystem_users", QStringList()).toStringList().contains(cuser) ){ runmode = DesktopSettings::SystemFull; }
      else if( settings[path]->value("staticinterface_users", QStringList()).toStringList().contains(cuser) ){ runmode = DesktopSettings::SystemInterface; }
      else{
        //No rule found for this specific user - check the group rules
        //Read off the list of groups
        gid_t grpList[100];
        int grpSize = 100;
        if( getgrouplist(cuser.toLocal8Bit(), getgid(), grpList, &grpSize) > 0 ){
          QStringList groups;
          for(int i=0; i<grpSize; i++){
            struct group *g = getgrgid(grpList[i]);
            if(g!=0){
              groups << QString(g->gr_name);
              free(g);
            }
          }
          QStringList fromfile = settings[path]->value("fulluser_groups", QStringList()).toStringList();
            fromfile.removeDuplicates();
          if( (fromfile+groups).removeDuplicates() > 0 ){ runmode = DesktopSettings::UserFull; }
          else{
            fromfile = settings[path]->value("fullsystem_groups", QStringList()).toStringList();
            fromfile.removeDuplicates();
            if((fromfile+groups).removeDuplicates() > 0 ){ runmode = DesktopSettings::SystemFull; }
            else{
              fromfile = settings[path]->value("staticinterface_groups", QStringList()).toStringList();
              fromfile.removeDuplicates();
              if((fromfile+groups).removeDuplicates() > 0 ){ runmode =  DesktopSettings::SystemInterface; }
            }
          }
        } //end group list read
      }
    }else{
      runmode = DesktopSettings::SystemFull; //could not read user name - assume system files only
    }

    break; //found this file - go ahead and stop now (no hierarchy for this special file)
  }
  //Now report the run mode that was detected
  QString mode;
  if(runmode == DesktopSettings::UserFull){ mode = "Full User"; }
  else if(runmode == DesktopSettings::SystemFull){ mode = "Full System"; }
  else if(runmode == DesktopSettings::SystemInterface){ mode = "System Interface"; }
   qDebug() << "Detected Lumina Runtime Mode:" << mode;
}

void DesktopSettings::locateFiles(){
  //run at start - finds the locations of the various files (based on RunMode)
  QString userdir;
  QStringList systemdirs;
  userdir = QString(getenv("XDG_CONFIG_HOME"));
  systemdirs << QString(getenv("XDG_CONFIG_DIRS")).split(":") << QString(getenv("XDG_DATA_DIRS")).split(":");
  //Load all the user-level files for this run mode
  QList< DesktopSettings::File > tmp;
  if(runmode!=DesktopSettings::SystemFull){
     //Load the user-level files
     tmp= filesForRunMode(runmode);
    for(int i=0; i<tmp.length(); i++){
      QString path = userdir+rel_path(tmp[i]);
      touchFile(path);
      files.insert(tmp[i], QStringList() << path);
      settings.insert(path, new QSettings(path, QSettings::IniFormat, this) );
      watcher->addPath(path);
    }
  }
  //Now load all the system-level files
  tmp = filesForRunMode(DesktopSettings::SystemFull);
  for(int i=0; i<systemdirs.length(); i++){
    if(systemdirs[i].endsWith("/xdg")){ systemdirs[i] = systemdirs[i].section("/",0,-2); }
    if( !QFile::exists(systemdirs[i]+"/lumina-desktop") ){ continue; }
    for(int j=0; j<tmp.length(); j++){
      QString path = systemdirs[i]+rel_path(tmp[j]);
      if(QFile::exists(path)){
        QStringList filepaths;
          if(files.contains(tmp[j])){ filepaths = files[tmp[j]]; }
           filepaths << path; //add this file to the end of the list for this type of settings file
        files.insert(tmp[j], filepaths);
        settings.insert(path, new QSettings(path, QSettings::IniFormat, this) );
        watcher->addPath(path);
      }
    }
  }

}

void DesktopSettings::touchFile(QString path){
  if(QFile::exists(path)){ return; } //already exists
  //Make sure the parent directory exists
  if(!QFile::exists(path.section("/",0,-2)) ){
    QDir dir;
    dir.mkpath(path.section("/",0,-2));
  }
  //Now create the empty file
  QFile file(path);
  if( file.open(QIODevice::ReadWrite) ){ //if it opens successfully, then it has created the file
    file.close();
  }
}

QList< DesktopSettings::File > DesktopSettings::filesForRunMode(RunMode mode){
  // Note that the "System" file is always ignored here - that is specially loaded
  QList< DesktopSettings::File > tmp;
  if(mode == DesktopSettings::UserFull || mode == DesktopSettings::SystemFull){
    tmp << DesktopSettings::Favorites << DesktopSettings::Environment << DesktopSettings::Session << DesktopSettings::Desktop <<  DesktopSettings::Panels << DesktopSettings::Plugins << DesktopSettings::Keys << DesktopSettings::ContextMenu << DesktopSettings::Animation << DesktopSettings::ScreenSaver << DesktopSettings::WM;
  }else if(runmode == DesktopSettings::SystemInterface){
    tmp << DesktopSettings::Favorites << DesktopSettings::Environment << DesktopSettings::Session;
  }
  return tmp;
}

QString DesktopSettings::rel_path(DesktopSettings::File file){
  QString name;
  switch(file){
    case DesktopSettings::System:
	name="system"; break;
    case DesktopSettings::Favorites:
	name="favorites"; break;
    case DesktopSettings::Environment:
	name="environment"; break;
    case DesktopSettings::Session:
	name="session"; break;
    case DesktopSettings::Desktop:
	name="desktop"; break;
    case DesktopSettings::ContextMenu:
	name="contextmenu"; break;
    case DesktopSettings::Keys:
	name="keys"; break;
    case DesktopSettings::Animation:
	name="animations"; break;
    case DesktopSettings::Panels:
	name="panels"; break;
    case DesktopSettings::Plugins:
	name="plugins"; break;
    case DesktopSettings::ScreenSaver:
	name="screensaver"; break;
    case DesktopSettings::WM:
	name="windows"; break;
  }
  return FILEPREFIX+name+".conf";
}

//=== PRIVATE SLOTS ===
void DesktopSettings::fileChanged(QString file){
  //qDebug() << "Got File Changed:" << file;
  //QFileSystemWatcher change detected
  if(!watcher->files().contains(file)){
    //Make sure this file stays watched for changes
    touchFile(file);
    watcher->addPath(file);
  }
  //Make sure the settings structure for this file is updated to match what is on disk
  if(settings.contains(file)){ settings[file]->sync(); }
  //Find which file type this is and send out the signal about it
  QList< DesktopSettings::File > types = files.keys();
  for(int i=0; i<types.length(); i++){
    if(files[types[i]].contains(file)){
      //qDebug() << "Emit File Type Changed:" << types[i];
      emit FileModified(types[i]);
      break;
    }
  }
}

void DesktopSettings::dirChanged(QString){
  //Not used yet - placeholder in case we need it later on
}
