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

#ifdef USE_WIDGETS
#include <Plugin.h>
#endif

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
bool PanelObject::isVertical(){
  return ( geom.width() < geom.height() );
}

QStringList PanelObject::plugins(){
  return panel_plugins;
}

void PanelObject::setBackground(QString fileOrColor){
  if(bg!=fileOrColor){
    bg = fileOrColor;
    emit backgroundChanged();
  }
}

void PanelObject::setGeometry( QRect newgeom ){
  if(geom!=newgeom){
    geom = newgeom;
    fullside_geom = geom; //unknown full-side geom - no parent info given
    emit geomChanged();
  }
}

void PanelObject::setPlugins( QStringList plist){
  //Iterate through the list and find the URL's for the files
  QStringList dirs; dirs << ":/qml/plugins/"; //add local directories here
  static QStringList built_in;
#ifdef USE_WIDGETS
  if(built_in.isEmpty()){ built_in = Plugin::built_in_plugins(); }
#endif
  for(int i=0; i<plist.length(); i++){
    bool found = built_in.contains(plist[i].toLower());
    for(int j=0; j<dirs.length() && !found; j++){
      if(QFile::exists(dirs[j]+plist[i]+".qml")){
        plist[i] = QUrl::fromLocalFile(dirs[j]+plist[i]+".qml").url();
        found = true;
      }
    }
    if(!found){
      qWarning() << "Could not find panel plugin on system:" << plist[i];
      plist.removeAt(i);
      i--;
    }
  }
  //Now update the internal list if it has changed
  if(panel_plugins != plist){
    panel_plugins = plist;
    this->emit pluginsChanged();
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
    //qDebug() << " Got Panel Width From Settings:" << width;
  this->setBackground( DesktopSettings::instance()->value(DesktopSettings::Panels, id+"/background", "rgba(0,0,0,120)").toString() );
 // qDebug() << "Update Panel:" << panel_id << id << anchor+"/"+align << length << width;
  //Now calculate the geometry of the panel
  QRect newgeom, newfullsidegeom;
  //Figure out the size of the panel
  if(anchor=="top" || anchor=="bottom"){
    newgeom.setWidth( parent_geom.width()*length ); newgeom.setHeight(width);
    newfullsidegeom.setWidth(parent_geom.width()); newfullsidegeom.setHeight(width);
    if(anchor=="top"){ newfullsidegeom.moveTopLeft(QPoint(0,0)); }
    else{ newfullsidegeom.moveBottomLeft( QPoint(0, parent_geom.height()) ); }
  }else{
    newgeom.setWidth(width); newgeom.setHeight(parent_geom.height()*length);
    newfullsidegeom.setWidth(width); newfullsidegeom.setHeight(parent_geom.height());
    if(anchor=="left"){ newfullsidegeom.moveTopLeft(QPoint(0,0)); }
    else{ newfullsidegeom.moveTopRight( QPoint(parent_geom.width(), 0) ); }
  }
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
  newfullsidegeom.translate(parent_geom.x(), parent_geom.y());
  fullside_geom = newfullsidegeom;
  //qDebug() << " - Calculated Geometry (global):" << newgeom;
  this->setGeometry(newgeom); //in global coordinates
  this->setPlugins( DesktopSettings::instance()->value(DesktopSettings::Panels, id+"/plugins", QStringList()).toStringList() );
}
