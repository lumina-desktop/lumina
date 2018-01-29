//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "PanelObject.h"
#include <global-objects.h>

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

void PanelObject::syncWithSettings(QRect parent_geom){
  //Read off all the settings
  QString id = panel_id.section("/",-1); //last section (allow for prefixes to distinguish multiple monitors with the same profile but different screens)
  //qDebug() << "Sync Panel Settings:" << panel_id << id << parent_geom;
  QString anchor = DesktopSettings::instance()->value(DesktopSettings::Panels, id+"/anchor", "bottom").toString().toLower();
  QString align = DesktopSettings::instance()->value(DesktopSettings::Panels, id+"/align", "center").toString().toLower();
  double length = DesktopSettings::instance()->value(DesktopSettings::Panels, id+"/length_percent", 100).toDouble()/100.0;
  double width = DesktopSettings::instance()->value(DesktopSettings::Panels, id+"/width_font_percent", 2.1).toDouble();
    width =  qRound(width * QApplication::fontMetrics().height() );
  this->setBackground( DesktopSettings::instance()->value(DesktopSettings::Panels, id+"/background", "rgba(0,0,0,120)").toString() );
 // qDebug() << "Update Panel:" << panel_id << id << anchor+"/"+align << length << width;
  //Now calculate the geometry of the panel
  QRect newgeom;
  //Figure out the size of the panel
  if(anchor=="top" || anchor=="bottom"){ newgeom.setWidth( parent_geom.width()*length ); newgeom.setHeight(width); }
  else{ newgeom.setWidth(width); newgeom.setHeight(parent_geom.height()*length); }
  //qDebug() << " - Size:" << newgeom;
  //Now figure out the location of the panel
  if(align=="left" || align=="top"){
    if(anchor=="top" || anchor=="left"){ newgeom.moveTopLeft(QPoint(0,0)); }
    else if(anchor=="right"){ newgeom.moveTopRight(QPoint(parent_geom.width(), 0)); }
    else{ newgeom.moveBottomLeft(QPoint(0, parent_geom.height()) ); } //bottom by default

  }else if(align=="right" || align=="bottom"){
    if(anchor=="top"){ newgeom.moveTopRight(QPoint(parent_geom.width(),0)); }
    else if(anchor=="left"){ newgeom.moveBottomLeft(QPoint(0, parent_geom.height())); }
    else if(anchor=="right"){ newgeom.moveBottomRight(QPoint(parent_geom.width(), parent_geom.height())); }
    else{ newgeom.moveBottomRight(QPoint(parent_geom.width(), parent_geom.height()) ); }

  }else{ //center
    if(anchor=="top"){ newgeom.moveTopLeft(QPoint( (parent_geom.width()-newgeom.width())/2,0)); }
    else if(anchor=="left"){ newgeom.moveTopLeft(QPoint(0, (parent_geom.height()-newgeom.height())/2 )); }
    else if(anchor=="right"){ newgeom.moveTopRight(QPoint(parent_geom.width(), (parent_geom.height()-newgeom.height())/2 )); }
    else{ newgeom.moveBottomLeft(QPoint( (parent_geom.width()-newgeom.width())/2, parent_geom.height()) ); }
  }
  //qDebug() << " - Calculated Geometry (relative to parent):" << newgeom;
  //Note: newgeom is currently in parent-relative coordinates (not global)
  newgeom.translate(parent_geom.x(), parent_geom.y());
  //qDebug() << " - Calculated Geometry (global):" << newgeom;
  this->setGeometry(newgeom); //shift to global coordinates
}
