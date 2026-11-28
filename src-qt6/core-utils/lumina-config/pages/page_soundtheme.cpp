//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2017, JT (q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "page_soundtheme.h"
#include "ui_page_soundtheme.h"
#include "../AppDialog.h"

//==========
//    PUBLIC
//==========
page_soundtheme::page_soundtheme(QWidget *parent) : PageWidget(parent), ui(new Ui::page_soundtheme()){
  ui->setupUi(this);
}

page_soundtheme::~page_soundtheme(){

}

//================
//    PUBLIC SLOTS
//================
void page_soundtheme::SaveSettings(){
  //Stack object: a new QSettings was allocated (and leaked) on every save/load
  QSettings settings("lumina-desktop", "sessionsettings");

  settings.setValue("PlayStartupAudio", ui->checkBox_startup->isChecked());
  settings.setValue("audiofiles/login", ui->label_startup->whatsThis());

  settings.setValue("PlayLogoutAudio", ui->checkBox_logout->isChecked());
  settings.setValue("audiofiles/logout", ui->label_logout->whatsThis());

  settings.setValue("PlayBatteryLowAudio", ui->checkBox_battery->isChecked());
  settings.setValue("audiofiles/batterylow", ui->label_battery->whatsThis());

}

void page_soundtheme::LoadSettings(int){
  emit ChangePageTitle( tr("Sound Themes") );

  QSettings settings("lumina-desktop", "sessionsettings");
  //Defaults match the desktop (sounds play unless disabled), and each label gets its own file as tooltip
  // (all three tooltips used to be written to the startup label, with the battery file)
  QString file = settings.value("audiofiles/login").toString();
  ui->label_startup->setText(file.section("/",-1));
  ui->label_startup->setToolTip(file);
  ui->label_startup->setWhatsThis(file);
  ui->checkBox_startup->setChecked( settings.value("PlayStartupAudio", true).toBool() );

  file = settings.value("audiofiles/logout").toString();
  ui->label_logout->setText(file.section("/",-1));
  ui->label_logout->setToolTip(file);
  ui->label_logout->setWhatsThis(file);
  ui->checkBox_logout->setChecked( settings.value("PlayLogoutAudio", true).toBool() );

  file = settings.value("audiofiles/batterylow").toString();
  ui->label_battery->setText(file.section("/",-1));
  ui->label_battery->setToolTip(file);
  ui->label_battery->setWhatsThis(file);
  ui->checkBox_battery->setChecked( settings.value("PlayBatteryLowAudio", true).toBool() );
  emit HasPendingChanges(false);
}

//=================
//         PRIVATE
//=================

//=================
//    PRIVATE SLOTS
//=================

void page_soundtheme::on_pushButton_startup_clicked(){
  QString startupsound = QFileDialog::getOpenFileName(this, tr("Select Startup Sound"), QDir::homePath());
  if(startupsound.isEmpty()){ return; }
  ui->label_startup->setText(startupsound.section("/",-1));
  ui->label_startup->setToolTip(startupsound);
  ui->label_startup->setWhatsThis(startupsound);
  settingChanged();
  qDebug() << "startup whats this" << startupsound;
}

void page_soundtheme::on_pushButton_logout_clicked(){
  QString logoutsound = QFileDialog::getOpenFileName(this, tr("Select Logout Sound"), QDir::homePath());
  if(logoutsound.isEmpty()){ return; }
  ui->label_logout->setText(logoutsound.section("/",-1));
  ui->label_logout->setToolTip(logoutsound);
  ui->label_logout->setWhatsThis(logoutsound);
  qDebug() << "startup whats this" << logoutsound;
  settingChanged();
}

void page_soundtheme::on_pushButton_battery_clicked(){
  QString batterysound = QFileDialog::getOpenFileName(this, tr("Select Low Battery Sound"), QDir::homePath());
  if(batterysound.isEmpty()){ return; }
  ui->label_battery->setText(batterysound.section("/",-1));
  ui->label_battery->setToolTip(batterysound);
  ui->label_battery->setWhatsThis(batterysound);
  qDebug() << "startup whats this" << batterysound;
  settingChanged();
}


void page_soundtheme::on_checkBox_startup_toggled(bool checked){
  settingChanged();
}

void page_soundtheme::on_checkBox_logout_toggled(bool checked){
  settingChanged();
}

void page_soundtheme::on_checkBox_battery_toggled(bool checked){
  settingChanged();
}
