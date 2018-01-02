//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "plugins-base.h"

// ============
//    Base PLUGIN
// ============
BasePlugin::BasePlugin(){

}

BasePlugin::~BasePlugin(){

}

void BasePlugin::loadFile(QString path){
  data = QJsonObject();
  currentfile = path;
  QFile file(path);
  if(!file.exists() || !file.open(QIODevice::ReadOnly)){ return; }
  data = QJsonDocument::fromJson(file.readAll()).object();
  //qDebug() << "Loaded ScreenSaver Data:" << currentfile << data;
  file.close();
}

QString BasePlugin::translatedObject(QString obj){
  QJsonObject tmp = data.value(obj).toObject();
  //Get the current locale
  QString locale = getenv("LC_ALL");
  if(locale.isEmpty()){ locale = getenv("LC_MEBaseAGES"); }
  if(locale.isEmpty()){ locale = getenv("LANG"); }
  if(locale.isEmpty()){ locale = "default"; }
  if(locale.contains(".")){ locale = locale.section(".",0,0); } //chop any charset code off the end
  //Now find which localized string is available and return it
  if(tmp.contains(locale)){ return tmp.value(locale).toString(); }
  locale = locale.section("_",0,0); //full locale not found - look for shortened form
  if(tmp.contains(locale)){ return tmp.value(locale).toString(); }
  return tmp.value("default").toString(); //use the default version
}

QUrl BasePlugin::scriptURL(){
  QString exec = data.value("qml").toObject().value("exec").toString();
  if(!exec.startsWith("/")){ exec.prepend( currentfile.section("/",0,-2)+"/" ); }
  return QUrl::fromLocalFile(exec);
}

