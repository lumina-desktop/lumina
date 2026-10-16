//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, JT (q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details 
//===========================================
#ifndef _LCC_SOUND_THEME_MAINWINDOW_H
#define _LCC_SOUND_THEME_MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QFileDialog>
#include <QTimer>
#include <QDebug>

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

private:
    Ui::MainWindow *ui;
    QSettings *sessionsettings;
    QTimer *saveTimer;
    bool hasChanges;

    void LoadSettings();
    void SaveSettings();
    void settingChanged();

private slots:
    void on_pushButton_startup_clicked();
    void on_pushButton_logout_clicked();
    void on_pushButton_battery_clicked();
    void on_checkBox_startup_toggled(bool checked);
    void on_checkBox_logout_toggled(bool checked);
    void on_checkBox_battery_toggled(bool checked);
    
    void saveTimerTimeout();
    void showAbout();
};

#endif
