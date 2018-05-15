//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "Plugin.h"

//Add includes for individual plugin files here
#include <ClockPlugin.h>
#include <SpacerPlugin.h>

QStringList Plugin::built_in_plugins(){
  QStringList list;
  list << "clock" << "spacer";
  return list;
}

Plugin* Plugin::createPlugin(QWidget *parent, QString id, bool panelplug){
  Plugin *plug = 0;
  QString id_extra, id_primary;
  if(id.contains("--")){ id_extra = id.section("--",1,-1); id_primary = id.section("--",0,0); }
  else{ id_primary = id; } //no extra info on this one
  id_primary = id_primary.toLower();
  if(id_primary=="clock"){
    plug = new ClockPlugin(parent, id, panelplug);
  }else if(id_primary=="spacer"){
    plug = new SpacerPlugin(parent, id, panelplug);
  }

  return plug;
}
