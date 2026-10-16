//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LCC_WALLPAPER_MAINWINDOW_H
#define _LCC_WALLPAPER_MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QComboBox>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>
#include <QFrame>
#include <QSpacerItem>
#include <QSettings>
#include <QFileDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QMenu>
#include <QApplication>
#include <QScreen>
#include <QGuiApplication>
#include <QPixmap>
#include <QIcon>
#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QDebug>
#include <QResizeEvent>

#include <LuminaXDG.h>
#include <LUtils.h>
#include <LuminaOS.h>

namespace Ui{
    class MainWindow;
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
    int cScreen, cBG; //current screen number/background
    QString DEFAULTBG;
    bool loading;

    QString getColorStyle(QString current, bool allowTransparency);
    void setupUI();

private slots:
    void updateMenus();
    void deskbgchanged();
    void desktimechanged();
    void deskbgremoved();
    void deskbgadded();
    void deskbgcoloradded();
    void deskbgdiradded();
    void deskbgdirradded();
    void showAbout();

protected:
    void resizeEvent(QResizeEvent*){
        deskbgchanged(); //update the wallpaper preview
    }
};
#endif
