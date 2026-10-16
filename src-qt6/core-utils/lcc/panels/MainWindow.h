//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_LCC_PANELS_MAINWINDOW_H
#define _LUMINA_LCC_PANELS_MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QTimer>
#include <QHBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QInputDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QStatusBar>
#include <QAction>
#include <QMenu>

#include <LUtils.h>
#include <LuminaXDG.h>

// Forward declarations
class LPlugins;
class PanelWidget;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(int screen = 0, QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void SaveSettings();
    void LoadSettings();
    void updateIcons();

private slots:
    void panelValChanged();
    void newPanel();
    void removePanel(int panelNum);
    void applyProfile(QAction *act);
    void applyImport(QAction *act);
    void applyImport(QString fromID);
    void showAbout();
    void screenChanged();

private:
    Ui::MainWindow *ui;
    bool loading;
    int cscreen; // current monitor/screen number
    QSettings *settings;
    LPlugins *PINFO;
    QList<PanelWidget*> PANELS;

    void setupMenus();
    void setupProfiles();
    void setupScreenSelector();
    void settingChanged();
};

#endif // _LUMINA_LCC_PANELS_MAINWINDOW_H
