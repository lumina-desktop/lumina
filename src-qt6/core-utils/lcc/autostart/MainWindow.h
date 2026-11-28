//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LCC_AUTOSTART_MAINWINDOW_H
#define _LCC_AUTOSTART_MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QToolButton>
#include <QGroupBox>
#include <QSpacerItem>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>
#include <QApplication>
#include <QIcon>
#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QDebug>
#include <QDialog>
#include <QAbstractButton>
#include <QDialogButtonBox>
#include <QLineEdit>

#include <LuminaXDG.h>
#include <LUtils.h>
#include <LuminaOS.h>

namespace Ui{
    class MainWindow;
};

class AppDialog : public QDialog{
    Q_OBJECT
private:
    QListWidget *listApps;
    QDialogButtonBox *buttonBox;
    QLineEdit *lineSearch;
    XDGDesktopList *appsList;

public:
    AppDialog(QWidget *parent = 0, QString defaultPath = "");
    ~AppDialog();

    void allowReset(bool allow);

    QString appselected; //selected application
    bool appreset; //Did the user select to reset to defaults?

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
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void SaveSettings();
    void LoadSettings();
    void updateIcons();

private:
    Ui::MainWindow *ui;
    bool loading;

    QString getSysApp(bool allowreset);
    void setupUI();

private slots:
    void rmsessionstartitem();
    void addsessionstartapp();
    void addsessionstartbin();
    void addsessionstartfile();
    void settingChanged();
    void showAbout();
};
#endif
