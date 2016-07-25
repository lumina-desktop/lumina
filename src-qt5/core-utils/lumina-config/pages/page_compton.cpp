//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "page_compton.h"
#include "ui_page_compton.h"
#include "getPage.h"

//==========
//    PUBLIC
//==========
page_compton::page_compton(QWidget *parent) : PageWidget(parent), ui(new Ui::page_compton()){
  ui->setupUi(this);
  connect(ui->text_file, SIGNAL(textChanged()), this, SLOT(settingChanged()) );
  updateIcons();
}

page_compton::~page_compton(){

}

//================
//    PUBLIC SLOTS
//================
void page_compton::SaveSettings(){

  emit HasPendingChanges(false);
  QString set = QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/compton.conf";
  LUtils::writeFile(set, ui->text_file->toPlainText().split("\n"),true);
}

void page_compton::LoadSettings(int){
  emit HasPendingChanges(false);
  emit ChangePageTitle( tr("Compositor Settings") );
  QString set = QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/compton.conf";
  qDebug() << "Load Compton settings:" << set;
  ui->text_file->setPlainText( LUtils::readFile(set).join("\n") );
}

void page_compton::updateIcons(){

}

//=================
//         PRIVATE 
//=================

//=================
//    PRIVATE SLOTS
//=================
