//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LCC_LOCALE_MAINWINDOW_H
#define _LCC_LOCALE_MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QComboBox>
#include <QLabel>
#include <QStatusBar>
#include <QMenuBar>
#include <QAction>
#include <QMessageBox>
#include <QApplication>
#include <QLocale>
#include <QKeySequence>

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
    void slotSingleInstance();

private slots:
    void saveSettings();
    void loadSettings();
    void settingChanged();
    void showAbout();

private:
    Ui::MainWindow *ui;
    
    void setupLocales();
    void setupMenus();
    void setupConnections();
    void updateStatusBar(const QString &message);
};

#endif
