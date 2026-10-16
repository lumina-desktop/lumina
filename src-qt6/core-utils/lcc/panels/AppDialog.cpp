//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "AppDialog.h"
#include "ui_AppDialog.h"

AppDialog::AppDialog(QWidget* parent) : QDialog(parent), ui(new Ui::AppDialog){
  ui->setupUi(this);
  appreset = false;
}

AppDialog::~AppDialog(){
  delete ui;
}

void AppDialog::allowReset(bool allow){
  // Stub implementation
}

void AppDialog::accept(){
  // Stub implementation
  QDialog::accept();
}
