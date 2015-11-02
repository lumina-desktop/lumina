//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================

#include "SSBaseWidget.h"

static QStringList validPlugs;
// ========
//   PUBLIC
// ========
SSBaseWidget::SSBaseWidget(QWidget *parent, QSettings *set) : QWidget(parent){
  if(validPlugs.isEmpty()){ validPlugs << "none"; } //add more later
  settings = set; //needed to pass along for plugins to read any special options/settings
  ANIM = 0;
}

SSBaseWidget::~SSBaseWidget(){
	
}
	
void SSBaseWidget::setPlugin(QString plug){
  plug = plug.toLower();
  if(validPlugs.contains(plug) || plug=="random"){ plugType = plug; }
  else{ plugType = "none"; }
}

// =============
//  PUBLIC SLOTS
// =============
void SSBaseWidget::startPainting(){
  cplug = plugType;
  if(ANIM!=0){ ANIM->clear(); }
  //If a random plugin - grab one of the known plugins
  if(cplug=="random"){ 
    QStringList valid = BaseAnimGroup::KnownAnimations();
    if(valid.isEmpty()){ cplug = "none"; } //no known plugins
    else{ cplug = valid[ qrand()%valid.length() ]; } //grab a random plugin
  }
  //Now list all the various plugins and start them appropriately
  QString style;
  if(cplug=="none"){
    style = "background: transparent"; //show the underlying black parent widget
  }
  this->setStyleSheet(style);
  //If not a stylesheet-based plugin - set it here
  if(ANIM!=0){ free(ANIM); ANIM = 0; } //free up the old instance
  if(cplug!="none"){
    ANIM = BaseAnimGroup::NewAnimation(cplug, this, settings);
    connect(ANIM, SIGNAL(finished()), this, SLOT(startPainting()) ); //repeat the plugin as needed
  }
  //Now start the animation(s)
  if(ANIM!=0){
    if(ANIM->animationCount()>0){ ANIM->start(); }
  }
}

void SSBaseWidget::stopPainting(){
  if(ANIM!=0){
    ANIM->stop();
    ANIM->clear();
  }
}
