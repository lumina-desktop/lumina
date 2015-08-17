//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "SlideshowWidget.h"
#include "ui_SlideshowWidget.h"

#include <QImageWriter>
#include <QMessageBox>

SlideshowWidget::SlideshowWidget(QWidget *parent) : QWidget(parent), ui(new Ui::SlideshowWidget){
  ui->setupUi(this); //load the designer file

  UpdateIcons();
  UpdateText();	
}

SlideshowWidget::~SlideshowWidget(){
	
}

// ================
//    PUBLIC SLOTS
// ================
void SlideshowWidget::LoadImages(QList<LFileInfo> list){
  ui->combo_image_name->clear();
  for(int i=0; i<list.length(); i++){
    if(list[i].isImage()){ ui->combo_image_name->addItem(list[i].fileName(), list[i].absoluteFilePath() ); }
  }
}

//Theme change functions
void SlideshowWidget::UpdateIcons(){
  ui->tool_image_goBegin->setIcon( LXDG::findIcon("go-first-view","") );
  ui->tool_image_goEnd->setIcon( LXDG::findIcon("go-last-view","") );
  ui->tool_image_goPrev->setIcon( LXDG::findIcon("go-previous-view","") );
  ui->tool_image_goNext->setIcon( LXDG::findIcon("go-next-view","") );
  ui->tool_image_remove->setIcon( LXDG::findIcon("edit-delete","") );
  ui->tool_image_rotateleft->setIcon( LXDG::findIcon("object-rotate-left","") );
  ui->tool_image_rotateright->setIcon( LXDG::findIcon("object-rotate-right","") );
}

void SlideshowWidget::UpdateText(){
  ui->retranslateUi(this);
}


// =================
//       PRIVATE
// =================
void SlideshowWidget::UpdateImage(){
  if( !ui->label_image->isVisible() ){ return; } //don't update if not visible - can cause strange resizing issues
  QString file = ui->combo_image_name->currentData().toString();
  /*if(!file.endsWith("/")){ file.append("/"); }
  file.append(ui->combo_image_name->currentText());*/
  //qDebug() << "Show Image:" << file;
  QPixmap pix(file);
  if(pix.size().width() > ui->label_image->contentsRect().width() || pix.size().height() > ui->label_image->contentsRect().height()){ 
    pix = pix.scaled(ui->label_image->contentsRect().size(), Qt::KeepAspectRatio, Qt::SmoothTransformation); 
  }
  ui->label_image->setPixmap(pix);
  //Now set/load the buttons
  ui->tool_image_goBegin->setEnabled(ui->combo_image_name->currentIndex()>0);
  ui->tool_image_goPrev->setEnabled(ui->combo_image_name->currentIndex()>0);
  ui->tool_image_goEnd->setEnabled(ui->combo_image_name->currentIndex()<(ui->combo_image_name->count()-1));
  ui->tool_image_goNext->setEnabled(ui->combo_image_name->currentIndex()<(ui->combo_image_name->count()-1));
  ui->label_image_index->setText( QString::number(ui->combo_image_name->currentIndex()+1)+"/"+QString::number(ui->combo_image_name->count()) );
  static QList<QByteArray> writeableformats;
  if(writeableformats.isEmpty()){
    writeableformats  = QImageWriter::supportedImageFormats();
    qDebug() << "Writeable image formats:" << writeableformats;
  }
  bool canwrite = writeableformats.contains(file.section(".",-1).toLower().toLocal8Bit()); //compare the suffix with the list
  bool isUserWritable = QFileInfo(file).isWritable();
  ui->tool_image_remove->setEnabled(isUserWritable);
  ui->tool_image_rotateleft->setEnabled(isUserWritable && canwrite);
  ui->tool_image_rotateright->setEnabled(isUserWritable && canwrite);
}


// =================
//    PRIVATE SLOTS
// =================
// Picture rotation options
void SlideshowWidget::on_combo_image_name_indexChanged(int index){
  if(index>=0 && !ui->combo_image_name->currentData().toString().isEmpty()){
    UpdateImage();
  }
}

void SlideshowWidget::on_tool_image_goEnd_clicked(){
  ui->combo_image_name->setCurrentIndex( ui->combo_image_name->count()-1 );
}

void SlideshowWidget::on_tool_image_goNext_clicked(){
  ui->combo_image_name->setCurrentIndex( ui->combo_image_name->currentIndex()+1 );
}

void SlideshowWidget::on_tool_image_goPrev_clicked(){
  ui->combo_image_name->setCurrentIndex( ui->combo_image_name->currentIndex()-1 );
}

void SlideshowWidget::on_tool_image_goBegin_clicked(){
  ui->combo_image_name->setCurrentIndex(0);
}

// Picture modification options
void SlideshowWidget::on_tool_image_remove_clicked(){
  QString file = ui->combo_image_name->currentData().toString();
  //Verify permanent removal of file/dir
  if(QMessageBox::Yes != QMessageBox::question(this, tr("Verify Removal"), tr("WARNING: This will permanently delete the file from the system!")+"\n"+tr("Are you sure you want to continue?")+"\n\n"+file, QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ){
    return; //cancelled
  }
  if( QFile::remove(file) ){
    int index = ui->combo_image_name->currentIndex();
    ui->combo_image_name->removeItem( index );
  }
}

void SlideshowWidget::on_tool_image_rotateleft_clicked(){
  //First load the file fresh (not the scaled version in the UI)
  QString file = ui->combo_image_name->currentData().toString();
  QPixmap pix(file);	
  //Now rotate the image 90 degrees counter-clockwise
  QTransform trans;
  pix = pix.transformed( trans.rotate(-90) , Qt::SmoothTransformation);
  //Now save the image back to the same file
  pix.save(file);
  //Now re-load the image in the UI
  UpdateImage();	
}

void SlideshowWidget::on_tool_image_rotateright_clicked(){
  //First load the file fresh (not the scaled version in the UI)
  QString file = ui->combo_image_name->currentData().toString();
  QPixmap pix(file);	
  //Now rotate the image 90 degrees counter-clockwise
  QTransform trans;
  pix = pix.transformed( trans.rotate(90) , Qt::SmoothTransformation);
  //Now save the image back to the same file
  pix.save(file);
  //Now re-load the image in the UI
  UpdateImage();	
}
