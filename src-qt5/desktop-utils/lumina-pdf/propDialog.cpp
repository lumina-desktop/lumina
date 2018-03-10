//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================

#include "propDialog.h"
#include "ui_propDialog.h"

#include <LuminaXDG.h>

PropDialog::PropDialog(Renderer *Backend) : QDialog(), ui(new Ui::PropDialog()){
  ui->setupUi(this);
  this->setWindowTitle(tr("PDF Information"));
  this->setWindowIcon( LXDG::findIcon("dialog-information","unknown"));

  connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(close()));

  //Setup translations
  ui->titleL->setText(tr("Title:"));
  ui->subjectL->setText(tr("Subject:"));
  ui->authorL->setText(tr("Author:"));
  ui->creatorL->setText(tr("Creator:"));
  ui->producerL->setText(tr("Producer:"));
  ui->keywordsL->setText(tr("Keywords:"));
  ui->createdL->setText(tr("Created:"));
  ui->modifiedL->setText(tr("Modified:"));
  ui->sizeL->setText(tr("Page Size: "));
  ui->numberL->setText(tr("Number of Pages: "));
  ui->saveButton->setText(tr("Save"));
  ui->closeButton->setText(tr("Close"));

  QJsonObject info = Backend->properties();
  //Fill the text boxes with information from the document
  ui->titleE->setText( info.value("title").toString() );
  ui->subjectE->setText( info.value("subject").toString() );
  ui->authorE->setText( info.value("author").toString() );
  ui->creatorE->setText( info.value("creator").toString() );
  ui->producerE->setText( info.value("producer").toString() );
  ui->keywordE->setText( info.value("keywords").toString() );
  ui->createdEntry->setText( info.value("dt_created").toString() );
  ui->modifiedEntry->setText( info.value("dt_modified").toString() );
  ui->numberL->setText( QString::number(Backend->numPages()) );

}

//Load size from mainUI after pages have loaded
void PropDialog::setSize(QSizeF pageSize) {
  ui->sizeL->setText(ui->sizeL->text()+QString::number(pageSize.width())+", "+QString::number(pageSize.height()));
}
