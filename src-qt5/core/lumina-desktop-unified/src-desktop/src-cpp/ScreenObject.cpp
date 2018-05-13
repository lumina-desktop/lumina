//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "ScreenObject.h"
#include <QQmlEngine>
#include <QDebug>
#include "RootDesktopObject.h"

ScreenObject::ScreenObject(QScreen *scrn, QObject *parent) : QObject(parent){
  bg_screen = scrn;
  connect(this, SIGNAL(changePanels(QStringList)), this, SLOT(setPanels(QStringList)) );
  connect(RootDesktopObject::instance(), SIGNAL(sessionGeomAvailableChanged()), this, SLOT(updateAvailableGeometry()) );
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
QString ScreenObject::background(){ return bg; }
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

void ScreenObject::setPanels(QStringList ids){
  //Make this thread-safe for object creation
  if(this->thread() != QThread::currentThread()){
    //use internal signal/slot combo to change threads
    this->emit changePanels(ids);
    return;
  }

  QRegion *sess = RootDesktopObject::instance()->availableGeometry();
  QRect avail = sess->intersected(bg_screen->geometry()).boundingRect();
  if(avail.isNull()){ avail = bg_screen->geometry(); }
  //First update/remove any current panel objects
  bool change = false;
  for(int i=0; i<panel_objects.length(); i++){
    if(ids.contains(panel_objects[i]->name()) ){
      ids.removeAll(panel_objects[i]->name()); //already handled
      panel_objects[i]->syncWithSettings(avail);
    }else{
      panel_objects.takeAt(i)->deleteLater();
      i--;
      change = true; //list changed
    }
  }
  //Now create any new panel objects as needed
  for(int i=0; i<ids.length(); i++){
    PanelObject *tmp = new PanelObject(ids[i], this);
    tmp->syncWithSettings(bg_screen->geometry());
    panel_objects << tmp;
    change = true; //list changed
  }
  if(change){ emit panelsChanged(); }
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

QRect ScreenObject::availableGeometry(){
  return avail_geom;
}

void ScreenObject::updateAvailableGeometry(){
  QRegion *sess = RootDesktopObject::instance()->availableGeometry();
  QRegion availRegion = sess->intersected(bg_screen->geometry());
  QRect avail = availRegion.boundingRect();
  for(int i=0; i<panel_objects.length(); i++){
    panel_objects[i]->syncWithSettings(avail);
    //Note: Use the "full side" geometry to ensure that we are cleanly cutting off the entire side of the region
    availRegion = availRegion.subtracted( panel_objects[i]->fullSideGeometry() );
  }
  avail = availRegion.boundingRect();
  if(avail != avail_geom){
    avail_geom = avail;
    emit availableGeomChanged();
  }
}
