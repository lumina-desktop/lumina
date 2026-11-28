//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "plugins-screensaver.h"

// ============
//    SS PLUGIN
// ============
SSPlugin::SSPlugin(){

}

SSPlugin::~SSPlugin(){

}

bool SSPlugin::isValid(){
  if(data.isEmpty()){ return false; }
  if( data.value("disabled").toBool(false) ){ return false; } //Skip any plugin with "disabled" = true
  bool ok = data.contains("name") && data.contains("qml") && data.contains("description");
  ok &= containsDefault("name");
  ok &= containsDefault("description");
  if(ok) {
    QJsonObject tmp = data.value("qml").toObject();
    QStringList mustexist;
    QString exec = tmp.value("exec").toString();
    if(exec.isEmpty() || !exec.endsWith(".qml")){ return false; }
    mustexist << exec;
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
