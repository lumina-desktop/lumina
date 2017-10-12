//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "RootDesktopObject.h"

// === PUBLIC ===
RootDesktopObject::RootDesktopObject(QObject *parent = 0){
  updateScreens(); //make sure the internal list is updated right away
}

RootDesktopObject::~RootDesktopObject(){

}

static RootDesktopObject* RootDesktopObject::instance(){
  static RootDesktopObject* r_obj = new RootDesktopObject();
  return r_obj;
}

//QML Read Functions
QList<QScreen*> RootDesktopObject::screens(){
  return ;
}

// === PUBLIC SLOTS ===
void RootDesktopObject::updateScreens(){
  QList<QScreen*> scrns = QApplication::screens();
  QList<ScreenObject*> tmp; //copy of the internal array initially
  for(int i=0; i<scrns.length(); i++){
    bool found = false;
    for(int j=0; j<s_objects.length() && !found; j++){
      if(s_objects[j].name()==scrns.name()){ found = true; tmp << s_objects.takeAt(j); }
    }
    if(!found){ tmp << new ScreenObject(scrns[i], this); }
  }
  //Delete any leftover objects
  for(int i=0; i<s_objects.length(); i++){ s_objects[i]->deleteLater(); }
  s_objects = tmp;
}

// === PRIVATE ===
