//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================

#include "propDialog.h"
#include "ui_propDialog.h"

#include <LuminaXDG.h>

void PropDialog::setInfo(fz_context *CTX, pdf_obj *info, QTextEdit *widget, QString str) {
  pdf_obj *obj = pdf_dict_gets(CTX, info, str.toLatin1().data());
  if(obj)
    widget->setText(pdf_to_utf8(CTX, obj));
}

PropDialog::PropDialog(fz_context *CTX, pdf_document *DOC) : QDialog(), ui(new Ui::PropDialog()){
  this->setWindowTitle(tr("PDF Information"));
  this->setWindowIcon( LXDG::findIcon("dialog-information","unknown"));
  pdf_obj *info = pdf_dict_gets(CTX, pdf_trailer(CTX, DOC), "Info");

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
  ui->sizeL->setText(tr("Page Size: "));
  ui->numberL->setText(tr("Number of Pages: "));
  ui->saveButton->setText(tr("Save"));
  ui->closeButton->setText(tr("Close"));

  //Fill the text boxes with information from the document
  if(info) {
    setInfo(CTX, info, ui->titleE, "Title");
    setInfo(CTX, info, ui->subjectE, "Subject");
    setInfo(CTX, info, ui->authorE, "Author");
    setInfo(CTX, info, ui->creatorE, "Creator");
    setInfo(CTX, info, ui->producerE, "Producer");
    setInfo(CTX, info, ui->keywordE, "Keywords");
    pdf_obj *obj = pdf_dict_gets(CTX, info, (char *)"CreationDate");
    char *str = pdf_to_utf8(CTX, obj);
    if(obj)
      ui->createdEntry->setText(QDateTime::fromString(QString(str).left(16), "'D:'yyyyMMddHHmmss").toString());
    //ModDate not displaying when should, possibly broken
    obj = pdf_dict_gets(CTX, info, (char *)"ModDate");
    str = pdf_to_utf8(CTX, obj);
    if(obj)
      ui->modifiedEntry->setText(QDateTime::fromString(QString(str).left(16), "'D:'yyyyMMddHHmmss").toString());
    ui->numberL->setText(ui->numberL->text()+QString::number(pdf_count_pages(CTX, DOC)));
    free(str);
  }
}

//Load size from mainUI after pages have loaded
void PropDialog::setSize(QSizeF pageSize) { 
  ui->sizeL->setText(ui->sizeL->text()+QString::number(pageSize.width())+", "+QString::number(pageSize.height()));
}
