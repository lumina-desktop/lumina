//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "Panel.h"

// PUBLIC
Panel::Panel(PanelObject *pobj) : QWidget(0, Qt::Window | Qt::FramelessWindowHint){
  this->setObjectName("LuminaPanelBackgroundWidget");
  obj = pobj;
  layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    layout->setContentsMargins(0,0,0,0);
  this->setBackgroundRole(QPalette::AlternateBase);
  connect(obj, SIGNAL(backgroundChanged()), this, SLOT(updateBackground()) );
  connect(obj, SIGNAL(geomChanged()), this, SLOT(updateGeom()) );
  connect(obj, SIGNAL(pluginsChanged()), this, SLOT(updatePlugins()) );
  connect(obj, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)) );
  updateGeom();
  updateBackground();
  updatePlugins();
  this->showNormal();
}

Panel::~Panel(){

}

// PRIVATE
Plugin* Panel::findPlugin(QString id){
  for(int i=0; i<PLUGINS.count(); i++){
    if(PLUGINS[i]->id()==id){ return PLUGINS[i]; }
  }
  return 0;
}

Plugin* Panel::createPlugin(QString id){
  Plugin* tmp = Plugin::createPlugin(this, id, true, obj->isVertical());
  if(tmp!=0){ PLUGINS << tmp; }
  return tmp;
}

// PRIVATE SLOTS
void Panel::objectDestroyed(QObject *dobj){
  if(dobj == Q_NULLPTR || dobj == obj){
    //Make sure this widget is also cleaned up when the base object is deleted
    this->deleteLater();
  }
}

void Panel::updateGeom(){
  this->setGeometry(obj->geometry());
  this->setFixedSize(obj->geometry().size());
  layout->setDirection( obj->isVertical() ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight );
  for(int i=0; i<PLUGINS.length(); i++){
    PLUGINS[i]->setVertical( obj->isVertical() );
  }
}

void Panel::updateBackground(){
  static QString PANELTEMPLATE = "QToolButton::menu-indicator{ image: none; } QWidget#LuminaPanelBackgroundWidget{ background: %1; }";
  QString bg = obj->background();
  //qDebug() << "Got panel BG:" << obj->name() << bg;
  this->setStyleSheet(PANELTEMPLATE.arg(bg));
}

void Panel::updatePlugins(){
  QStringList plugs = obj->plugins();
  //qDebug() << "Got panel plugins:" << obj->name() << plugs;
  while(PLUGINS.length()>0){
    Plugin *plug = PLUGINS.takeFirst();
    //qDebug() << "Removing Plugin:" << plug->id() << PLUGINS.length();
    layout->removeWidget(plug);
    plug->deleteLater();
  }
  for(int i=0; i<plugs.length(); i++){
    //lastplugins.removeAll(plugs[i]); //remove this from the last list (handled)
    Plugin *plug = 0; //findPlugin(plugs[i]);
    if(plug==0){
      plug = createPlugin(plugs[i]);
      if(plug==0){ continue; } //not a valid plugin - just skip it
    }
    //Now setup the order of the plugins properly
    if( layout->indexOf(plug) >=0 ){
      layout->removeWidget(plug); //remove from the layout for a moment
    }
    layout->insertWidget(i, plug);
    plug->setupSizing();
  }
  //Now remove any plugins which were deleted from config
  /*for(int i=0; i<lastplugins.length(); i++){
    Plugin *plug = findPlugin(lastplugins[i]);
    if(plug==0){ continue; }
    plug->deleteLater();
  }*/
  //lastplugins = plugs;
}
