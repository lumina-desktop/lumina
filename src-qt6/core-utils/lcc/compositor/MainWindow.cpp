//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details 
//===========================================
#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDir>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);
    loading = false;
    
    // Setup auto-save timer
    settingsTimer = new QTimer(this);
    settingsTimer->setSingleShot(true);
    settingsTimer->setInterval(300); // 300ms delay
    connect(settingsTimer, SIGNAL(timeout()), this, SLOT(saveSettings()));
    
    setupMenus();
    setupConnections();
    loadSettings();
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::updateIcons(){
    // Update any icons if needed
    this->setWindowIcon(QIcon::fromTheme("preferences-desktop-effects"));
}

void MainWindow::setupConnections(){
    // Connect UI elements to settings changed slot
    connect(ui->text_file, SIGNAL(textChanged()), this, SLOT(settingChanged()));
    connect(ui->check_disablecompton, SIGNAL(toggled(bool)), this, SLOT(settingChanged()));
}

void MainWindow::setupMenus(){
    // File Menu
    QMenu *fileMenu = ui->menubar->addMenu(tr("&File"));
    
    QAction *saveAction = fileMenu->addAction(QIcon::fromTheme("document-save"), tr("&Save"));
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setToolTip(tr("Save compositor settings"));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(saveSettings()));
    
    fileMenu->addSeparator();
    
    QAction *quitAction = fileMenu->addAction(QIcon::fromTheme("application-exit"), tr("&Quit"));
    quitAction->setShortcut(QKeySequence::Quit);
    quitAction->setToolTip(tr("Close application"));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
    
    // Help Menu
    QMenu *helpMenu = ui->menubar->addMenu(tr("&Help"));
    
    QAction *aboutAction = helpMenu->addAction(QIcon::fromTheme("help-about"), tr("&About"));
    aboutAction->setToolTip(tr("About this application"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(showAbout()));
}

QString MainWindow::getComptonConfigPath(){
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    if(configDir.isEmpty()){
        configDir = QDir::homePath() + "/.config";
    }
    return configDir + "/lumina-desktop/compton.conf";
}

void MainWindow::saveSettings(){
    if(loading) return; // Don't save while loading
    
    // Save compositing enabled/disabled setting
    QSettings settings("lumina-desktop", "sessionsettings");
    settings.setValue("enableCompositing", !ui->check_disablecompton->isChecked());
    
    // Save compton configuration file
    QString configPath = getComptonConfigPath();
    QStringList configLines = ui->text_file->toPlainText().split("\n");
    
    // Ensure the directory exists
    QDir configDir = QFileInfo(configPath).absoluteDir();
    if(!configDir.exists()){
        configDir.mkpath(".");
    }
    
    // Write the configuration file
    bool success = LUtils::writeFile(configPath, configLines, true);
    
    if(success){
        ui->statusbar->showMessage(tr("Compositor settings saved successfully"), 3000);
    } else {
        ui->statusbar->showMessage(tr("Error: Could not save compositor settings"), 5000);
    }
}

void MainWindow::loadSettings(){
    loading = true;
    
    // Load compositing enabled/disabled setting
    QSettings settings("lumina-desktop", "sessionsettings");
    bool compositingEnabled = settings.value("enableCompositing", false).toBool();
    ui->check_disablecompton->setChecked(!compositingEnabled);
    
    // Load compton configuration file
    QString configPath = getComptonConfigPath();
    QStringList configLines = LUtils::readFile(configPath);
    
    if(configLines.isEmpty()){
        // Create a default configuration if none exists
        configLines << "# Compton Configuration File"
                   << "# This file configures the window compositor for Lumina Desktop"
                   << ""
                   << "# Backend to use: \"xrender\" or \"glx\"."
                   << "backend = \"xrender\";"
                   << ""
                   << "# Enable/disable VSync."
                   << "vsync = true;"
                   << ""
                   << "# Opacity settings"
                   << "inactive-opacity = 1.0;"
                   << "active-opacity = 1.0;"
                   << "frame-opacity = 1.0;"
                   << ""
                   << "# Shadow settings"
                   << "shadow = true;"
                   << "shadow-radius = 7;"
                   << "shadow-offset-x = -7;"
                   << "shadow-offset-y = -7;"
                   << "shadow-opacity = 0.7;"
                   << ""
                   << "# Fading settings"
                   << "fading = true;"
                   << "fade-delta = 4;"
                   << "fade-in-step = 0.03;"
                   << "fade-out-step = 0.03;"
                   << ""
                   << "# Window type settings"
                   << "wintypes:"
                   << "{"
                   << "  tooltip = { fade = true; shadow = true; opacity = 0.9; focus = true; };"
                   << "  dock = { shadow = false; };"
                   << "  dnd = { shadow = false; };"
                   << "  popup_menu = { opacity = 0.9; };"
                   << "  dropdown_menu = { opacity = 0.9; };"
                   << "};";
    }
    
    ui->text_file->setPlainText(configLines.join("\n"));
    
    loading = false;
    ui->statusbar->showMessage(tr("Compositor settings loaded"), 2000);
}

void MainWindow::settingChanged(){
    if(loading) return; // Don't trigger saves while loading
    
    // Restart the timer - this creates a delay so we don't save on every keystroke
    settingsTimer->stop();
    settingsTimer->start();
}

void MainWindow::showAbout(){
    QMessageBox::about(this, tr("About Compositor Settings"),
        tr("<h3>Lumina Compositor Settings</h3>"
           "<p>This application allows you to configure window effects and compositing for the Lumina Desktop Environment.</p>"
           "<p><b>Features:</b></p>"
           "<ul>"
           "<li>Enable/disable window compositing</li>"
           "<li>Edit compositor configuration directly</li>"
           "<li>Configure transparency, shadows, and animations</li>"
           "</ul>"
           "<p><b>Version:</b> %1<br>"
           "<b>Copyright:</b> 2024, jt(q5sys)<br>"
           "<b>License:</b> 3-clause BSD</p>"
           "<p>Changes to compositing settings require a session restart to take effect.</p>")
           .arg(qApp->applicationVersion()));
}
