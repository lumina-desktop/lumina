//===========================================
//  Copyright (c) 2018, JT(q5sys)
//  Available under the MIT license
//  See the LICENSE file for full details
//===========================================
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
//#include <QDebug>
#include <ExternalProcess.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),  ui(new Ui::MainWindow){
  ui->setupUi(this);
  QString qdt_value = QDateTime::currentDateTime().toString("yyyy.MM.dd HH:mm");
  ui->label_current_time->setText(qdt_value);
}

MainWindow::~MainWindow(){
   delete ui;
}

void MainWindow::on_pushButton_SetTime_clicked(){
  QString datestring = "qsudo date ";
  QString timestring = ui->lineEditNewTime->text();
  QString cmd = datestring + timestring;
  //qDebug() << cmd;
  ExternalProcess::launch(cmd);
  //finishedMessage();
}
