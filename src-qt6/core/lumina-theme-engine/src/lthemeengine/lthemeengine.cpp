#include <QDir>
#include <QLocale>
#include "lthemeengine.h"

#ifndef LTHEMEENGINE_DATADIR
#define LTHEMEENGINE_DATADIR "/usr/local/share"
#endif

#include <QDebug>

QStringList lthemeengine::readFile(QString filepath){
  QStringList out;
  QFile file(filepath);
  if(file.open(QIODevice::Text | QIODevice::ReadOnly)){
    QTextStream in(&file);
    while(!in.atEnd()){
      out << in.readLine();
    }
    file.close();
  }
  return out;
}

QString lthemeengine::configPath(){
  return QDir::homePath() + "/.config/lthemeengine/";
}

QString lthemeengine::configFile(){
  return configPath() + "lthemeengine.conf";
}

QStringList lthemeengine::iconPaths(){
  QString xdgDataDirs = qgetenv("XDG_DATA_DIRS");
  QString xdgDataHome = qgetenv("XDG_DATA_HOME");
  QStringList paths;
  paths << QDir::homePath() + "/.icons/";
  if(xdgDataDirs.isEmpty()){
    paths << "/usr/share/icons";
    paths << "/usr/local/share/icons";
    }
  else{
    foreach (QString p, xdgDataDirs.split(":")){ paths << QDir(p + "/icons/").absolutePath(); }
    }
  if(xdgDataHome.isEmpty()){ xdgDataHome = QDir::homePath() + "/.local/share"; }
  paths << "/usr/share/pixmaps";
  paths << xdgDataHome + "/icons";
  paths.removeDuplicates();
  //remove invalid
  foreach (QString p, paths){
    if(!QDir(p).exists()){ paths.removeAll(p); }
    }
  return paths;
}

QString lthemeengine::userStyleSheetPath(){
  return configPath() + "qss/";
}

QStringList lthemeengine::sharedStyleSheetPath(){
  QStringList dirs;
  dirs << QString(getenv("XDG_CONFIG_HOME"));
  dirs << QString(getenv("XDG_CONFIG_DIRS")).split(":");
  dirs << QString(getenv("XDG_DATA_DIRS")).split(":");
  for(int i=0; i<dirs.length(); i++){
      if (!dirs[i].endsWith("/")){ dirs[i].append("/"); }
      dirs[i].append("lthemeengine/qss/");
  }
  if(dirs.isEmpty()){ dirs << LTHEMEENGINE_DATADIR"/lthemeengine/qss/"; } //no XDG settings - use the hardcoded path
  return dirs;
 }

QString lthemeengine::userDesktopStyleSheetPath(){
  return configPath() + "desktop_qss/";
}

QStringList lthemeengine::sharedDesktopStyleSheetPath(){
  QStringList dirs;
  dirs << QString(getenv("XDG_CONFIG_HOME"));
  dirs << QString(getenv("XDG_CONFIG_DIRS")).split(":");
  dirs << QString(getenv("XDG_DATA_DIRS")).split(":");
  for(int i=0; i<dirs.length(); i++){
      if (!dirs[i].endsWith("/")){ dirs[i].append("/"); }
      dirs[i].append("lthemeengine/desktop_qss/");
  }
  if(dirs.isEmpty()){ dirs << LTHEMEENGINE_DATADIR"/lthemeengine/desktop_qss/"; } //no XDG settings - use the hardcoded path
  return dirs;
 }

QString lthemeengine::userColorSchemePath(){
  return configPath() + "colors/";
}

QStringList lthemeengine::sharedColorSchemePath(){
  QStringList dirs;
  dirs << QString(getenv("XDG_CONFIG_HOME"));
  dirs << QString(getenv("XDG_CONFIG_DIRS")).split(":");
  dirs << QString(getenv("XDG_DATA_DIRS")).split(":");
  for(int i=0; i<dirs.length(); i++){
      if (!dirs[i].endsWith("/")){ dirs[i].append("/"); }
      dirs[i].append("lthemeengine/colors/");
  }
  if(dirs.isEmpty()){ dirs << LTHEMEENGINE_DATADIR"/lthemeengine/colors/"; } //no XDG settings - use the hardcoded path
  qDebug() << "Got Color Dirs:" << dirs;
  return dirs;
}

QString lthemeengine::systemLanguageID(){
#ifdef Q_OS_UNIX
  QByteArray v = qgetenv ("LC_ALL");
  if (v.isEmpty()){ v = qgetenv ("LC_MESSAGES"); }
  if (v.isEmpty()){ v = qgetenv ("LANG"); }
  if (!v.isEmpty()){ return QLocale (v).name(); }
#endif
  return  QLocale::system().name();
}

QStringList lthemeengine::availableSystemCursors(){	//returns: [name] for each item
  //Find all the directories which could contain themes
 QStringList paths;
  paths << QDir::homePath()+"/.icons";
  QStringList xdd = QString(getenv("XDG_DATA_HOME")).split(":");
  xdd << QString(getenv("XDG_DATA_DIRS")).split(":");
  for(int i=0; i<xdd.length(); i++){
    if(QFile::exists(xdd[i]+"/icons")){
      paths << xdd[i]+"/icons";
    }
  }
  //Now get all the icon themes in these directories
  QStringList themes, tmpthemes;
  QDir dir;
  for(int i=0; i<paths.length(); i++){
    if(dir.cd(paths[i])){
       tmpthemes = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
       for(int j=0; j<tmpthemes.length(); j++){
	 if(tmpthemes[j].startsWith("default")){ continue; }
         if( QFile::exists(dir.absoluteFilePath(tmpthemes[j]+"/cursors")) ){ themes << tmpthemes[j]; }
       }
    }
  }
  //Clean up the list and return
  themes.removeDuplicates();
  themes.sort();
  return themes;
}

//Return the currently-selected Cursor theme
QString lthemeengine::currentCursor(){
  //qDebug() << "Reading Current Cursor Theme:";
  QStringList info = readFile(QDir::homePath()+"/.icons/default/index.theme");
  if(info.isEmpty()){ return ""; }
  QString cursor;
  bool insection = false;
  for(int i=0; i<info.length(); i++){
    if(info[i]=="[Icon Theme]"){ insection = true; continue;}
    else if(insection && info[i].startsWith("Inherits=")){
      cursor = info[i].section("=",1,1).simplified();
      break;
    }
  }
  //qDebug() << " - found theme:" << cursor;
  return cursor;
}

//Change the current Cursor Theme
bool lthemeengine::setCursorTheme(QString cursorname){
//qDebug() << "Set Cursor Theme:" << cursorname;
  if(cursorname=="default"){
    //special case - this will cause a recursive inheritance loop - just remove the file instead
    if(QFile::exists(QDir::homePath()+"/.icons/default/index.theme")){
      return QFile::remove(QDir::homePath()+"/.icons/default/index.theme");
    }
    return true; //already does not exist
  }
  QStringList info = readFile(QDir::homePath()+"/.icons/default/index.theme");
    bool insection = false;
    bool changed = false;
    QString newval = "Inherits="+cursorname;
    for(int i=0; i<info.length() && !changed; i++){
      if(info[i]=="[Icon Theme]"){
	insection = true;
      }else if( info[i].startsWith("[") && insection){
	//Section does not have the setting - add it
	info.insert(i, newval);
	changed =true;
      }else if( info[i].startsWith("[") ){
	insection = false;
      }else if(insection && info[i].startsWith("Inherits=")){
        info[i] = newval; //replace the current setting
        changed = true;
      }
    } //end loop over file contents
    if(!changed){ //Could not change the file contents for some reason
      if(insection){ info << newval; } //end of file while in the section
      else{ info << "[Icon Theme]" << newval; } //entire section missing from file
    }
    //Now save the file
    if( !QFile::exists(QDir::homePath()+"/.icons/default") ){
      //Need to create the directory first
      QDir().mkpath(QDir::homePath()+"/.icons/default");
    }
    QFile file(QDir::homePath()+"/.icons/default/index.theme");
    bool ok = false;
    if( file.open(QIODevice::WriteOnly | QIODevice::Truncate) ){
      QTextStream out(&file);
      out << info.join("\n");
      if(!info.last().isEmpty()){ out << "\n"; } //always end with a new line
      file.close();
      ok = true;
    }
    //qDebug() << "Done saving the cursor:" << info;
    return ok;
}
