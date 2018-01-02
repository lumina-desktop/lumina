//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "PanelObject.h"
#include <QQmlEngine>
#include <QDebug>

PanelObject::PanelObject(QString id, QObject *parent) : QObject(parent){
  panel_id = id;
}

void PanelObject::RegisterType(){
  static bool done = false;
  if(done){ return; }
  done=true;
  qmlRegisterType<PanelObject>("Lumina.Backend.PanelObject",2,0, "PanelObject");
}

QString PanelObject::name(){ return panel_id; }
QString PanelObject::background(){
  if(bg.isEmpty()){ return "transparent"; }
  return bg;
}
int PanelObject::x(){ return geom.x(); }
int PanelObject::y(){ return geom.y(); }
int PanelObject::width(){ return geom.width(); }
int PanelObject::height(){ return geom.height(); }

void PanelObject::setBackground(QString fileOrColor){
  if(bg!=fileOrColor){
    bg = fileOrColor;
    emit backgroundChanged();
  }
}

void PanelObject::setGeometry( QRect newgeom ){
  if(geom!=newgeom){
    geom = newgeom;
    emit geomChanged();
  }
}
