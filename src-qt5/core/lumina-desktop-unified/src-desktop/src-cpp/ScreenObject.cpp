//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "ScreenObject.h"
#include <QQmlEngine>
#include <QDebug>

ScreenObject::ScreenObject(QScreen *scrn, QObject *parent) : QObject(parent){
  bg_screen = scrn;
}

void ScreenObject::RegisterType(){
  static bool done = false;
  if(done){ return; }
  done=true;
  qmlRegisterType<ScreenObject>("Lumina.Backend.ScreenObject",2,0, "ScreenObject");
  //Also register any types that are needed by this class
  PanelObject::RegisterType();
}

QString ScreenObject::name(){ return bg_screen->name(); }
QString ScreenObject::background(){ qDebug() << "Got Background:" << bg_screen->name() << bg << bg_screen->geometry(); return bg; }
int ScreenObject::x(){ return bg_screen->geometry().x(); }
int ScreenObject::y(){ return bg_screen->geometry().y(); }
int ScreenObject::width(){ return bg_screen->geometry().width(); }
int ScreenObject::height(){ return bg_screen->geometry().height(); }

void ScreenObject::setBackground(QString fileOrColor){
  if(bg!=fileOrColor){
    bg = fileOrColor;
    emit backgroundChanged();
  }
}

void ScreenObject::setPanels(QList<PanelObject*> list){
  panel_objects = list;
  emit panelsChanged();
}

//QML Read Functions
QStringList ScreenObject::panels(){
  //qDebug() << "Request Panels:" << panel_objects.length();
  QStringList names;
  for(int i=0; i<panel_objects.length(); i++){ names << panel_objects[i]->name(); }
  return names;
}

PanelObject* ScreenObject::panel(QString id){
  //qDebug() << "Got Panel Request:" << id;
  for(int i=0; i<panel_objects.length(); i++){
    if(panel_objects[i]->name()==id){ return panel_objects[i]; }
  }
  return 0;
}
