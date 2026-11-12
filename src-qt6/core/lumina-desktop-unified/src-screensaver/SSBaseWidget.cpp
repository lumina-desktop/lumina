//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================

#include "SSBaseWidget.h"

//Relative directory to search along the XDG paths for screensavers
#define REL_DIR QString("/lumina-desktop/screensavers")

#define DEBUG 0

// ========
//   PUBLIC
// ========
SSBaseWidget::SSBaseWidget(QWidget *parent) : QQuickView(parent->windowHandle()){
  this->setObjectName("LuminaBaseSSWidget");
  this->setResizeMode(QQuickView::SizeRootObjectToView);
  this->setColor(QColor("black")); //default color for the view
  this->setCursor(Qt::BlankCursor);
  plugType="none";
  restartTimer = new QTimer(this);
    restartTimer->setInterval( DesktopSettings::instance()->value(DesktopSettings::ScreenSaver, "globals/plugin_time_seconds", 120).toInt() * 1000);
    restartTimer->setSingleShot(true);
  connect(restartTimer, SIGNAL(timeout()), this, SLOT(startPainting()) );
}

SSBaseWidget::~SSBaseWidget(){
}

void SSBaseWidget::setPlugin(QString plug){
  plugType = plug.toLower();
}

// =============
//  PUBLIC SLOTS
// =============
void SSBaseWidget::startPainting(){
  //free up any old animation instance
  stopPainting();
  //If a random plugin - grab one of the known plugins
  if(plugType=="random"){
    QList<SSPlugin> valid = PluginSystem::findAllPlugins<SSPlugin>(REL_DIR);
    if(!valid.isEmpty()){ cplug = valid[ qrand()%valid.length() ]; } //grab a random plugin
  }else if(plugType!="none"){
    cplug = PluginSystem::findPlugin<SSPlugin>(plugType, REL_DIR);
  }
  if(DEBUG){ qDebug() << " - Screen Saver:" << plugType << cplug.scriptURL() << cplug.isValid(); }
  if(cplug.isValid()){
    this->setSource( cplug.scriptURL() );
    if(plugType=="random"){ restartTimer->start(); }
  }

}

void SSBaseWidget::stopPainting(){
  if(!this->source().isEmpty()){
    this->setSource(QUrl());
    cplug = SSPlugin(); //empty structure
  }
  if(restartTimer->isActive()){ restartTimer->stop(); }
}
