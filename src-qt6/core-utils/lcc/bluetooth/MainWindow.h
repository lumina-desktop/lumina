//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details 
//===========================================
#ifndef _LCC_BLUETOOTH_MAINWINDOW_H
#define _LCC_BLUETOOTH_MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QSettings>
#include <QProcess>
#include <QListWidget>
#include <QListWidgetItem>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QCheckBox>
#include <QSpinBox>
#include <QComboBox>
#include <QTextEdit>
#include <QSplitter>
#include <QTabWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QAction>
#include <QApplication>
#include <QCloseEvent>

#include <LuminaXDG.h>
#include <LUtils.h>

namespace Ui{
    class MainWindow;
};

class MainWindow : public QMainWindow{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void slotSingleInstance(){
        this->show();
        this->raise();
        this->activateWindow();
    }

    void updateIcons();

private slots:
    void slotSaveSettings();
    void slotLoadSettings();
    void slotBluetoothToggle();
    void slotRefreshDevices();
    void slotPairDevice();
    void slotUnpairDevice();
    void slotConnectDevice();
    void slotDisconnectDevice();
    void slotDeviceSelectionChanged();
    void slotDiscoveryFinished();
    void slotProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void slotProcessError(QProcess::ProcessError error);
    void slotAbout();

protected:
    void closeEvent(QCloseEvent *ev){
        this->hide();
        ev->ignore(); //just hide the window
    }

private:
    Ui::MainWindow *ui;
    QTimer *saveTimer;
    QSettings *settings;
    QProcess *bluetoothProcess;
    
    // Bluetooth management
    bool bluetoothEnabled;
    QStringList availableDevices;
    QStringList pairedDevices;
    QStringList connectedDevices;
    
    void setupUI();
    void setupConnections();
    void setupMenus();
    void detectBluetoothSystem();
    void updateDeviceList();
    void updateBluetoothStatus();
    void runBluetoothCommand(const QString &command, const QStringList &args = QStringList());
    QString getBluetoothBackend();
    void showStatusMessage(const QString &message, int timeout = 3000);
};

#endif
