//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, JT (q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details 
//===========================================
#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QApplication>
#include <QMessageBox>
#include <QDir>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);
    
    // Initialize variables
    sessionsettings = nullptr;
    hasChanges = false;
    
    // Setup save timer for auto-save functionality
    saveTimer = new QTimer(this);
    saveTimer->setSingleShot(true);
    saveTimer->setInterval(300); // 300ms delay
    connect(saveTimer, SIGNAL(timeout()), this, SLOT(saveTimerTimeout()));
    
    // Connect menu actions
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(saveTimerTimeout()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    
    // Load current settings
    LoadSettings();
    
    // Set window properties
    this->setWindowTitle(tr("Sound Theme Settings"));
    ui->statusbar->showMessage(tr("Ready"), 2000);
}

MainWindow::~MainWindow(){
    if(sessionsettings != nullptr){
        delete sessionsettings;
    }
}

void MainWindow::slotSingleInstance(){
    this->show();
    this->raise();
    this->activateWindow();
}

void MainWindow::LoadSettings(){
    if(sessionsettings != nullptr){
        delete sessionsettings;
    }
    sessionsettings = new QSettings("lumina-desktop", "sessionsettings");
    
    // Load startup sound settings
    QString startupFile = sessionsettings->value("audiofiles/login").toString();
    bool playStartup = sessionsettings->value("PlayStartupAudio", false).toBool();
    
    if(!startupFile.isEmpty() && QFile::exists(startupFile)){
        ui->label_startup->setText(startupFile.section("/",-1));
        ui->label_startup->setToolTip(startupFile);
        ui->label_startup->setWhatsThis(startupFile);
    } else {
        ui->label_startup->setText(tr("No sound file selected"));
        ui->label_startup->setToolTip("");
        ui->label_startup->setWhatsThis("");
    }
    ui->checkBox_startup->setChecked(playStartup);
    
    // Load logout sound settings
    QString logoutFile = sessionsettings->value("audiofiles/logout").toString();
    bool playLogout = sessionsettings->value("PlayLogoutAudio", false).toBool();
    
    if(!logoutFile.isEmpty() && QFile::exists(logoutFile)){
        ui->label_logout->setText(logoutFile.section("/",-1));
        ui->label_logout->setToolTip(logoutFile);
        ui->label_logout->setWhatsThis(logoutFile);
    } else {
        ui->label_logout->setText(tr("No sound file selected"));
        ui->label_logout->setToolTip("");
        ui->label_logout->setWhatsThis("");
    }
    ui->checkBox_logout->setChecked(playLogout);
    
    // Load battery sound settings
    QString batteryFile = sessionsettings->value("audiofiles/batterylow").toString();
    bool playBattery = sessionsettings->value("PlayBatteryLowAudio", false).toBool();
    
    if(!batteryFile.isEmpty() && QFile::exists(batteryFile)){
        ui->label_battery->setText(batteryFile.section("/",-1));
        ui->label_battery->setToolTip(batteryFile);
        ui->label_battery->setWhatsThis(batteryFile);
    } else {
        ui->label_battery->setText(tr("No sound file selected"));
        ui->label_battery->setToolTip("");
        ui->label_battery->setWhatsThis("");
    }
    ui->checkBox_battery->setChecked(playBattery);
    
    hasChanges = false;
}

void MainWindow::SaveSettings(){
    if(sessionsettings == nullptr){
        sessionsettings = new QSettings("lumina-desktop", "sessionsettings");
    }
    
    // Save startup sound settings
    sessionsettings->setValue("PlayStartupAudio", ui->checkBox_startup->isChecked());
    sessionsettings->setValue("audiofiles/login", ui->label_startup->whatsThis());
    
    // Save logout sound settings
    sessionsettings->setValue("PlayLogoutAudio", ui->checkBox_logout->isChecked());
    sessionsettings->setValue("audiofiles/logout", ui->label_logout->whatsThis());
    
    // Save battery sound settings
    sessionsettings->setValue("PlayBatteryLowAudio", ui->checkBox_battery->isChecked());
    sessionsettings->setValue("audiofiles/batterylow", ui->label_battery->whatsThis());
    
    sessionsettings->sync();
    hasChanges = false;
    
    ui->statusbar->showMessage(tr("Settings saved"), 2000);
}

void MainWindow::settingChanged(){
    hasChanges = true;
    saveTimer->start(); // Start/restart the save timer
}

void MainWindow::saveTimerTimeout(){
    if(hasChanges){
        SaveSettings();
    }
}

void MainWindow::on_pushButton_startup_clicked(){
    QString startupSound = QFileDialog::getOpenFileName(this, 
        tr("Select Startup Sound"), 
        QDir::homePath(),
        tr("Audio Files (*.wav *.mp3 *.ogg *.flac *.aac);;All Files (*)"));
    
    if(startupSound.isEmpty()){ 
        return; 
    }
    
    ui->label_startup->setText(startupSound.section("/",-1));
    ui->label_startup->setToolTip(startupSound);
    ui->label_startup->setWhatsThis(startupSound);
    settingChanged();
}

void MainWindow::on_pushButton_logout_clicked(){
    QString logoutSound = QFileDialog::getOpenFileName(this, 
        tr("Select Logout Sound"), 
        QDir::homePath(),
        tr("Audio Files (*.wav *.mp3 *.ogg *.flac *.aac);;All Files (*)"));
    
    if(logoutSound.isEmpty()){ 
        return; 
    }
    
    ui->label_logout->setText(logoutSound.section("/",-1));
    ui->label_logout->setToolTip(logoutSound);
    ui->label_logout->setWhatsThis(logoutSound);
    settingChanged();
}

void MainWindow::on_pushButton_battery_clicked(){
    QString batterySound = QFileDialog::getOpenFileName(this, 
        tr("Select Low Battery Sound"), 
        QDir::homePath(),
        tr("Audio Files (*.wav *.mp3 *.ogg *.flac *.aac);;All Files (*)"));
    
    if(batterySound.isEmpty()){ 
        return; 
    }
    
    ui->label_battery->setText(batterySound.section("/",-1));
    ui->label_battery->setToolTip(batterySound);
    ui->label_battery->setWhatsThis(batterySound);
    settingChanged();
}

void MainWindow::on_checkBox_startup_toggled(bool checked){
    Q_UNUSED(checked);
    settingChanged();
}

void MainWindow::on_checkBox_logout_toggled(bool checked){
    Q_UNUSED(checked);
    settingChanged();
}

void MainWindow::on_checkBox_battery_toggled(bool checked){
    Q_UNUSED(checked);
    settingChanged();
}

void MainWindow::showAbout(){
    QMessageBox::about(this, tr("About Sound Theme Settings"),
        tr("<h3>Sound Theme Settings</h3>"
           "<p>Configure system sound events for the Lumina Desktop Environment.</p>"
           "<p><b>Version:</b> %1<br>"
           "<b>License:</b> 3-clause BSD<br>"
           "<b>Copyright:</b> 2024, JT (q5sys)</p>"
           "<p>Part of the Lumina Control Center (LCC) suite.</p>")
           .arg(qApp->applicationVersion()));
}
