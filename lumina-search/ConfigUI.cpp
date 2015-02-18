//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "ConfigUI.h"
#include "ui_ConfigUI.h"

#include <QFileDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QListView>
#include <QTreeView>
#include <QPoint>

#include <LuminaXDG.h>

ConfigUI::ConfigUI(QWidget *parent) : QDialog(parent), ui(new Ui::ConfigUI){
  ui->setupUi(this);
  //Make sure this dialog is centered on the parent
  if(parent!=0){
    QPoint ctr = parent->geometry().center();
    this->move( ctr.x()-(this->width()/2), ctr.y()-(this->height()/2) );
  }
  ui->tool_getStartDir->setIcon( LXDG::findIcon("folder","") );
  ui->tool_adddirs->setIcon( LXDG::findIcon("list-add","") );
  ui->tool_rmdir->setIcon( LXDG::findIcon("list-remove","") );
}

ConfigUI::~ConfigUI(){
	
}

void ConfigUI::loadInitialValues(QString startdir, QStringList skipdirs){
  ui->label_start->setText(startdir);
  ui->list_excludes->clear();
  ui->list_excludes->addItems(skipdirs);
}

void ConfigUI::on_tool_getStartDir_clicked(){
  QString dir = QFileDialog::getExistingDirectory(this, tr("Select Search Directory"), QDir::homePath() );
  if(dir.isEmpty()){ return; }
  ui->label_start->setText(dir);
}

void ConfigUI::on_tool_adddirs_clicked(){
  QFileDialog dlg(this);
  dlg.setFileMode(QFileDialog::DirectoryOnly);
  QListView *l = dlg.findChild<QListView*>("listView");
  if(l){ l->setSelectionMode(QAbstractItemView::MultiSelection); }
  QTreeView *t = dlg.findChild<QTreeView*>();
  if(t){ t->setSelectionMode(QAbstractItemView::MultiSelection); }
  dlg.setDirectory(QDir::homePath());
  dlg.setWindowTitle( tr("Exclude Directories") );
  if(dlg.exec()){
    //Directories selected
    QStringList paths = dlg.selectedFiles();
    ui->list_excludes->addItems(paths);
  }
}

void ConfigUI::on_tool_rmdir_clicked(){
  QList<QListWidgetItem*> sel = ui->list_excludes->selectedItems();
  for(int i=0; i<sel.length(); i++){
    ui->list_excludes->removeItemWidget(sel[i]);
  }
}

void ConfigUI::on_list_excludes_itemSelectionChanged(){
  ui->tool_rmdir->setEnabled( !ui->list_excludes->selectedItems().isEmpty() );
}

void ConfigUI::on_buttonBox_accepted(){
  newStartDir = ui->label_start->text();
  QStringList dirs;
  for(int i=0; i<ui->list_excludes->count(); i++){
    dirs << ui->list_excludes->item(i)->text();
  }
  dirs.removeDuplicates();
  newSkipDirs = dirs;
  this->close();
}

void ConfigUI::on_buttonBox_rejected(){
  this->close();
}
