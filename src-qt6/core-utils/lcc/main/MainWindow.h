//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LCC_MAIN_WINDOW_H
#define _LCC_MAIN_WINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QShortcut>
#include <QTimer>
#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QDebug>

#include <LuminaXDG.h>
#include <LUtils.h>
#include <LuminaOS.h>

namespace Ui{
    class MainWindow;
};

//Structure for application information
struct APPINFO{
    QString name, title, icon, comment, category, id;
    QStringList search_tags;
    bool isLCC;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void slotSingleInstance();

private slots:
    void showFind();
    void itemTriggered(QTreeWidgetItem*, int);
    void searchChanged(QString);
    void showAbout();

private:
    Ui::MainWindow *ui;
    QList<APPINFO> INFO;
    QShortcut *findShort;
    
    void LoadApplications();
    void UpdateItems(QString search = "");
    QList<APPINFO> GetLCCApplications();
    QList<APPINFO> GetSystemApplications();
    APPINFO CreateAppInfo(QString id, QString name, QString title, QString icon, QString comment, QString category, QStringList tags, bool isLCC = false);
};

#endif
