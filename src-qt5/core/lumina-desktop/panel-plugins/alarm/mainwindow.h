//===========================================
//  qalarm source code
//  Copyright (c) 2017, q5sys
//  Available under the MIT License
//  See the LICENSE file for full details
//===========================================


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QDialog>
#include <QTimer>
#include <QString>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QObject>
#include <QSettings>
#include <QMediaPlayer>
#include <QDir>

namespace Ui {
class mainWindow;
}

class mainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit mainWindow(QWidget *parent = 0);
    ~mainWindow();

    QDateTime customDateTime;
    QString nowDateTime;
    QString userDateTime;
    int volume;

public slots:
//    void setVolume(int);
    void setCustomTime();

private:
    Ui::mainWindow *ui;
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    QTimer *rechecktimer;
    QMediaPlayer *alarmFile;
    QString alarmFilePath;
    void closeEvent(QCloseEvent*);
    void setClock();

private slots:
    void showWindow();
    void showWindow(QSystemTrayIcon::ActivationReason);
    void quitApp();
    void testAlarm();
    void syncClock();
    void alarmCheck();
    void tripAlarm();
    void startTimer();
};

#endif // MAINWINDOW_H


