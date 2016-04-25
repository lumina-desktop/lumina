//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "MonitorWidget.h"
#include "ui_MonitorWidget.h"


#include <LuminaXDG.h>
#include <LuminaOS.h>

MonitorWidget::MonitorWidget(QWidget *parent) : QWidget(parent), ui(new Ui::MonitorWidget()){
  ui->setupUi(this); //load the designer form
  upTimer = new QTimer(this);
    upTimer->setInterval(2000); //update every 2 seconds
  connect(upTimer, SIGNAL(timeout()), this, SLOT(UpdateStats()) );
  LoadIcons();
  upTimer->start();
}

MonitorWidget::~MonitorWidget(){
  //qDebug() << "Removing MonitorWidget";
}

void MonitorWidget::LoadIcons(){
  ui->tabWidget->setTabIcon(0,LXDG::findIcon("appointment-recurring","") ); //Summary
  ui->tabWidget->setTabIcon(1,LXDG::findIcon("drive-harddisk","") ); //Disk Usage
  //ui->tabWidget->setTabIcon(1,LXDG::findIcon("cpu","") ); //CPU Log
  //ui->tabWidget->setTabIcon(2,LXDG::findIcon("media-flash-memory-stick","") ); //Mem Log
}

void MonitorWidget::UpdateStats(){ 
  //qDebug() << "Updating System statistics...";
  ui->label_temps->setText( LOS::CPUTemperatures().join(", ") );
  if(ui->progress_cpu->isEnabled()){
    int perc = LOS::CPUUsagePercent();
    ui->progress_cpu->setValue(perc);
    if(perc<0){ ui->progress_cpu->setEnabled(false); } //disable this for future checks
  }
  if(ui->progress_mem->isEnabled()){
    int perc = LOS::MemoryUsagePercent();
    ui->progress_mem->setValue(perc);
    if(perc<0){ ui->progress_mem->setEnabled(false); } //disable this for future checks
  }
  ui->label_diskinfo->setText( LOS::DiskUsage().join("\n") );
  //Also perform/update the logs as necessary
  // -- TO DO --
}

SysMonitorPlugin::SysMonitorPlugin(QWidget *parent, QString ID) : LDPlugin(parent, ID){
  monitor = new MonitorWidget(this);
  this->setLayout( new QVBoxLayout() );
    this->layout()->setContentsMargins(0,0,0,0);
    this->layout()->addWidget(monitor);
	
  //this->setInitialSize(monitor->sizeHint().width(),monitor->sizeHint().height());
}

SysMonitorPlugin::~SysMonitorPlugin(){
  //qDebug() << "Remove SysMonitorPlugin";
}