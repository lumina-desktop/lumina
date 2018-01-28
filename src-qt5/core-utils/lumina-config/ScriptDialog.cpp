//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "ScriptDialog.h"
#include "ui_ScriptDialog.h"

//===========
//    PUBLIC
//===========
ScriptDialog::ScriptDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ScriptDialog){
  ui->setupUi(this);
  ok = false;
  connect(ui->line_name, SIGNAL(textEdited(QString)), this, SLOT(checkItems()) );
  connect(ui->line_exec, SIGNAL(textEdited(QString)), this, SLOT(checkItems()) );
  connect(ui->line_icon, SIGNAL(textEdited(QString)), this, SLOT(checkItems()) );
  checkItems(true);
}

ScriptDialog::~ScriptDialog(){

}

//Main interaction functions
bool ScriptDialog::isValid(){
  return ok;
}

QString ScriptDialog::icon(){
  return ui->line_icon->text();
}

QString ScriptDialog::name(){
  return ui->line_name->text();
}

QString ScriptDialog::command(){
  return ui->line_exec->text();
}

//==============
// PRIVATE SLOTS
//==============
void ScriptDialog::on_pushApply_clicked(){
  ok = true;
  this->close();
}

void ScriptDialog::on_pushCancel_clicked(){
 ok = false;
  this->close();
}

void ScriptDialog::on_tool_getexec_clicked(){
  QString file = QFileDialog::getOpenFileName( this, tr("Select a menu script"), LOS::LuminaShare()+"/menu-scripts/" );
  if(file.isEmpty()){ return; } //cancelled
  ui->line_exec->setText(file);
  checkItems();
}

void ScriptDialog::on_tool_geticon_clicked(){
  QString file = QFileDialog::getOpenFileName( this, tr("Select an icon file"), QDir::homePath() );
  if(file.isEmpty()){ return; } //cancelled
  ui->line_icon->setText(file);
  checkItems();
}

void ScriptDialog::checkItems(bool firstrun){
  if(firstrun){
    ui->line_name->setFocus();
    ui->label_sample->setPixmap( LXDG::findIcon("text-x-script","").pixmap(32,32) );
    ui->tool_geticon->setIcon( LXDG::findIcon("system-search","") );
    ui->tool_getexec->setIcon( LXDG::findIcon("system-search","") );
  }
  //Update the icon sample if needed
  if(icon()!=ui->label_sample->whatsThis()){
    ui->label_sample->setPixmap( LXDG::findIcon(icon(),"text-x-script").pixmap(32,32) );
    ui->label_sample->setWhatsThis(icon());
  }
  bool good = true;
  if(name().isEmpty()){ good = false; ui->line_name->setStyleSheet("color: red;"); }
  else{ ui->line_name->setStyleSheet(""); }
  QString cmd = command().section(" ",0,0).simplified();
  if( cmd.isEmpty() || !LUtils::isValidBinary(cmd) ){ good = false; ui->line_exec->setStyleSheet("color: red;"); }
  else{ ui->line_exec->setStyleSheet(""); }

  ui->pushApply->setEnabled(good);
}
