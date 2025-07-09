//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LCC_DESKTOP_MAINWINDOW_H
#define _LCC_DESKTOP_MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLabel>
#include <QCheckBox>
#include <QSettings>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QApplication>
#include <QScreen>
#include <QGuiApplication>
#include <QIcon>
#include <QDebug>
#include <QTimer>
#include <QDialog>
#include <QStringList>
#include <QFileDialog>
#include <QHash>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QAbstractButton>

#include <LuminaXDG.h>
#include <LUtils.h>
#include <LuminaOS.h>

namespace Ui{
    class MainWindow;
};

// Lumina Plugin Info class 
class LPI{
public:
    QString name, ID, description, icon;
    LPI(){}
    ~LPI(){}
};

// Plugin information class for desktop plugins
class LPlugins{
public:
    LPlugins();
    ~LPlugins();
    
    QStringList desktopPlugins();
    LPI desktopPluginInfo(QString);

private:
    QHash<QString, LPI> DESKTOP;
    void LoadDesktopPlugins();
};

// Plugin selection dialog
class GetPluginDialog : public QDialog{
    Q_OBJECT
public:
    GetPluginDialog(QWidget* parent = 0);
    ~GetPluginDialog();
    
    void LoadPlugins(QString type, LPlugins *DB);

    bool selected; //this is set to true if a plugin was selected by the user
    QString plugID; //this is set to the ID of the selected plugin

private:
    QListWidget *listWidget;
    QLabel *descLabel;

private slots:
    void pluginchanged();
    void accept();
};

// Application selection dialog
class AppDialog : public QDialog{
    Q_OBJECT
public:
    AppDialog(QWidget *parent = 0, QString defaultPath = "");
    ~AppDialog();
    
    void allowReset(bool allow);

    QString appselected; //selected application
    bool appreset; //Did the user select to reset to defaults?

private:
    QListWidget *listApps;
    QLineEdit *lineSearch;
    QDialogButtonBox *buttonBox;

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_buttonBox_clicked(QAbstractButton *button);
    void on_listApps_itemDoubleClicked(QListWidgetItem *item);
    void on_lineSearch_textChanged(const QString &term);
};

class MainWindow : public QMainWindow{
    Q_OBJECT
public:
    MainWindow(int screen = 0, QWidget *parent = 0);
    ~MainWindow();

public slots:
    void SaveSettings();
    void LoadSettings();
    void updateIcons();

private:
    Ui::MainWindow *ui;
    int cscreen; //current monitor/screen number
    LPlugins *PINFO;
    bool loading;

    //Get an application on the system
    QString getSysApp(bool allowreset = false);
    void setupMenus();

private slots:
    void deskplugadded();
    void deskplugremoved();
    void settingChanged();
    void showAbout();
};
#endif
