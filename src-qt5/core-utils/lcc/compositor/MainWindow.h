//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details 
//===========================================
#ifndef _LCC_COMPOSITOR_MAINWINDOW_H
#define _LCC_COMPOSITOR_MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QTimer>
#include <QPlainTextEdit>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QStatusBar>
#include <QAction>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDebug>

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
    void saveSettings();
    void loadSettings();
    void settingChanged();
    void showAbout();

private:
    Ui::MainWindow *ui;
    QTimer *settingsTimer;
    bool loading;
    
    void setupConnections();
    void setupMenus();
    QString getComptonConfigPath();
};

#endif
