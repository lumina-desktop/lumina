//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore & JT Pennington
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "page_bluetooth_trueos.h"
#include "ui_page_bluetooth_trueos.h"

//==========
//    PUBLIC
//==========
page_bluetooth::page_bluetooth(QWidget *parent) : PageWidget(parent), ui(new Ui::page_bluetooth()){
  ui->setupUi(this);

 updateIcons();
}

page_bluetooth::~page_bluetooth(){

}

//================
//    PUBLIC SLOTS
//================
void page_bluetooth::SaveSettings(){

  emit HasPendingChanges(false);
}

void page_bluetooth::LoadSettings(int){
  emit HasPendingChanges(false);
  emit ChangePageTitle( tr("Bluetooth Settings") );
}

void page_bluetooth::updateIcons(){

}

//=================
//         PRIVATE 
//=================

//=================
//    PRIVATE SLOTS
//=================
