//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "plugins-desktop.h"

#include <LuminaXDG.h>

// ============
//    DT PLUGIN
// ============
DTPlugin::DTPlugin(){
}

DTPlugin::~DTPlugin(){
}

bool DTPlugin::isValid(){
  if(data.isEmpty()){ return false; }
  bool ok = data.contains("name") && data.contains("qml") && data.contains("description");
  ok &= containsDefault("name");
  ok &= containsDefault("description");
  ok &= containsDefault("data");
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

QSize DTPlugin::getSize(){
  QString gridS = data.value("data").toObject().value("grid_size").toString();
  QSize gridSize(0,0);
  if(!gridS.isEmpty()) {
    QStringList gridList = gridS.split("x");
    if(gridList.size() == 2) {
      int width = gridList[0].toInt();
      int height = gridList[1].toInt();
      if(width > 0 && height > 0)
        gridSize = QSize(width, height);
    }
  }
  return gridSize;
}

bool DTPlugin::supportsPanel(){
  QString possibleS = data.value("data").toObject().value("panel_possible").toString().toLower();
  bool panelPossible = false;
  if(!possibleS.isEmpty() && (possibleS == "yes" || possibleS == "true")){
    panelPossible = true;
  }
  return panelPossible;
}

QString DTPlugin::getIcon(){
    QString iconS = data.value("data").toObject().value("plugin_icon").toString();
    if(iconS.isEmpty()){ iconS = "preferences-plugin"; }
    return iconS;
}
