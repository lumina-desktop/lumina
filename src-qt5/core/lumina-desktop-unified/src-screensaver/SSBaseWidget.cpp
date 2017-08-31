//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================

#include "SSBaseWidget.h"

#define DEBUG 1

// ========
//   PUBLIC
// ========
SSBaseWidget::SSBaseWidget(QWidget *parent) : QWidget(parent){
  this->setObjectName("LuminaBaseSSWidget");
  ANIM = 0;
  this->setMouseTracking(true);
  plugType="none";
  restartTimer = new QTimer(this);
    restartTimer->setInterval( DesktopSettings::instance()->value(DesktopSettings::ScreenSaver, "globals/plugin_time_seconds", 60).toInt() * 1000);
    restartTimer->setSingleShot(true);
  connect(restartTimer, SIGNAL(timeout()), this, SLOT(startPainting()) );
}

SSBaseWidget::~SSBaseWidget(){
  if(ANIM!=0){ this->stopPainting(); }
}

void SSBaseWidget::setPlugin(QString plug){
  plugType = plug.toLower();
}

// =============
//  PUBLIC SLOTS
// =============
void SSBaseWidget::startPainting(){
  cplug = plugType;
  //free up any old animation instance
  if(ANIM!=0){
    stopPainting();
  }
  //If a random plugin - grab one of the known plugins
  if(cplug=="random"){
    QStringList valid = BaseAnimGroup::KnownAnimations();
    valid.removeAll("none"); //they want a screensaver - remove the "none" option from the valid list
    if(valid.isEmpty()){ cplug = "none"; } //no known plugins
    else{ cplug = valid[ qrand()%valid.length() ]; } //grab a random plugin
  }
  if(DEBUG){ qDebug() << " - Screen Saver:" << plugType << cplug; }
  //Now list all the various plugins and start them appropriately
  QString style;
  if(cplug=="none"){
    style = "background: black;"; //show the underlying black parent widget
  }else{
    style = "background: black;";
  }
  this->setStyleSheet("QWidget#LuminaBaseSSWidget{ "+style+"}");
  this->repaint();
  //If not a stylesheet-based plugin - set it here
  if(cplug!="none"){
    ANIM = BaseAnimGroup::NewAnimation(cplug, this);
    connect(ANIM, SIGNAL(finished()), this, SLOT(startPainting()) ); //repeat the plugin as needed
    ANIM->LoadAnimations();
  }
  //Now start the animation(s)
  if(ANIM!=0){
    if(ANIM->animationCount()>0){
      if(DEBUG){ qDebug() << " - Starting SS Plugin:" << cplug << ANIM->animationCount() << ANIM->duration() << ANIM->loopCount(); }
      ANIM->start();
    }
  }
  restartTimer->start();
}

void SSBaseWidget::stopPainting(){
  if(ANIM!=0){
    if(DEBUG){ qDebug() << "Stopping Animation!!"; }
    ANIM->stop();
    //ANIM->clear();
    ANIM->deleteLater();
    ANIM = 0;
    //Delete any child widgets of the canvas
    QList<QWidget*> widgets = this->findChildren<QWidget*>("",Qt::FindDirectChildrenOnly);
    for(int i=0; i<widgets.length(); i++){ widgets[i]->deleteLater(); }
  }
  if(restartTimer->isActive()){ restartTimer->stop(); }
}
