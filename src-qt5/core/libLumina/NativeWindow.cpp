//===========================================
//  Lumina-DE source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "NativeWindow.h"

// === PUBLIC ===
NativeWindow::NativeWindow(WId id) : QObject(){
  winid = id;
  WIN = QWindow::fromWinId(winid);
}

NativeWindow::~NativeWindow(){
  hash.clear();
  //WIN->deleteLater(); //This class only deals with Native windows which were created outside the app - they need to be cleaned up outside the app too
}

void NativeWindow::addFrameWinID(WId fid){
  relatedTo << fid;
}

bool NativeWindow::isRelatedTo(WId tmp){
  return (relatedTo.contains(tmp) || winid == tmp);
}

WId NativeWindow::id(){
  return winid;
}

QWindow* NativeWindow::window(){
  return WIN;
}

QVariant NativeWindow::property(NativeWindow::Property prop){
  if(hash.contains(prop)){ return hash.value(prop); }
  return QVariant(); //null variant
}

void NativeWindow::setProperty(NativeWindow::Property prop, QVariant val){
  if(prop == NativeWindow::None || hash.value(prop)==val){ return; }
  hash.insert(prop, val);
  emit PropertiesChanged(QList<NativeWindow::Property>() << prop, QList<QVariant>() << val);
}

void NativeWindow::setProperties(QList<NativeWindow::Property> props, QList<QVariant> vals){
  for(int i=0; i<props.length(); i++){
    if(i>=vals.length()){ props.removeAt(i); i--; continue; } //no corresponding value for this propertu
    if(props[i] == NativeWindow::None || (hash.value(props[i]) == vals[i]) ){ props.removeAt(i); vals.removeAt(i); i--; continue; } //Invalid property or identical value
    hash.insert(props[i], vals[i]);
  }
  emit PropertiesChanged(props, vals);
}

void NativeWindow::requestProperty(NativeWindow::Property prop, QVariant val){
  if(prop == NativeWindow::None || hash.value(prop)==val ){ return; }
  emit RequestPropertiesChange(winid, QList<NativeWindow::Property>() << prop, QList<QVariant>() << val);
}

void NativeWindow::requestProperties(QList<NativeWindow::Property> props, QList<QVariant> vals){
  //Verify/adjust inputs as needed
  for(int i=0; i<props.length(); i++){
    if(i>=vals.length()){ props.removeAt(i); i--; continue; } //no corresponding value for this property
    if(props[i] == NativeWindow::None || hash.value(props[i])==vals[i] ){ props.removeAt(i); vals.removeAt(i); i--; continue; } //Invalid property or identical value
  }
  emit RequestPropertiesChange(winid, props, vals);
}

// ==== PUBLIC SLOTS ===
void NativeWindow::requestClose(){
  emit RequestClose(winid);
}

void NativeWindow::requestKill(){
  emit RequestKill(winid);
}

void NativeWindow::requestPing(){
  emit RequestPing(winid);
}
