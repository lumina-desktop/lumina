//===========================================
//  qalarm source code
//  Copyright (c) 2017, q5sys
//  Available under the MIT License
//  See the LICENSE file for full details
//===========================================

#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "ui_snooze.h"
#include <QMenu>
#include <QtMultimedia/QMediaPlayer>
#include <QCloseEvent>
#include <QDebug>

mainWindow::mainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::mainWindow){
    ui->setupUi(this);
    trayIcon=new QSystemTrayIcon(this);
    trayIconMenu=new QMenu(this);
    QAction *actionShow=new QAction("&Show",this);
    QAction *actionQuit=new QAction("&Quit",this);

    trayIconMenu->addAction(actionShow);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(actionQuit);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(QIcon(":/media/alarm.svg"));
    trayIcon->setToolTip("Alarm");
    trayIcon->show();
    setClock();

    connect(actionQuit,SIGNAL(triggered()),this,SLOT(quitApp()));
    connect(actionShow,SIGNAL(triggered()),this,SLOT(showWindow()));
    connect(trayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(ShowWindow(QSystemTrayIcon::ActivationReason)));
    connect(ui->pushButtonActivate,SIGNAL(clicked()),this,SLOT(SetCustomTime()));
    connect(ui->pushButtonTest, SIGNAL(clicked()), this,SLOT(testAlarm()));
//  QTimer *countDownTimer = new QTimer(this); // This alarm not written yet.
//  connect (countDownTimer, SIGNAL(timeout()), this, SLOT(tripAlarm()));

    //Set Volume
    int volume = 25;
    ui->volumeSlider->setValue(volume<=0? 50:volume);
}

mainWindow::~mainWindow(){
    delete ui;
}

void mainWindow::closeEvent(QCloseEvent * event){
    event->ignore();
    this->hide();
}

void mainWindow::tripAlarm(){
    QMessageBox *alarmBox = new QMessageBox;
    alarmBox->setText(tr("ALARM"));
    alarmFile = new QMediaPlayer(this);
    alarmFile->setMedia(QUrl::fromLocalFile(":/media/alarm.mp3"));
    alarmFile->setVolume(volume);
    alarmFile->play();
    QAbstractButton* pushButtonStop = alarmBox->addButton(tr("Stop Alarm"), QMessageBox::YesRole);
    alarmBox->addButton(tr("Snooze for 5 Minutes"), QMessageBox::NoRole);
    alarmBox->exec();
    if (alarmBox->clickedButton()==pushButtonStop){
        alarmFile->stop();
        }
    else{
        alarmFile->stop();
        QTimer *snoozeabit = new QTimer(this);
        connect(snoozeabit,SIGNAL(timeout()), this, SLOT(tripAlarm()));
        snoozeabit->start(50000);
        }
}

void mainWindow::testAlarm(){
     QMessageBox *alarmTestBox = new QMessageBox;
     alarmTestBox->setText("Ba Doop A Doop");
     alarmFile = new QMediaPlayer(this);
     alarmFile->setMedia(QUrl(":/media/alarm.mp3"));
     alarmFile->setVolume(volume);
     alarmFile->play();
     QAbstractButton* pushButtonStop = alarmTestBox->addButton(tr("Stop Alarm"), QMessageBox::YesRole);
     alarmTestBox->addButton(tr("Snooze for 5 Minutes"), QMessageBox::NoRole);
     alarmTestBox->exec();
     if (alarmTestBox->clickedButton()==pushButtonStop){
         alarmFile->stop();
         }
     else{
         alarmFile->stop();
         QTimer *snoozeabit = new QTimer(this);
         connect(snoozeabit,SIGNAL(timeout()), this, SLOT(tripAlarm()));
         snoozeabit->start(50000);
         }
 }ur ue.

void mainWindow::setClock(){
    //Sync Clock in App with System Clock
    QTimer *CurrentTime=new QTimer(this);
    connect(CurrentTime,SIGNAL(timeout()),this,SLOT(syncClock()));
    CurrentTime->start(500);
}

void mainWindow::showWindow(QSystemTrayIcon::ActivationReason Reason){
    if(Reason==QSystemTrayIcon::DoubleClick || Reason==QSystemTrayIcon::Trigger)
    {
        showWindow();
    }
}

void mainWindow::showWindow(){
    this->show();
}

void mainWindow::setCustomTime()
{
    ui->alarmDateTime->setDate(ui->calendarWidget->selectedDate());
    qDebug() << "selected date" << ui->calendarWidget->selectedDate();
    customDateTime.setTime(ui->alarmDateTime->time());  //this is not working yet
    customDateTime.setDate(ui->calendarWidget->selectedDate()); //this is not working yet
    qDebug() << "customDateTime" << customDateTime.toString("d MMMM yyyy hh:mm:ss");
    userDateTime = customDateTime.toString("d MMMM yyyy hh:mm:ss");
    qDebug() << "userDateTime" << userDateTime;
    qDebug() << "nowtime" << QDateTime::currentDateTime().toString("d MMMM yyyy hh:mm:ss");
    startTimer();
}

void mainWindow::startTimer(){
    QTimer *rechecktimer = new QTimer(this);
    connect(rechecktimer,SIGNAL(timeout()),this,SLOT(alarmCheck()));
    qDebug() << "Timer Started";
    rechecktimer->start(1000);
}

void mainWindow::alarmCheck(){
    nowDateTime = QDateTime::currentDateTime().toString("d MMMM yyyy hh:mm:ss");
    qDebug() << "nowDateTime" << nowDateTime;
    qDebug() << "userDateTime" << userDateTime;
    qDebug() << "customDateTime" << customDateTime.toString("d MMMM yyyy hh:mm:ss");
    if(nowDateTime==userDateTime){tripAlarm();}
    }

void mainWindow::quitApp(){
    qApp->quit();
}

void mainWindow::syncClock(){
    ui->currentClock->setText(QDateTime::currentDateTime().toString("hh:mm:ss ap"));
}

