//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LDPlugin.h"

#include "../LSession.h"
#include <LuminaXDG.h>

LDPlugin::LDPlugin(QWidget *parent, QString id) : QFrame(parent){
  PLUGID=id;
  prefix = id.replace("/","_")+"/";
  //qDebug() << "ID:" << PLUGID << prefix;
  settings = LSession::handle()->DesktopPluginSettings();
  //Setup the plugin system control menu
  menu = new QMenu(this);
  setupMenu();
  //Setup the internal timer for when to start/stop drag events
  dragTimer = new QTimer(this);
    dragTimer->setSingleShot(true);
    dragTimer->setInterval(500); //1/2 second to show the plugin menu
    connect(dragTimer, SIGNAL(timeout()), this, SLOT(showPluginMenu()));
  //Use plugin-specific values for stylesheet control (applauncher, desktopview, etc...)
  this->setObjectName(id.section("---",0,0).section("::",0,0));
  this->setContextMenuPolicy(Qt::CustomContextMenu);
  this->setMouseTracking(false); //only catch mouse movement events if the mouse is clicked/held on the plugin
  connect(QApplication::instance(), SIGNAL(LocaleChanged()), this, SLOT(LocaleChange()) );
  connect(QApplication::instance(), SIGNAL(IconThemeChanged()), this, SLOT(ThemeChange()) );
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showPluginMenu()) );
}

void LDPlugin::setupMenu(){
  menu->clear();
  //SPECIAL CONTEXT MENU OPTIONS FOR PARTICULAR PLUGIN TYPES
  if(PLUGID.startsWith("applauncher::")){
    menu->addAction( LXDG::findIcon("quickopen",""), tr("Launch Item"), this, SIGNAL(PluginActivated()) );
    menu->addSeparator();
  }
  //General Options
  menu->addAction( LXDG::findIcon("transform-move",""), tr("Start Moving Item"), this, SLOT(slotStartMove()) );
  menu->addAction( LXDG::findIcon("transform-scale",""), tr("Start Resizing Item"), this, SLOT(slotStartResize()) );
  menu->addSeparator();
  menu->addAction( LXDG::findIcon("zoom-in",""), tr("Increase Item Sizes"), this, SIGNAL(IncreaseIconSize()) );
  menu->addAction( LXDG::findIcon("zoom-out",""), tr("Decrease Item Sizes"), this, SIGNAL(DecreaseIconSize()) );
  menu->addSeparator();
  menu->addAction( LXDG::findIcon("edit-delete",""), tr("Remove Item"), this, SLOT(slotRemovePlugin()) );
}

/*void LDPlugin::setInitialSize(int width, int height){
    //Note: Only run this in the plugin initization routine:
    //  if the plugin is completely new (first time used), it will be this size
    if(settings->allKeys().filter(prefix+"location").isEmpty()){
	//Brand new plugin: set initial size
	//qDebug() << "Setting Initial Size:" << PLUGID << width << height;
	settings->setValue(prefix+"location/width",width);
	settings->setValue(prefix+"location/height",height);
	settings->sync();
    }
    //Now make sure the plugin is the saved size right away
    this->resize( settings->value(prefix+"location/width").toInt(), settings->value(prefix+"location/height").toInt());
}*/
