//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "ScriptDialog.h"
#include "ui_ScriptDialog.h"

ScriptDialog::ScriptDialog(QWidget* parent) : QDialog(parent), ui(new Ui::ScriptDialog){
  ui->setupUi(this);
}

ScriptDialog::~ScriptDialog(){
  delete ui;
}

bool ScriptDialog::isValid(){
  return !ui->line_name->text().isEmpty() && !ui->line_command->text().isEmpty();
}

QString ScriptDialog::name(){
  return ui->line_name->text();
}

QString ScriptDialog::command(){
  return ui->line_command->text();
}

QString ScriptDialog::icon(){
  return ui->line_icon->text();
}
