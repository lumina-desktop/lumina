//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>
#include <QSettings>
#include <QLocale>
#include <QApplication>
#include <QMessageBox>
#include <QKeySequence>
#include <QMenuBar>
#include <QStatusBar>
#include <QAction>

#include <LUtils.h>

//==========
//    PUBLIC
//==========
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);
    
    setupMenus();
    setupLocales();
    setupConnections();
    loadSettings();
    
    // Set window properties
    setWindowTitle(tr("Lumina Locale Settings"));
    resize(500, 400);
    
    // Show initial status
    updateStatusBar(tr("Ready"));
}

MainWindow::~MainWindow(){
    delete ui;
}

//================
//    PUBLIC SLOTS
//================
void MainWindow::slotSingleInstance(){
    this->show();
    this->raise();
    this->activateWindow();
}

//=================
//    PRIVATE SLOTS
//=================
void MainWindow::saveSettings(){
    QSettings sessionsettings("lumina-desktop","sessionsettings");
    sessionsettings.setValue("InitLocale/LANG", ui->combo_locale_lang->currentData().toString() );
    sessionsettings.setValue("InitLocale/LC_MESSAGES", ui->combo_locale_message->currentData().toString() );
    sessionsettings.setValue("InitLocale/LC_TIME", ui->combo_locale_time->currentData().toString() );
    sessionsettings.setValue("InitLocale/LC_NUMERIC", ui->combo_locale_numeric->currentData().toString() );
    sessionsettings.setValue("InitLocale/LC_MONETARY", ui->combo_locale_monetary->currentData().toString() );
    sessionsettings.setValue("InitLocale/LC_COLLATE", ui->combo_locale_collate->currentData().toString() );
    sessionsettings.setValue("InitLocale/LC_CTYPE", ui->combo_locale_ctype->currentData().toString() );
    
    updateStatusBar(tr("Settings saved successfully"));
}

void MainWindow::loadSettings(){
    QSettings sessionsettings("lumina-desktop","sessionsettings");

    QString val = sessionsettings.value("InitLocale/LANG", "").toString();
    int index = ui->combo_locale_lang->findData(val);
    if(index<0){ index = 0; } //system default
    ui->combo_locale_lang->setCurrentIndex(index);
    
    val = sessionsettings.value("InitLocale/LC_MESSAGES", "").toString();
    index = ui->combo_locale_message->findData(val);
    if(index<0){ index = 0; } //system default
    ui->combo_locale_message->setCurrentIndex(index);
    
    val = sessionsettings.value("InitLocale/LC_TIME", "").toString();
    index = ui->combo_locale_time->findData(val);
    if(index<0){ index = 0; } //system default
    ui->combo_locale_time->setCurrentIndex(index);
    
    val = sessionsettings.value("InitLocale/LC_NUMERIC", "").toString();
    index = ui->combo_locale_numeric->findData(val);
    if(index<0){ index = 0; } //system default
    ui->combo_locale_numeric->setCurrentIndex(index);
    
    val = sessionsettings.value("InitLocale/LC_MONETARY", "").toString();
    index = ui->combo_locale_monetary->findData(val);
    if(index<0){ index = 0; } //system default
    ui->combo_locale_monetary->setCurrentIndex(index);
    
    val = sessionsettings.value("InitLocale/LC_COLLATE", "").toString();
    index = ui->combo_locale_collate->findData(val);
    if(index<0){ index = 0; } //system default
    ui->combo_locale_collate->setCurrentIndex(index);
    
    val = sessionsettings.value("InitLocale/LC_CTYPE", "").toString();
    index = ui->combo_locale_ctype->findData(val);
    if(index<0){ index = 0; } //system default
    ui->combo_locale_ctype->setCurrentIndex(index);
    
    updateStatusBar(tr("Settings loaded"));
}

void MainWindow::settingChanged(){
    // Auto-save when settings change
    saveSettings();
}

void MainWindow::showAbout(){
    QMessageBox::about(this, tr("About Lumina Locale Settings"),
        tr("Lumina Locale Settings\n\n"
           "Configure system localization settings for the Lumina Desktop Environment.\n\n"
           "Copyright (c) 2024, jt(q5sys)\n"
           "Available under the 3-clause BSD license"));
}

//=================
//         PRIVATE 
//=================
void MainWindow::setupLocales(){
    // Available localizations
    QStringList langs = LUtils::knownLocales();
    langs.sort();
    QString def = tr("System Default");
    
    ui->combo_locale_lang->addItem(def,"");
    ui->combo_locale_collate->addItem(def,"");
    ui->combo_locale_ctype->addItem(def,"");
    ui->combo_locale_message->addItem(def,"");
    ui->combo_locale_monetary->addItem(def,"");
    ui->combo_locale_numeric->addItem(def,"");
    ui->combo_locale_time->addItem(def,"");
    
    for(int i=0; i<langs.length(); i++){
        QString lan = QLocale(langs[i]).nativeLanguageName();
        ui->combo_locale_lang->addItem(lan,langs[i]);
        ui->combo_locale_collate->addItem(lan,langs[i]);
        ui->combo_locale_ctype->addItem(lan,langs[i]);
        ui->combo_locale_message->addItem(lan,langs[i]);
        ui->combo_locale_monetary->addItem(lan,langs[i]);
        ui->combo_locale_numeric->addItem(lan,langs[i]);
        ui->combo_locale_time->addItem(lan,langs[i]);
    }
}

void MainWindow::setupMenus(){
    // File Menu
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    
    QAction *saveAction = fileMenu->addAction(tr("&Save Settings"));
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setStatusTip(tr("Save current locale settings"));
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveSettings);
    
    fileMenu->addSeparator();
    
    QAction *quitAction = fileMenu->addAction(tr("&Quit"));
    quitAction->setShortcut(QKeySequence::Quit);
    quitAction->setStatusTip(tr("Exit the application"));
    connect(quitAction, &QAction::triggered, this, &QWidget::close);
    
    // Help Menu
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    
    QAction *aboutAction = helpMenu->addAction(tr("&About"));
    aboutAction->setStatusTip(tr("Show information about this application"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
}

void MainWindow::setupConnections(){
    connect(ui->combo_locale_lang, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::settingChanged);
    connect(ui->combo_locale_collate, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::settingChanged);
    connect(ui->combo_locale_ctype, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::settingChanged);
    connect(ui->combo_locale_message, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::settingChanged);
    connect(ui->combo_locale_monetary, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::settingChanged);
    connect(ui->combo_locale_numeric, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::settingChanged);
    connect(ui->combo_locale_time, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::settingChanged);
}

void MainWindow::updateStatusBar(const QString &message){
    statusBar()->showMessage(message, 3000);
}
