//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "ColorDialog.h"
#include "ui_ColorDialog.h"

#include <QColorDialog>
#include <QStringList>

#include <LuminaXDG.h>

ColorDialog::ColorDialog(QSettings *set, QWidget *parent) : QDialog(parent), ui(new Ui::ColorDialog()){
  ui->setupUi(this);
  settings = set;
  connect(ui->push_cancel, SIGNAL(clicked()), this, SLOT(close()) );
  connect(ui->push_apply, SIGNAL(clicked()), this, SLOT(saveColors()) );
  connect(ui->push_getcolor, SIGNAL(clicked()), this, SLOT(changeColor()) );
}

void ColorDialog::LoadColors(){
  ui->treeWidget->clear();
  QStringList colors = settings->allKeys().filter("colors/");

  for(int i=0; i<colors.length(); i++){
    QTreeWidgetItem *it = new QTreeWidgetItem();
    it->setText(0, colors[i].section("/",-1));
    it->setText(1, settings->value(colors[i]).toString() );
    it->setBackground(2, QBrush(QColor( it->text(1) ) ) );
    ui->treeWidget->addTopLevelItem(it);
  }
}

void ColorDialog::updateIcons(){
  this->setWindowIcon( LXDG::findIcon("format-fill-color") );
  ui->push_cancel->setIcon( LXDG::findIcon("dialog-cancel") );
  ui->push_apply->setIcon( LXDG::findIcon("dialog-ok") );
  ui->push_getcolor->setIcon( LXDG::findIcon("format-fill-color") );
}

void ColorDialog::saveColors(){
  for(int i=0; i<ui->treeWidget->topLevelItemCount(); i++){
    QTreeWidgetItem *it = ui->treeWidget->topLevelItem(i);
    settings->setValue("colors/"+it->text(0), it->text(1));
  }
  emit colorsChanged();
  this->close();
}

void ColorDialog::changeColor(){
  QTreeWidgetItem *it = ui->treeWidget->currentItem();
  if(it==0){ return; }
  QColor color = QColorDialog::getColor(QColor( it->text(1)), this, tr("Select Color"));
  if(!color.isValid()){ return; }
  it->setText(1, color.name());
  it->setBackground(2, QBrush(color));
}
