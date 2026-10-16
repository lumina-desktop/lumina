//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_LCC_MENU_MAINWINDOW_H
#define _LUMINA_LCC_MENU_MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QListWidget>
#include <QListWidgetItem>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QMessageBox>
#include <QApplication>
#include <QDir>
#include <QStringList>

#include <LuminaXDG.h>
#include <LUtils.h>

#include "LPlugins.h"
#include "GetPluginDialog.h"
#include "AppDialog.h"
#include "ScriptDialog.h"

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
    void updateIcons();

private slots:
    void saveSettings();
    void loadSettings();
    void showAbout();
    void addMenuPlugin();
    void removeMenuPlugin();
    void moveMenuPluginUp();
    void moveMenuPluginDown();
    void checkMenuIcons();

private:
    Ui::MainWindow *ui;
    LPlugins *PINFO;
    
    void setupConnections();
    void setupMenuBar();
    void setupStatusBar();
    QString getSysApp(bool allowreset = false);
};

#endif
