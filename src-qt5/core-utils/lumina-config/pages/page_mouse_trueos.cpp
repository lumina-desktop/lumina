//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore & JT Pennington
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "page_mouse_trueos.h"
#include "ui_page_mouse_trueos.h"

#include <QDebug>
#include <QProcess>


//==========
//    PUBLIC
//==========
page_mouse_trueos::page_mouse_trueos(QWidget *parent) : PageWidget(parent), ui(new Ui::page_mouse_trueos()){
  ui->setupUi(this);
  QString program = "/usr/sbin/moused";
  
  ui->slider_mouseAcceleration->setRange(1,200);
  ui->slider_mouseAcceleration->setValue(100);
  connect( ui->slider_mouseAcceleration, SIGNAL(valueChanged(int)), this, SLOT(setValue(double)));
  realAccelValue = ( ui->slider_mouseAcceleration->value() / divisor);
  realAccelValueString = QString::number(ui->slider_mouseAcceleration->value() / divisor, 'f', 2);
  
  ui->slider_doubleClickThreshold->setRange(1,1000);
  ui->slider_doubleClickThreshold->setValue(500);
  connect( ui->slider_doubleClickThreshold, SIGNAL(valueChanged(int)), this, SLOT(setValue(double)));
  realDoubleClickValue = (ui->slider_doubleClickThreshold->value());
  realDoubleClickValueString = QString::number(ui->slider_doubleClickThreshold->value());

  ui->combobox_resolutionBox->setCurrentIndex(1);
  connect(ui->combobox_resolutionBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(setMouseResolution()) );

  connect(ui->checkBoxHandedness, SIGNAL(toggled(bool)), this, SLOT(swapHandedness()) );
  connect(ui->checkBoxTerminateDrift, SIGNAL(toggled(bool)), this, SLOT(terminateDrift()) );

  connect( ui->button_apply, SIGNAL(clicked()), this, SLOT(updateMoused()));
}

page_mouse_trueos::~page_mouse_trueos(){
}

//================
//    PUBLIC SLOTS
//================
void page_mouse_trueos::SaveSettings(){

  emit HasPendingChanges(false);
}

void page_mouse_trueos::LoadSettings(int){
  emit HasPendingChanges(false);
  emit ChangePageTitle( tr("Mouse Settings") );
}

//=================
//         PRIVATE
//=================

void page_mouse_trueos::swapHandedness(){
    if(ui->checkBoxHandedness->isChecked()){
    handString = "-m 1=3 -m 3=1";
    }
    else{
    handString = "-m 1=1 -m 3=3";
    }
}

void page_mouse_trueos::setMouseResolution(){
    resolutionValue = ui->combobox_resolutionBox->itemText(ui->combobox_resolutionBox->currentIndex());
    resString = "-r " + resolutionValue;
}

void page_mouse_trueos::setMouseAcceleration(){
    realAccelValue = ( ui->slider_mouseAcceleration->value() / divisor);
    realAccelValueString = QString::number( ui->slider_mouseAcceleration->value() / divisor, 'f', 2);
    accelString = "-a " + realAccelValueString;
}

void page_mouse_trueos::setDoubleClickThreshold(){
    realDoubleClickValueString = QString::number( ui->slider_doubleClickThreshold->value());
    dclickString = "-C " + realDoubleClickValueString;
}

void page_mouse_trueos::terminateDrift(){
    if(ui->checkBoxTerminateDrift->isChecked()){
    driftString = "-T 4[,500[,4000]]";
    }
    else{
    driftString = "";
    }
}

void page_mouse_trueos::updateMoused(){
    setMouseAcceleration(); setDoubleClickThreshold();
    qDebug() << "handString" << handString;
    qDebug() << "resString"  << resString;
    qDebug() << "accelString" << accelString;
    qDebug() << "dclickString" << dclickString;
    qDebug() << "driftstring" << driftString;
    deviceString = "-p /dev/sysmouse";
    mousedargs << deviceString << handString << resString << accelString << dclickString << driftString;
    qDebug() << "mousedargs" << mousedargs;
    QProcess *moused = new QProcess(this);
    moused->start(program, mousedargs);
}
