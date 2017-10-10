//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "plugins-screensaver.h"
#include <QJsonDocument>

//Relative directory to search along the XDG paths for screensavers
#define REL_DIR QString("/lumina-desktop/screensavers")

// ============
//    SS PLUGIN
// ============
SSPlugin::SSPlugin(){

}

SSPlugin::~SSPlugin(){

}

void SSPlugin::loadFile(QString path){
  data = QJsonObject();
  currentfile = path;
  SSPlugin SSP;
  QFile file(path);
  if(!file.exists() || !file.open(QIODevice::ReadOnly)){ return SSP; }
  SSP.data = QJsonDocument::fromJson(file.readAll()).object();
  file.close();
  return SSP;
}

bool SSPlugin::isLoaded(){
  return !data.isEmpty();
}

bool SSPlugin::isValid(){
  if(data.isEmpty()){ return false; }
  bool ok = data.contains("name") && data.contains("qml") && data.contains("description");
  if(ok){
    //go to the next name level and see if required sub-items exist
    QJsonObject tmp = data.value("name").toObject();
    ok = tmp.contains("default");
  }
  if(ok){
    //go to the next description level and see if required sub-items exist
    QJsonObject tmp = data.value("description").toObject();
    ok = tmp.contains("default");
  }
if(ok){
    //go to the next qml level and see if required sub-items exist
    QJsonObject tmp = data.value("qml").toObject();
    QStringList mustexist;
    mustexist << tmp.value("exec").toString();
    ok = !mustexist.isEmpty(); //exec file should **always** be listed
    QJsonArray tmpA = data.value("additional_files").toArray();
    for(int i=0; i<tmpA.count(); i++){ mustexist << tmpA[i].toString(); }
    QString reldir = currentfile.section("/",0,-2) + "/";
    for(int i=0; i<mustexist.length() && ok; i++){
      if(mustexist[i].startsWith("/")){ ok = QFile::exists(mustexist[i]); }
      else { ok = QFile::exists(reldir+mustexist[i]); }
    }
  }
  return ok;
}

QString SSPlugin::translatedName(){
  QJsonObject tmp = data.value("name").toObject();
  //Get the current locale
  QString locale = getenv("LC_ALL");
  if(locale.isEmpty()){ locale = getenv("LC_MESSAGES"); }
  if(locale.isEmpty()){ locale = getenv("LANG"); }
  if(locale.isEmpty()){ locale = "default"; }
  if(locale.contains(".")){ locale = locale.section(".",0,0); } //chop any charset code off the end
  //Now find which localized string is available and return it
  if(tmp.contains(locale)){ return tmp.value(locale).toString(); }
  locale = locale.section("_",0,0); //full locale not found - look for shortened form
  if(tmp.contains(locale)){ return tmp.value(locale).toString(); }
  return tmp.value("default").toString(); //use the default version
}

QString SSPlugin::translatedDescription(){
  QJsonObject tmp = data.value("description").toObject();
  //Get the current locale
  QString locale = getenv("LC_ALL");
  if(locale.isEmpty()){ locale = getenv("LC_MESSAGES"); }
  if(locale.isEmpty()){ locale = getenv("LANG"); }
  if(locale.isEmpty()){ locale = "default"; }
  if(locale.contains(".")){ locale = locale.section(".",0,0); } //chop any charset code off the end
  //Now find which localized string is available and return it
  if(tmp.contains(locale)){ return tmp.value(locale).toString(); }
  locale = locale.section("_",0,0); //full locale not found - look for shortened form
  if(tmp.contains(locale)){ return tmp.value(locale).toString(); }
  return tmp.value("default").toString(); //use the default version
}

QUrl SSPlugin::scriptURL(){
  QString exec = data.value("qml").toObject().value("exec").toString();
  if(!exec.startsWith("/")){ exec.prepend( currentfile.section("/",0,-2)+"/" ); }
  return QUrl::fromLocalFile(exec);
}

// ===================
// 	SS PLUGIN SYSTEM
// ===================
SSPlugin SSPluginSystem::findPlugin(QString name){
  SSPlugin SSP;
  if(name.startsWith("/") && QFile::exists(name)){ SSP.loadFile(name); return SSP;} //absolute path give - just load that one
  //Cleanup the input name and ensure it has the right suffix
  name = name.section("/",-1);
  if(!name.endsWith(".json")){ name.append(".json"); }
  //Get the list of directories to search
  QStringList dirs;
  dirs << QString(getenv("XDG_DATA_HOME")) << QString(getenv("XDG_DATA_DIRS")).split(":");
  //Look for that file within these directories and return the first one found
  for(int i=0; i<dirs.length(); i++){
    if(!QFile::exists(dirs[i]+REL_DIR+"/"+name)){ continue; }
    SSP.loadFile(dirs[i]+REL_DIR+"/"+name);
    if(SSP.isValid()){ break; } //got a good one - stop here
  }
  return SSP;
}

QList<SSPlugin> SSPluginSystem::findAllPlugins(bool validonly = true){
  QList<SSPlugin> LIST;
  //Get the list of directories to search
  QStringList dirs;
  dirs << QString(getenv("XDG_DATA_HOME")) << QString(getenv("XDG_DATA_DIRS")).split(":");
 //Look for that file within these directories and return the first one found
  for(int i=0; i<dirs.length(); i++){
    if(!QFile::exists(dirs[i]+REL_DIR)){ continue; }
    QDir dir(dirs[i]+REL_DIR);
    QStringList files = dir.entryList(QStringList() << "*.json", QDir::Files, QDir::Names);
    for(int j=0; j<files.length(); j++){
      SSPlugin tmp;
      tmp.loadFile(dir.canonicalFilePath(files[j]));
      if(!validonly || tmp.isValid()){ LIST << tmp; }
    }
  }
  return LIST;
}
