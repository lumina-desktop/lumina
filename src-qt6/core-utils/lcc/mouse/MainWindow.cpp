//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details 
//===========================================
#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QX11Info>

#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);
    loading = false;
    
    // Initialize the save timer
    saveTimer = new QTimer(this);
    saveTimer->setSingleShot(true);
    saveTimer->setInterval(300); // 300ms delay
    connect(saveTimer, SIGNAL(timeout()), this, SLOT(saveSettings()));
    
    // Setup menu connections
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(saveSettings()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    
    // Setup UI connections
    setupConnections();
    
    // Load current settings
    loadSettings();
    
    // Set window properties
    this->setWindowTitle(tr("Mouse Settings"));
    this->setWindowIcon(QIcon::fromTheme("preferences-desktop-mouse"));
    
    // Show initial status
    ui->statusbar->showMessage(tr("Mouse settings loaded"), 2000);
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::slotSingleInstance(){
    this->show();
    this->raise();
    this->activateWindow();
}

void MainWindow::setupConnections(){
    // Connect all UI elements to the settings changed slot
    connect(ui->slider_sensitivity, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged()));
    connect(ui->slider_acceleration, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged()));
    connect(ui->spinBox_threshold, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged()));
    connect(ui->checkBox_leftHanded, SIGNAL(toggled(bool)), this, SLOT(settingsChanged()));
    connect(ui->slider_doubleClick, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged()));
    connect(ui->spinBox_scrollLines, SIGNAL(valueChanged(int)), this, SLOT(settingsChanged()));
    connect(ui->comboBox_device, SIGNAL(currentTextChanged(QString)), this, SLOT(settingsChanged()));
    
    // Connect the double-click slider to update the label
    connect(ui->slider_doubleClick, &QSlider::valueChanged, [this](int value){
        ui->label_doubleClickMs->setText(QString::number(value) + " ms");
    });
    
    // Connect reset button
    connect(ui->pushButton_defaults, SIGNAL(clicked()), this, SLOT(resetToDefaults()));
}

void MainWindow::loadSettings(){
    loading = true;
    
    QSettings settings("lumina-desktop", "lcc-mouse");
    
    // Load pointer settings
    ui->slider_sensitivity->setValue(settings.value("pointer/sensitivity", 10).toInt());
    ui->slider_acceleration->setValue(settings.value("pointer/acceleration", 10).toInt());
    ui->spinBox_threshold->setValue(settings.value("pointer/threshold", 4).toInt());
    
    // Load button settings
    ui->checkBox_leftHanded->setChecked(settings.value("buttons/leftHanded", false).toBool());
    int doubleClickTime = settings.value("buttons/doubleClickTime", 400).toInt();
    ui->slider_doubleClick->setValue(doubleClickTime);
    ui->label_doubleClickMs->setText(QString::number(doubleClickTime) + " ms");
    
    // Load wheel settings
    ui->spinBox_scrollLines->setValue(settings.value("wheel/scrollLines", 3).toInt());
    
    // Load advanced settings
    QString device = settings.value("advanced/device", "Auto-detect").toString();
    int deviceIndex = ui->comboBox_device->findText(device);
    if(deviceIndex >= 0){
        ui->comboBox_device->setCurrentIndex(deviceIndex);
    } else {
        ui->comboBox_device->setCurrentText(device);
    }
    
    loading = false;
}

void MainWindow::saveSettings(){
    if(loading) return;
    
    QSettings settings("lumina-desktop", "lcc-mouse");
    
    // Save pointer settings
    settings.setValue("pointer/sensitivity", ui->slider_sensitivity->value());
    settings.setValue("pointer/acceleration", ui->slider_acceleration->value());
    settings.setValue("pointer/threshold", ui->spinBox_threshold->value());
    
    // Save button settings
    settings.setValue("buttons/leftHanded", ui->checkBox_leftHanded->isChecked());
    settings.setValue("buttons/doubleClickTime", ui->slider_doubleClick->value());
    
    // Save wheel settings
    settings.setValue("wheel/scrollLines", ui->spinBox_scrollLines->value());
    
    // Save advanced settings
    settings.setValue("advanced/device", ui->comboBox_device->currentText());
    
    settings.sync();
    
    // Apply the settings to the system
    applyMouseSettings();
    
    ui->statusbar->showMessage(tr("Settings saved"), 2000);
}

void MainWindow::settingsChanged(){
    if(loading) return;
    
    // Update double-click time label if needed
    if(sender() == ui->slider_doubleClick){
        ui->label_doubleClickMs->setText(QString::number(ui->slider_doubleClick->value()) + " ms");
    }
    
    // Start/restart the save timer
    saveTimer->start();
}

void MainWindow::resetToDefaults(){
    loading = true;
    
    // Reset to default values
    ui->slider_sensitivity->setValue(10);
    ui->slider_acceleration->setValue(10);
    ui->spinBox_threshold->setValue(4);
    ui->checkBox_leftHanded->setChecked(false);
    ui->slider_doubleClick->setValue(400);
    ui->label_doubleClickMs->setText("400 ms");
    ui->spinBox_scrollLines->setValue(3);
    ui->comboBox_device->setCurrentIndex(0); // Auto-detect
    
    loading = false;
    
    // Save the defaults
    saveSettings();
    
    ui->statusbar->showMessage(tr("Settings reset to defaults"), 2000);
}

void MainWindow::showAbout(){
    QMessageBox::about(this, tr("About Mouse Settings"),
        tr("<h3>Lumina Mouse Settings</h3>"
           "<p>Configure mouse and pointer device settings for the Lumina Desktop Environment.</p>"
           "<p><b>Version:</b> %1<br>"
           "<b>License:</b> 3-clause BSD<br>"
           "<b>Copyright:</b> 2024, jt(q5sys)</p>"
           "<p>Part of the Lumina Control Center (LCC) suite.</p>")
           .arg(QApplication::applicationVersion()));
}

void MainWindow::applyMouseSettings(){
    // Apply settings using X11 calls
    if(!QX11Info::isPlatformX11()){
        qDebug() << "Not running on X11, mouse settings may not apply";
        return;
    }
    
    Display *display = XOpenDisplay(NULL);
    if(!display){
        qDebug() << "Could not open X11 display";
        return;
    }
    
    // Set pointer acceleration and threshold
    double sensitivity = ui->slider_sensitivity->value() / 10.0;
    double acceleration = ui->slider_acceleration->value() / 10.0;
    int threshold = ui->spinBox_threshold->value();
    
    XChangePointerControl(display, True, True, 
                         (int)(acceleration * 10), 10, threshold);
    
    // Set button mapping for left-handed mode
    setLeftHanded(ui->checkBox_leftHanded->isChecked());
    
    // Set double-click time
    setDoubleClickTime(ui->slider_doubleClick->value());
    
    // Set wheel scroll lines
    setWheelScrollLines(ui->spinBox_scrollLines->value());
    
    XCloseDisplay(display);
}

QString MainWindow::getMouseDevice(){
    QString device = ui->comboBox_device->currentText();
    if(device == "Auto-detect"){
        // Try to auto-detect mouse device
        QStringList candidates;
        candidates << "/dev/input/mice" << "/dev/psaux" << "/dev/mouse";
        
        for(const QString &candidate : candidates){
            if(QFileInfo::exists(candidate)){
                return candidate;
            }
        }
        return "/dev/input/mice"; // fallback
    }
    return device;
}

void MainWindow::setMouseAcceleration(double accel){
    if(!QX11Info::isPlatformX11()) return;
    
    Display *display = XOpenDisplay(NULL);
    if(!display) return;
    
    XChangePointerControl(display, True, False, (int)(accel * 10), 10, 0);
    XCloseDisplay(display);
}

void MainWindow::setMouseThreshold(int threshold){
    if(!QX11Info::isPlatformX11()) return;
    
    Display *display = XOpenDisplay(NULL);
    if(!display) return;
    
    XChangePointerControl(display, False, True, 0, 0, threshold);
    XCloseDisplay(display);
}

void MainWindow::setMouseSensitivity(double sensitivity){
    // Sensitivity is handled through acceleration in X11
    setMouseAcceleration(sensitivity);
}

void MainWindow::setLeftHanded(bool leftHanded){
    if(!QX11Info::isPlatformX11()) return;
    
    Display *display = XOpenDisplay(NULL);
    if(!display) return;
    
    // Get current button mapping
    unsigned char buttons[256];
    int nbuttons = XGetPointerMapping(display, buttons, 256);
    
    if(nbuttons >= 3){
        if(leftHanded){
            // Swap buttons 1 and 3 (left and right)
            unsigned char temp = buttons[0];
            buttons[0] = buttons[2];
            buttons[2] = temp;
        } else {
            // Standard mapping: 1, 2, 3
            buttons[0] = 1;
            buttons[1] = 2;
            buttons[2] = 3;
        }
        
        XSetPointerMapping(display, buttons, nbuttons);
    }
    
    XCloseDisplay(display);
}

void MainWindow::setDoubleClickTime(int time){
    // Set double-click time using Qt settings
    QSettings qtSettings(QSettings::UserScope, "Trolltech");
    qtSettings.beginGroup("Qt");
    qtSettings.setValue("doubleClickInterval", time);
    qtSettings.endGroup();
    qtSettings.sync();
    
    // Also set it for the current application
    QApplication::setDoubleClickInterval(time);
}

void MainWindow::setWheelScrollLines(int lines){
    // Set wheel scroll lines using Qt settings
    QSettings qtSettings(QSettings::UserScope, "Trolltech");
    qtSettings.beginGroup("Qt");
    qtSettings.setValue("wheelScrollLines", lines);
    qtSettings.endGroup();
    qtSettings.sync();
    
    // Also set it for the current application
    QApplication::setWheelScrollLines(lines);
}
