//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details 
//===========================================
#include "MainWindow.h"
#include <QApplication>
#include <QSignalBlocker>
#include "ui_MainWindow.h"

#include <QDir>
#include <QStandardPaths>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);
    
    // Initialize variables
    bluetoothEnabled = false;
    newBlueZ = true;
    bluetoothProcess = new QProcess(this);
    saveTimer = new QTimer(this);
    saveTimer->setSingleShot(true);
    saveTimer->setInterval(300);
    
    // Initialize settings
    QString settingsPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/lumina-desktop";
    QDir().mkpath(settingsPath);
    settings = new QSettings(settingsPath + "/lcc-bluetooth.conf", QSettings::IniFormat, this);
    
    setupUI();
    setupConnections();
    setupMenus();
    detectBluetoothSystem();
    slotLoadSettings();
    updateBluetoothStatus();
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::setupUI(){
    // Set window properties
    this->setWindowTitle(tr("Bluetooth Settings"));
    this->setWindowIcon(LXDG::findIcon("preferences-system-bluetooth", "bluetooth"));
    
    // Set initial tab
    ui->tabWidget->setCurrentIndex(0);
    
    // Configure list widgets
    ui->listWidget_available->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->listWidget_paired->setContextMenuPolicy(Qt::CustomContextMenu);
    
    // Set status bar
    showStatusMessage(tr("Ready"), 2000);
}

void MainWindow::setupConnections(){
    // Timer connections
    connect(saveTimer, SIGNAL(timeout()), this, SLOT(slotSaveSettings()));
    
    // Process connections
    connect(bluetoothProcess, SIGNAL(finished(int, QProcess::ExitStatus)), 
            this, SLOT(slotProcessFinished(int, QProcess::ExitStatus)));
    // QProcess::error(ProcessError) was removed in Qt6; errorOccurred exists since Qt 5.6
    connect(bluetoothProcess, SIGNAL(errorOccurred(QProcess::ProcessError)), 
            this, SLOT(slotProcessError(QProcess::ProcessError)));
    
    // UI connections
    connect(ui->checkBox_enable, SIGNAL(toggled(bool)), this, SLOT(slotBluetoothToggle()));
    connect(ui->pushButton_refresh, SIGNAL(clicked()), this, SLOT(slotRefreshDevices()));
    connect(ui->pushButton_pair, SIGNAL(clicked()), this, SLOT(slotPairDevice()));
    connect(ui->pushButton_unpair, SIGNAL(clicked()), this, SLOT(slotUnpairDevice()));
    connect(ui->pushButton_connect, SIGNAL(clicked()), this, SLOT(slotConnectDevice()));
    connect(ui->pushButton_disconnect, SIGNAL(clicked()), this, SLOT(slotDisconnectDevice()));
    
    // List widget connections
    connect(ui->listWidget_available, SIGNAL(itemSelectionChanged()), 
            this, SLOT(slotDeviceSelectionChanged()));
    connect(ui->listWidget_paired, SIGNAL(itemSelectionChanged()), 
            this, SLOT(slotDeviceSelectionChanged()));
    
    // Settings connections
    connect(ui->checkBox_discoverable, SIGNAL(toggled(bool)), saveTimer, SLOT(start()));
    connect(ui->spinBox_timeout, SIGNAL(valueChanged(int)), saveTimer, SLOT(start()));
}

void MainWindow::setupMenus(){
    // File menu actions
    connect(ui->action_Save, SIGNAL(triggered()), this, SLOT(slotSaveSettings()));
    connect(ui->action_Quit, SIGNAL(triggered()), qApp, SLOT(quit()));
    
    // Help menu actions
    connect(ui->action_About, SIGNAL(triggered()), this, SLOT(slotAbout()));
}

void MainWindow::detectBluetoothSystem(){
    QString backend = getBluetoothBackend();
    ui->label_backend_value->setText(backend);
    
    if(backend == "None detected"){
        ui->checkBox_enable->setEnabled(false);
        ui->pushButton_refresh->setEnabled(false);
        ui->tabWidget->setEnabled(false);
        showStatusMessage(tr("No bluetooth system detected"), 5000);
        ui->textEdit_log->append(tr("No bluetooth management system found."));
        ui->textEdit_log->append(tr("Supported systems: bluetoothctl (BlueZ), hcitool, rfcomm"));
    } else {
        ui->textEdit_log->append(tr("Detected bluetooth backend: %1").arg(backend));
    }
}

QString MainWindow::getBluetoothBackend(){
    //Detected once and cached: this is called on every button press/timer, and used to run up to three
    // blocking "which" processes each time (and "which" itself may not be installed)
    if(!backendCache.isEmpty()){ return backendCache; }
    newBlueZ = true;
    if(!QStandardPaths::findExecutable("bluetoothctl").isEmpty()){
        backendCache = "BlueZ (bluetoothctl)";
        //"paired-devices" was removed in BlueZ 5.65 ("devices Paired" replaces it)
        QProcess ver; ver.start("bluetoothctl", QStringList() << "--version"); ver.waitForFinished(2000);
        QString v = QString(ver.readAllStandardOutput()).section(":",-1).trimmed(); //"bluetoothctl: 5.88"
        if(!v.isEmpty()){ newBlueZ = (v.section(".",0,0).toInt() > 5) || (v.section(".",0,0).toInt()==5 && v.section(".",1,1).toInt() >= 65); }
    }else if(!QStandardPaths::findExecutable("hcitool").isEmpty()){ backendCache = "BlueZ (hcitool)"; }
    else if(!QStandardPaths::findExecutable("hccontrol").isEmpty()){ backendCache = "FreeBSD (hccontrol)"; }
    else{ backendCache = "None detected"; }
    return backendCache;
}

void MainWindow::updateBluetoothStatus(){
    QString backend = getBluetoothBackend();
    
    if(backend.contains("BlueZ")){
        // Use bluetoothctl to check status
        runBluetoothCommand("bluetoothctl", QStringList() << "show");
    } else if(backend.contains("FreeBSD")){
        // Use FreeBSD tools to check status
        runBluetoothCommand("hccontrol", QStringList() << "read_local_name");
    } else {
        ui->label_status->setText(tr("Status: No bluetooth system"));
        bluetoothEnabled = false;
        ui->checkBox_enable->setChecked(false);
    }
}

void MainWindow::updateDeviceList(){
    QString backend = getBluetoothBackend();
    
    ui->listWidget_available->clear();
    ui->listWidget_paired->clear();
    
    if(backend.contains("BlueZ")){
        // Scan for devices: a bounded scan that ends by itself (bluetoothctl stops a client's discovery when it
        // exits), then list what was found. These are queued, so each runs after the previous one finished
        // (they used to kill each other: "scan on" died as soon as the next command started).
        runBluetoothCommand("bluetoothctl", QStringList() << "--timeout" << "5" << "scan" << "on");
        runBluetoothCommand("bluetoothctl", QStringList() << "devices");
        // Get paired devices
        if(newBlueZ){ runBluetoothCommand("bluetoothctl", QStringList() << "devices" << "Paired"); }
        else{ runBluetoothCommand("bluetoothctl", QStringList() << "paired-devices"); }
    } else if(backend.contains("FreeBSD")){
        // FreeBSD bluetooth device discovery
        runBluetoothCommand("hccontrol", QStringList() << "inquiry");
    }
    
    showStatusMessage(tr("Scanning for devices..."), 5000);
}

void MainWindow::runBluetoothCommand(const QString &command, const QStringList &args){
    cmdQueue << qMakePair(command, args);
    if(bluetoothProcess->state() == QProcess::NotRunning){ startNextCommand(); }
}

void MainWindow::startNextCommand(){
    if(cmdQueue.isEmpty()){ runningArgs.clear(); return; }
    QPair<QString, QStringList> next = cmdQueue.takeFirst();
    runningArgs = next.second;
    ui->textEdit_log->append(tr("Running: %1 %2").arg(next.first).arg(next.second.join(" ")));
    bluetoothProcess->start(next.first, next.second);
}

void MainWindow::showStatusMessage(const QString &message, int timeout){
    ui->statusbar->showMessage(message, timeout);
}

void MainWindow::slotSaveSettings(){
    if(!settings) return;
    
    settings->setValue("bluetooth/enabled", ui->checkBox_enable->isChecked());
    settings->setValue("bluetooth/discoverable", ui->checkBox_discoverable->isChecked());
    settings->setValue("bluetooth/discovery_timeout", ui->spinBox_timeout->value());
    
    settings->sync();
    showStatusMessage(tr("Settings saved"), 2000);
}

void MainWindow::slotLoadSettings(){
    if(!settings) return;
    
    //Restore the checkbox without triggering slotBluetoothToggle() - that ran "bluetoothctl power on" just
    // from opening the tool (the real state is read back by "show" in updateBluetoothStatus())
    { QSignalBlocker block(ui->checkBox_enable); ui->checkBox_enable->setChecked(settings->value("bluetooth/enabled", false).toBool()); }
    ui->checkBox_discoverable->setChecked(settings->value("bluetooth/discoverable", false).toBool());
    ui->spinBox_timeout->setValue(settings->value("bluetooth/discovery_timeout", 30).toInt());
}

void MainWindow::slotBluetoothToggle(){
    bool enable = ui->checkBox_enable->isChecked();
    QString backend = getBluetoothBackend();
    
    if(backend.contains("BlueZ")){
        if(enable){
            runBluetoothCommand("bluetoothctl", QStringList() << "power" << "on");
        } else {
            runBluetoothCommand("bluetoothctl", QStringList() << "power" << "off");
        }
    } else if(backend.contains("FreeBSD")){
        if(enable){
            runBluetoothCommand("service", QStringList() << "bluetooth" << "start");
        } else {
            runBluetoothCommand("service", QStringList() << "bluetooth" << "stop");
        }
    }
    
    bluetoothEnabled = enable;
    ui->tabWidget->setEnabled(enable);
    
    if(enable){
        showStatusMessage(tr("Enabling bluetooth..."), 3000);
        QTimer::singleShot(2000, this, SLOT(slotRefreshDevices()));
    } else {
        showStatusMessage(tr("Disabling bluetooth..."), 3000);
        ui->listWidget_available->clear();
        ui->listWidget_paired->clear();
    }
    
    saveTimer->start();
}

void MainWindow::slotRefreshDevices(){
    if(!bluetoothEnabled){
        showStatusMessage(tr("Bluetooth is disabled"), 2000);
        return;
    }
    
    updateDeviceList();
}

void MainWindow::slotPairDevice(){
    QListWidgetItem *item = ui->listWidget_available->currentItem();
    if(!item) return;
    
    QString deviceInfo = item->text();
    QString deviceAddress = deviceInfo.split(" ").first(); // Assume format "AA:BB:CC:DD:EE:FF Device Name"
    
    QString backend = getBluetoothBackend();
    if(backend.contains("BlueZ")){
        runBluetoothCommand("bluetoothctl", QStringList() << "pair" << deviceAddress);
    } else if(backend.contains("FreeBSD")){
        runBluetoothCommand("hccontrol", QStringList() << "create_connection" << deviceAddress);
    }
    
    showStatusMessage(tr("Pairing with device..."), 5000);
}

void MainWindow::slotUnpairDevice(){
    QListWidgetItem *item = ui->listWidget_paired->currentItem();
    if(!item) return;
    
    QString deviceInfo = item->text();
    QString deviceAddress = deviceInfo.split(" ").first();
    
    QString backend = getBluetoothBackend();
    if(backend.contains("BlueZ")){
        runBluetoothCommand("bluetoothctl", QStringList() << "remove" << deviceAddress);
    }
    
    showStatusMessage(tr("Unpairing device..."), 3000);
}

void MainWindow::slotConnectDevice(){
    QListWidgetItem *item = ui->listWidget_paired->currentItem();
    if(!item) return;
    
    QString deviceInfo = item->text();
    QString deviceAddress = deviceInfo.split(" ").first();
    
    QString backend = getBluetoothBackend();
    if(backend.contains("BlueZ")){
        runBluetoothCommand("bluetoothctl", QStringList() << "connect" << deviceAddress);
    }
    
    showStatusMessage(tr("Connecting to device..."), 3000);
}

void MainWindow::slotDisconnectDevice(){
    QListWidgetItem *item = ui->listWidget_paired->currentItem();
    if(!item) return;
    
    QString deviceInfo = item->text();
    QString deviceAddress = deviceInfo.split(" ").first();
    
    QString backend = getBluetoothBackend();
    if(backend.contains("BlueZ")){
        runBluetoothCommand("bluetoothctl", QStringList() << "disconnect" << deviceAddress);
    }
    
    showStatusMessage(tr("Disconnecting device..."), 3000);
}

void MainWindow::slotDeviceSelectionChanged(){
    // Enable/disable buttons based on selection
    ui->pushButton_pair->setEnabled(ui->listWidget_available->currentItem() != nullptr);
    
    bool pairedSelected = (ui->listWidget_paired->currentItem() != nullptr);
    ui->pushButton_unpair->setEnabled(pairedSelected);
    ui->pushButton_connect->setEnabled(pairedSelected);
    ui->pushButton_disconnect->setEnabled(pairedSelected);
}

void MainWindow::slotDiscoveryFinished(){
    //(the scan is now bounded with --timeout and followed by a queued "devices" - see updateDeviceList())
    showStatusMessage(tr("Device scan completed"), 2000);
}

void MainWindow::slotProcessFinished(int exitCode, QProcess::ExitStatus exitStatus){
    QString output = bluetoothProcess->readAllStandardOutput();
    QString error = bluetoothProcess->readAllStandardError();
    
    //Which command produced this output (it used to be guessed from the text: "paired-devices" output contains
    // no "Paired", so paired devices landed in the available list)
    const QStringList args = runningArgs;
    bool pairedList = args.contains("paired-devices") || (args.contains("devices") && args.contains("Paired"));
    bool availList = !pairedList && args.contains("devices");
    //Replace (not append to) the list with each fresh result: queued refreshes otherwise duplicated every entry
    if(availList){ ui->listWidget_available->clear(); }
    if(pairedList){ ui->listWidget_paired->clear(); }
    if(!output.isEmpty()){
        ui->textEdit_log->append(tr("Output: %1").arg(output));
        
        // Parse output for device information
        if(availList && output.contains("Device ")){
            QStringList lines = output.split("\n");
            for(const QString &line : lines){
                if(line.contains("Device ") && line.contains(":")){
                    QString deviceLine = line.trimmed();
                    if(deviceLine.startsWith("Device ")){
                        deviceLine = deviceLine.mid(7); // Remove "Device " prefix
                        ui->listWidget_available->addItem(deviceLine);
                    }
                }
            }
        }
        
        // Parse paired devices
        if(pairedList){
            QStringList lines = output.split("\n");
            for(const QString &line : lines){
                if(line.contains("Device ") && line.contains(":")){
                    QString deviceLine = line.trimmed();
                    if(deviceLine.startsWith("Device ")){
                        deviceLine = deviceLine.mid(7);
                        ui->listWidget_paired->addItem(deviceLine);
                    }
                }
            }
        }
        
        // Update status based on output
        if(output.contains("Powered: yes")){
            ui->label_status->setText(tr("Status: Enabled"));
            bluetoothEnabled = true;
        } else if(output.contains("Powered: no")){
            ui->label_status->setText(tr("Status: Disabled"));
            bluetoothEnabled = false;
        }
        if(args.contains("show")){
            //Reflect the real power state without toggling it
            QSignalBlocker block(ui->checkBox_enable);
            ui->checkBox_enable->setChecked(bluetoothEnabled);
            ui->tabWidget->setEnabled(bluetoothEnabled);
        }
    }
    
    if(!error.isEmpty()){
        ui->textEdit_log->append(tr("Error: %1").arg(error));
    }
    
    if(exitStatus != QProcess::NormalExit || exitCode != 0){
        ui->textEdit_log->append(tr("Command failed with exit code: %1").arg(exitCode));
    }
    startNextCommand(); //run whatever was queued meanwhile
}

void MainWindow::slotProcessError(QProcess::ProcessError error){
    QString errorString;
    switch(error){
        case QProcess::FailedToStart:
            errorString = tr("Failed to start bluetooth command");
            break;
        case QProcess::Crashed:
            errorString = tr("Bluetooth command crashed");
            break;
        case QProcess::Timedout:
            errorString = tr("Bluetooth command timed out");
            break;
        default:
            errorString = tr("Unknown bluetooth command error");
            break;
    }
    
    ui->textEdit_log->append(tr("Process Error: %1").arg(errorString));
    showStatusMessage(errorString, 5000);
}

void MainWindow::slotAbout(){
    QMessageBox::about(this, tr("About Bluetooth Settings"),
        tr("Bluetooth Settings\n\n"
           "A cross-platform bluetooth configuration utility for FreeBSD and Linux.\n"
           "Part of the Lumina Control Center (LCC) suite.\n\n"
           "Copyright (c) 2024, jt(q5sys)\n"
           "Available under the 3-clause BSD license"));
}

void MainWindow::updateIcons(){
    // Update all icons when theme changes
    this->setWindowIcon(LXDG::findIcon("preferences-system-bluetooth", "bluetooth"));
    ui->action_Save->setIcon(LXDG::findIcon("document-save", ""));
    ui->action_Quit->setIcon(LXDG::findIcon("application-exit", ""));
    ui->action_About->setIcon(LXDG::findIcon("help-about", ""));
    ui->pushButton_refresh->setIcon(LXDG::findIcon("view-refresh", ""));
    ui->pushButton_pair->setIcon(LXDG::findIcon("list-add", ""));
    ui->pushButton_unpair->setIcon(LXDG::findIcon("list-remove", ""));
    ui->pushButton_connect->setIcon(LXDG::findIcon("network-connect", ""));
    ui->pushButton_disconnect->setIcon(LXDG::findIcon("network-disconnect", ""));
}
