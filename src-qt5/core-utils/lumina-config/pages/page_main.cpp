//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "page_main.h"
#include "ui_page_main.h"

//==========
//    PUBLIC
//==========
page_main::page_main(QWidget *parent) : PageWidget(parent), ui(new Ui::page_main()){
  ui->setupUi(this);

}

page_main::~page_main(){

}



//================
//    PUBLIC SLOTS
//================
void page_main::SaveSettings(){

}

void page_main::LoadSettings(int screennum){

}

void page_main::updateIcons(){

}

//===========
//    PRIVATE
//===========
