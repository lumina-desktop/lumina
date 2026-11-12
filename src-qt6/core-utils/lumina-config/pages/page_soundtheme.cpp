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
  sessionsettings = new QSettings("lumina-desktop", "sessionsettings");

  sessionsettings->setValue("PlayStartupAudio", ui->checkBox_startup->isChecked());
  sessionsettings->setValue("audiofiles/login", ui->label_startup->whatsThis());

  sessionsettings->setValue("PlayLogoutAudio", ui->checkBox_logout->isChecked());
  sessionsettings->setValue("audiofiles/logout", ui->label_logout->whatsThis());

  sessionsettings->setValue("PlayBatteryLowAudio", ui->checkBox_battery->isChecked());
  sessionsettings->setValue("audiofiles/batterylow", ui->label_battery->whatsThis());

}

void page_soundtheme::LoadSettings(int){
  emit ChangePageTitle( tr("Sound Themes") );

  sessionsettings = new QSettings("lumina-desktop", "sessionsettings");

  ui->label_startup->setText(sessionsettings->value("audiofiles/login").toString().section("/",-1));
  ui->label_startup->setToolTip(sessionsettings->value("audiofiles/batterylow").toString());
  ui->label_startup->setWhatsThis(sessionsettings->value("audiofiles/login").toString());
  bool playstartup = sessionsettings->value("PlayStartupAudio").toBool();
  if( playstartup ){ ui->checkBox_startup->setChecked(true); }

  ui->label_logout->setText(sessionsettings->value("audiofiles/logout").toString().section("/",-1));
  ui->label_startup->setToolTip(sessionsettings->value("audiofiles/batterylow").toString());
  ui->label_logout->setWhatsThis(sessionsettings->value("audiofiles/logout").toString());
  bool playlogout = sessionsettings->value("PlayLogoutAudio").toBool();
  if( playlogout ){ ui->checkBox_logout->setChecked(true); }

  ui->label_battery->setText(sessionsettings->value("audiofiles/batterylow").toString().section("/",-1));
  ui->label_startup->setToolTip(sessionsettings->value("audiofiles/batterylow").toString());
  ui->label_battery->setWhatsThis(sessionsettings->value("audiofiles/batterylow").toString());
  bool playbattery = sessionsettings->value("PlayBatteryLowAudio").toBool();
  if( playbattery ){ ui->checkBox_battery->setChecked(true);}
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
