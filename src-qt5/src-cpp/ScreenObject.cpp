//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "ScreenObject.h"
#include <QQmlEngine>

ScreenObject::ScreenObject(QScreen *scrn, QObject *parent) : QObject(parent){
  bg_screen = scrn;
}

void ScreenObject::RegisterType(){
  qmlRegisterType<ScreenObject>("Lumina.Backend.ScreenObject",2,0, "ScreenObject");
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
