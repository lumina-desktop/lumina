//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LCC_KEYBOARD_MAINWINDOW_H
#define _LCC_KEYBOARD_MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
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
#include <QKeySequenceEdit>
#include <QKeySequence>
#include <QRadioButton>
#include <QStackedWidget>
#include <QPlainTextEdit>
#include <QLabel>
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <QStatusBar>
#include <QMenuBar>

#include <LuminaXDG.h>
#include <LUtils.h>
#include <LuminaOS.h>

namespace Ui{
    class MainWindow;
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

    //Convert to/from fluxbox keyboard shortcuts
    QString dispToFluxKeys(QString);
    QString fluxToDispKeys(QString);

    //Read/overwrite a text file
    QStringList readFile(QString path);
    bool overwriteFile(QString path, QStringList contents);

    void setupUI();

private slots:
    void clearKeyBinding();
    void applyKeyBinding();
    void updateKeyConfig();
    void switchEditor();
    void togglesyntaxgroup();
    void settingChanged();
    void showAbout();
};
#endif
