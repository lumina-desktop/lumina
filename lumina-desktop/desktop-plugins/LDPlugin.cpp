//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LDPlugin.h"

#include "../LSession.h"

LDPlugin::LDPlugin(QWidget *parent, QString id) : QFrame(parent){
  PLUGID=id;
  prefix = id.replace("/","_")+"/";
  //qDebug() << "ID:" << PLUGID << prefix;
  settings = LSession::handle()->DesktopPluginSettings();
  //Use plugin-specific values for stylesheet control (applauncher, desktopview, etc...)
  this->setObjectName(id.section("---",0,0).section("::",0,0));
}

void LDPlugin::setInitialSize(int width, int height){
    //Note: Only run this in the plugin initization routine:
    //  if the plugin is completely new (first time used), it will be this size
    if(settings->allKeys().filter(prefix+"location").isEmpty()){
	//Brand new plugin: set initial size
	//qDebug() << "Setting Initial Size:" << PLUGID << width << height;
	settings->setValue(prefix+"location/width",width);
	settings->setValue(prefix+"location/height",height);
	settings->sync();
    }
}

void LDPlugin::adjustSize(int width, int height){
  settings->setValue(prefix+"location/width",width);
  settings->setValue(prefix+"location/height",height);
  settings->sync();	
  emit PluginResized();
}