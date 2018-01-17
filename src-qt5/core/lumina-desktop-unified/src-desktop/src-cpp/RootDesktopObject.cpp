//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "RootDesktopObject.h"
#include <QQmlEngine>
#include <QApplication>
#include <QScreen>

#include <QDebug>

// === PUBLIC ===
RootDesktopObject::RootDesktopObject(QObject *parent) : QObject(parent){
  updateScreens(); //make sure the internal list is updated right away
  connect(this, SIGNAL(changePanels(QStringList)), this, SLOT(setPanels(QStringList)) );
}

RootDesktopObject::~RootDesktopObject(){

}

void RootDesktopObject::RegisterType(){
  static bool done = false;
  if(done){ return; }
  done=true;
  qmlRegisterType<RootDesktopObject>("Lumina.Backend.RootDesktopObject", 2, 0, "RootDesktopObject");
  //Also register any types that are needed by this class
  ScreenObject::RegisterType();
  NativeWindowObject::RegisterType();
}

RootDesktopObject* RootDesktopObject::instance(){
  static RootDesktopObject* r_obj = new RootDesktopObject();
  return r_obj;
}

//QML Read Functions
QStringList RootDesktopObject::screens(){
  //qDebug() << "Request Screens:" << s_objects.length();
  QStringList names;
  for(int i=0; i<s_objects.length(); i++){ names << s_objects[i]->name(); }
  return names;
}

ScreenObject* RootDesktopObject::screen(QString id){
  //qDebug() << "Got Screen Request:" << id;
  for(int i=0; i<s_objects.length(); i++){
    if(s_objects[i]->name()==id){ return s_objects[i]; }
  }
  return 0;
}

QStringList RootDesktopObject::panels(){
  //qDebug() << "Request Panels:" << panel_objects.length();
  QStringList names;
  for(int i=0; i<panel_objects.length(); i++){ names << panel_objects[i]->name(); }
  return names;
}

PanelObject* RootDesktopObject::panel(QString id){
  //qDebug() << "Got Panel Request:" << id;
  for(int i=0; i<panel_objects.length(); i++){
    if(panel_objects[i]->name()==id){ return panel_objects[i]; }
  }
  return 0;
}

QStringList RootDesktopObject::windows(){
  //qDebug() << "Request Panels:" << panel_objects.length();
  QStringList names;
  for(int i=0; i<window_objects.length(); i++){ names << QString::number(window_objects[i]->id()); }
  return names;
}

NativeWindowObject* RootDesktopObject::window(QString id){
  //qDebug() << "Got Panel Request:" << id;
  WId chk = id.toInt(); //numerical ID's in this case
  for(int i=0; i<window_objects.length(); i++){
    if(window_objects[i]->id()==chk){ return window_objects[i]; }
  }
  return 0;
}

void RootDesktopObject::setPanels(QList<PanelObject*> list){
  panel_objects = list;
  emit panelsChanged();
}

void RootDesktopObject::setPanels(QStringList ids){
  //Make this thread-safe for object creation
  if(this->thread() != QThread::currentThread()){
    //use internal signal/slot combo to change threads
    this->emit changePanels(ids);
    return;
  }

  //Get the current bounding rectangle for the session
  QRect total;
  for(int i=0; i<s_objects.length(); i++){
    total = total.united(s_objects[i]->geometry());
  }
  //First update/remove any current panel objects
  bool change = false;
  for(int i=0; i<panel_objects.length(); i++){
    if(ids.contains(panel_objects[i]->name()) ){
      ids.removeAll(panel_objects[i]->name()); //already handled
      panel_objects[i]->syncWithSettings(total);
    }else{
      panel_objects.takeAt(i)->deleteLater();
      i--;
      change = true; //list changed
    }
  }
  //Now create any new panel objects as needed
  for(int i=0; i<ids.length(); i++){
    PanelObject *tmp = new PanelObject(ids[i], this);
    tmp->syncWithSettings(total);
    panel_objects << tmp;
    change = true; //list changed
  }
  if(change){ emit panelsChanged(); }
}

void RootDesktopObject::setWindows(QList<NativeWindowObject*> list){
  window_objects = list;
  emit windowsChanged();
}

void RootDesktopObject::logout(){
  emit startLogout();
}

void RootDesktopObject::lockscreen(){
  emit lockScreen();
}

void RootDesktopObject::mousePositionChanged(){
  emit mouseMoved();
}

void RootDesktopObject::launchApp(QString appOrPath){
  emit launchApplication(appOrPath);
}

// === PUBLIC SLOTS ===
void RootDesktopObject::updateScreens(){
  QList<QScreen*> scrns = QApplication::screens();
  QList<ScreenObject*> tmp; //copy of the internal array initially
  for(int i=0; i<scrns.length(); i++){
    bool found = false;
    for(int j=0; j<s_objects.length() && !found; j++){
      if(s_objects[j]->name()==scrns[i]->name()){ found = true; tmp << s_objects.takeAt(j); }
    }
    if(!found){ tmp << new ScreenObject(scrns[i], this); }
  }
  //Delete any leftover objects
  for(int i=0; i<s_objects.length(); i++){ s_objects[i]->deleteLater(); }
  s_objects = tmp;
  emit screensChanged();
  for(int i=0; i<s_objects.length(); i++){
    s_objects[i]->emit geomChanged();
  }
}

void RootDesktopObject::ChangeWallpaper(QString screen, QString value){
  for(int i=0; i<s_objects.length(); i++){
    if(s_objects[i]->name()==screen){ s_objects[i]->setBackground(value); break; }
  }
}

QString RootDesktopObject::CurrentWallpaper(QString screen){
  for(int i=0; i<s_objects.length(); i++){
    if(s_objects[i]->name()==screen){ return s_objects[i]->background();}
  }
  return ""; //unknown
}



// === PRIVATE ===
