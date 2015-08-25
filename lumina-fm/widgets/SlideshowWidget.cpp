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
  zoom = 1;
  UpdateIcons();
  UpdateText();	
}

SlideshowWidget::~SlideshowWidget(){
	
}

// ================
//    PUBLIC SLOTS
// ================
void SlideshowWidget::ClearImages(){
  ui->combo_image_name->clear();	
}

void SlideshowWidget::LoadImages(QList<LFileInfo> list){
  int cmax = ui->combo_image_name->count(); //current number of items
  for(int i=0; i<list.length(); i++){
    if(list[i].isImage()){ ui->combo_image_name->addItem(list[i].fileName(), list[i].absoluteFilePath() ); }
  }
  //Now automatically show the first item from the batch of new ones
  if(cmax < ui->combo_image_name->count()){ ui->combo_image_name->setCurrentIndex(cmax); }
}

void SlideshowWidget::refresh(){
  UpdateImage();
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
  ui->tool_image_zoomin->setIcon( LXDG::findIcon("zoom-in","") );
  ui->tool_image_zoomout->setIcon( LXDG::findIcon("zoom-out","") );
}

void SlideshowWidget::UpdateText(){
  ui->retranslateUi(this);
}


// =================
//       PRIVATE
// =================
void SlideshowWidget::UpdateImage(){
  QString file = ui->combo_image_name->currentData().toString();
  qDebug() << "Show Image:" << file << "Zoom:" << zoom;
  QPixmap pix(file);
    QSize sz = ui->scrollArea->contentsRect().size();
    if( sz.width()>pix.size().width() || sz.height()>pix.size().height()){ sz = pix.size(); } //100% size already - apply zoom after this
    pix = pix.scaled(sz*zoom, Qt::KeepAspectRatio, Qt::SmoothTransformation); 
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
  ui->tool_image_zoomin->setEnabled(zoom<2);
  ui->tool_image_zoomout->setEnabled(zoom>0.25);
}


// =================
//    PRIVATE SLOTS
// =================
// Picture rotation options
void SlideshowWidget::on_combo_image_name_currentIndexChanged(int index){
  if(index>=0 && !ui->combo_image_name->currentData().toString().isEmpty()){
    zoom = 1; //always reset the zoom level when changing images
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

void SlideshowWidget::on_tool_image_zoomin_clicked(){
  zoom+=0.25; //change 25% every time
  UpdateImage();
}

void SlideshowWidget::on_tool_image_zoomout_clicked(){
  zoom-=0.25; //change 25% every time
  UpdateImage();
}

