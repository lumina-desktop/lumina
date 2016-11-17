//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "page_mouse.h"
#include "ui_page_mouse.h"
#include "getPage.h"

//==========
//    PUBLIC
//==========
page_mouse::page_mouse(QWidget *parent) : PageWidget(parent), ui(new Ui::page_mouse()){
  ui->setupUi(this);
  connect(ui->slider_accel, SIGNAL(valueChanged(int)), this, SLOT(accelChanged(int)) );
  updateIcons();
    qDebug() << "List Devices:";
   QList<LInputDevice*> devices = LInput::listDevices();
    for(int i=0; i<devices.length(); i++){
      if(!devices[i]->isPointer()){
        ::free( devices.takeAt(i));
        i--;
      }else{
        qDebug() << "Found Pointer:" << devices[i]->devNumber();
        qDebug() << " - isExtension:" << devices[i]->isExtension();
        devices[i]->listProperties();
      }
    }
}

page_mouse::~page_mouse(){
  for(int i=0; i<devices.length(); i++){ ::free(devices[i]); }
}

//================
//    PUBLIC SLOTS
//================
void page_mouse::SaveSettings(){

  emit HasPendingChanges(false);
}

void page_mouse::LoadSettings(int){
  emit HasPendingChanges(false);
  emit ChangePageTitle( tr("Desktop Settings") );

}

void page_mouse::updateIcons(){

}

//=================
//         PRIVATE 
//=================

//=================
//    PRIVATE SLOTS
//=================
void page_mouse::accelChanged(int val){
  if(val<=4){ 
    val = 4-val; 
    ui->label_accel->setText( QString("1/%1").arg(QString::number(val)) ); 
    QProcess::startDetached("xset mouse 1/"+QString::number(val));
  }else{
    val = val-4;
    if(val%2==0){ 
      val = val/2;
      ui->label_accel->setText( QString::number(val) ); 
      QProcess::startDetached("xset mouse "+QString::number(val));
    }else{
      ui->label_accel->setText( QString::number(val)+"/2" ); 
      QProcess::startDetached("xset mouse "+QString::number(val)+"/2");
    }
  }
}
