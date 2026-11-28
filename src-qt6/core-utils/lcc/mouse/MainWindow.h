//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details 
//===========================================
#ifndef _LUMINA_CONFIG_MOUSE_H
#define _LUMINA_CONFIG_MOUSE_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QMainWindow>
#include <QTimer>
#include <QSettings>
#include <QSlider>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QApplication>
#include <QProcess>

#include <LUtils.h>
#include <LuminaOS.h>

namespace Ui{
    class MainWindow;
};

class MainWindow : public QMainWindow{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void slotSingleInstance();

private slots:
    void saveSettings();
    void loadSettings();
    void settingsChanged();
    void resetToDefaults();
    void showAbout();

private:
    Ui::MainWindow *ui;
    QTimer *saveTimer;
    bool loading;
    
    void setupConnections();
    void applyMouseSettings();
    QString getMouseDevice();
    void setMouseAcceleration(double accel);
    void setMouseThreshold(int threshold);
    void setMouseSensitivity(double sensitivity);
    void setLeftHanded(bool leftHanded);
    void setDoubleClickTime(int time);
    void setWheelScrollLines(int lines);
};

#endif
