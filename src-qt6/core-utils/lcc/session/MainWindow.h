//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LCC_SESSION_MAINWINDOW_H
#define _LCC_SESSION_MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QTimer>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QFileDialog>
#include <QImageReader>
#include <QPixmap>
#include <QMessageBox>
#include <QTime>
#include <QDate>
#include <QDir>
#include <QFile>
#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QKeySequence>

#include <LuminaXDG.h>
#include <LUtils.h>
#include <LDesktopUtils.h>

namespace Ui{
    class MainWindow;
};

class MainWindow : public QMainWindow{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void LoadSettings();
    void SaveSettings();
    void updateIcons();

private slots:
    void settingChanged();
    void sessionChangeUserIcon();
    void sessionResetSys();
    void sessionResetLumina();
    void sessionLoadTimeSample();
    void sessionShowTimeCodes();
    void sessionLoadDateSample();
    void sessionShowDateCodes();

private:
    Ui::MainWindow *ui;
    bool loading;
    
    void FindWindowManagerOptions();
    bool verifySettingsReset();
    void setupConnections();
    void setupMenus();
};

#endif
