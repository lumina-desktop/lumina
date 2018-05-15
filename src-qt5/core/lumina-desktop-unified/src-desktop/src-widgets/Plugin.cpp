//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "Plugin.h"

//Add includes for individual plugin files here
#include <ClockPlugin.h>

QStringList Plugin::built_in_plugins(){
  QStringList list;
  list << "clock";
  return list;
}

Plugin* Plugin::createPlugin(QWidget *parent, QString id, bool panelplug){
  Plugin *plug = 0;
  //QString id_extra;
  //if(id.contains("--")){ id_extra = id.section("--",1,-1); id = id.section("--",0,0); }
  if(id=="clock"){
    plug = new ClockPlugin(parent, id, panelplug);
  }

  return plug;
}
