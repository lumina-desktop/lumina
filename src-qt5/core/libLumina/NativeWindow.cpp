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
  WIN->deleteLater();
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
  if(prop == NativeWindow::None){ return; }
  hash.insert(prop, val);
  emit PropertyChanged(prop, val);
}
