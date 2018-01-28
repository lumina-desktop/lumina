//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "page_compton.h"
#include "ui_page_compton.h"

//==========
//    PUBLIC
//==========
page_compton::page_compton(QWidget *parent) : PageWidget(parent), ui(new Ui::page_compton()){
    ui->setupUi(this);
    connect(ui->text_file, SIGNAL(textChanged()), this, SLOT(settingChanged()) );
    connect(ui->check_disablecompton, SIGNAL(toggled(bool)), this, SLOT(settingChanged()) );
    updateIcons();
}

page_compton::~page_compton(){

}

//================
//    PUBLIC SLOTS
//================
void page_compton::SaveSettings(){
  emit HasPendingChanges(false);
  QSettings settings("lumina-desktop","sessionsettings");
    settings.setValue("enableCompositing", !ui->check_disablecompton->isChecked());
    settings.setValue("compositingWithGpuAccelOnly", ui->check_GPUverify->isChecked());
  QString set = QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/compton.conf";
  LUtils::writeFile(set, ui->text_file->toPlainText().split("\n"),true);
}

void page_compton::LoadSettings(int){
  emit ChangePageTitle( tr("Window Effects") );
  QSettings settings("lumina-desktop","sessionsettings");
    ui->check_disablecompton->setChecked( !settings.value("enableCompositing", false).toBool() );
    ui->check_GPUverify->setChecked( settings.value("compositingWithGpuAccelOnly", true).toBool() );
  QString set = QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/compton.conf";
  qDebug() << "Load Compton settings:" << set;
  ui->text_file->setPlainText( LUtils::readFile(set).join("\n") );
  emit HasPendingChanges(false);
}

void page_compton::updateIcons(){
emit HasPendingChanges(false);
}

//=================
//         PRIVATE 
//=================

//=================
//    PRIVATE SLOTS
//=================
