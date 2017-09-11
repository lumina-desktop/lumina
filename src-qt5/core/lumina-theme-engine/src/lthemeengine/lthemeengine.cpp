#include <QDir>
#include <QLocale>
#include "lthemeengine.h"

#ifndef LTHEMEENGINE_DATADIR
#define LTHEMEENGINE_DATADIR "/usr/local/share"
#endif

#include <QDebug>

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
  for(int i=0; i<dirs.length(); i++){ dirs[i].append("/lthemeengine/qss/"); }
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
  for(int i=0; i<dirs.length(); i++){ dirs[i].append("/lthemeengine/desktop_qss/"); }
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
  for(int i=0; i<dirs.length(); i++){ dirs[i].append("/lthemeengine/colors/"); }
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
