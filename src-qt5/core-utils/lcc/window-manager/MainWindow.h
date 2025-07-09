//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details 
//===========================================
#ifndef _LCC_WINDOW_MANAGER_MAINWINDOW_H
#define _LCC_WINDOW_MANAGER_MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QStackedWidget>
#include <QRadioButton>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QGroupBox>
#include <QScrollArea>
#include <QPlainTextEdit>
#include <QLine>
#include <QFrame>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QMessageBox>
#include <QApplication>
#include <QTimer>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QPixmap>

#include <LuminaOS.h>

namespace Ui{
    class MainWindow;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void updateIcons();

private slots:
    void saveSettings();
    void showAbout();
    void settingChanged();
    void sessionThemeChanged();
    void switchEditor();

private:
    Ui::MainWindow *ui;
    bool loading;

    // File operations
    QStringList readFile(QString path);
    bool overwriteFile(QString path, QStringList contents);
    
    // UI setup
    void setupMenus();
    void setupConnections();
    void loadSettings();
    void showStatusMessage(const QString &message, int timeout = 3000);
};

#endif
