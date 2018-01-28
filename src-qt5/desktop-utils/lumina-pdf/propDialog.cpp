//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================

#include "propDialog.h"
#include "ui_propDialog.h"

#include <LuminaXDG.h>

PropDialog::PropDialog(Poppler::Document *DOC) : QDialog(), ui(new Ui::PropDialog()){
  this->setWindowTitle(tr("PDF Information"));
  this->setWindowIcon( LXDG::findIcon("dialog-information","unknown"));
  int verMa, verMi;
  QString version;
  QSize size = DOC->page(0)->pageSize();

  //Grab the version
  DOC->getPdfVersion(&verMa, &verMi);
  version = QString::number(verMa)+"."+QString::number(verMi);

  ui->setupUi(this);

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
  ui->versionL->setText(tr("PDF Version:"));
  ui->sizeL->setText(tr("Page Size:"));
  ui->numberL->setText(tr("Number of Pages:"));
  ui->saveButton->setText(tr("Save"));
  ui->closeButton->setText(tr("Close"));

  //Fill the text boxes with information from the document
  ui->titleE->setText(DOC->title());
  ui->subjectE->setText(DOC->subject());
  ui->authorE->setText(DOC->author());
  ui->creatorE->setText(DOC->creator());
  ui->producerE->setText(DOC->producer());
  ui->keywordE->setText(DOC->keywords());
  ui->createdEntry->setText(DOC->creationDate().toString(Qt::TextDate));
  ui->modifiedEntry->setText(DOC->modificationDate().toString(Qt::TextDate));
  ui->versionL->setText(ui->versionL->text()+version);
  ui->sizeL->setText(ui->sizeL->text()+QString::number(size.height())+
    ", "+QString::number(size.width()));
  ui->numberL->setText(ui->numberL->text()+QString::number(DOC->numPages()));
}
