//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "page_sample.h"
#include "ui_page_sample.h"

//==========
//    PUBLIC
//==========
page_sample::page_sample(QWidget *parent) : PageWidget(parent), ui(new Ui::page_sample()){
  ui->setupUi(this);

 updateIcons();
}

page_sample::~page_sample(){

}

//================
//    PUBLIC SLOTS
//================
void page_sample::SaveSettings(){

  emit HasPendingChanges(false);
}

void page_sample::LoadSettings(int){
  emit HasPendingChanges(false);
  emit ChangePageTitle( tr("Desktop Settings") );

}

void page_sample::updateIcons(){

}

//=================
//         PRIVATE 
//=================

//=================
//    PRIVATE SLOTS
//=================
