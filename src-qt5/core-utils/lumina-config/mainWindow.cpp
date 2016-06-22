//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "mainWindow.h"
#include "ui_mainWindow.h"
#include "globals.h"

//=============
//      PUBLIC
//=============
mainWindow::mainWindow() : QMainWindow(), ui(new Ui::mainWindow()){
  ui->setupUi(this);

}

mainWindow::~mainWindow(){

}

//==============
//  PUBLIC SLOTS
//==============
void mainWindow::slotSingleInstance(){

}

void mainWindow::setupIcons(){

}

//=============
//      PRIVATE
//=============

//================
//  PRIVATE SLOTS
//================
