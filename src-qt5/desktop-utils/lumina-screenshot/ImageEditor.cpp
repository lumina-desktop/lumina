//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "ImageEditor.h"

#include <QPainter>

// === PUBLIC ===
ImageEditor::ImageEditor(QWidget*parent) : QWidget(parent){
  contextMenu = new QMenu(this);
    contextMenu->addAction(tr("Zoom In"), this, SLOT(scaleUp()) );
    contextMenu->addAction(tr("Zoom Out"), this, SLOT(scaleDown()) );
  this->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showMenu()) );
}

ImageEditor::~ImageEditor(){

}

void ImageEditor::LoadImage(QImage img){
  fullIMG = img;
  scaledIMG = fullIMG.scaled( defaultSize, Qt::KeepAspectRatio,Qt::SmoothTransformation);
  this->update(); //trigger paint event
  selRect = QRect();
  emit selectionChanged(false);
}

void ImageEditor::setDefaultSize(QSize sz){
  defaultSize = sz;
  bool change = false;
  if(scaledIMG.width() > scaledIMG.height()){
    change = (sz.width() > scaledIMG.width() && sz.width() < fullIMG.width()); //new viewport is larger than the scaled image
  }else{
    change = (sz.height() > scaledIMG.height() && sz.height() < fullIMG.height()); //new viewport is larger than the scaled image
  }
  if(change){
    scaledIMG = fullIMG.scaled( defaultSize, Qt::KeepAspectRatio,Qt::SmoothTransformation);
    selRect = QRect();
    emit selectionChanged(false);
    this->update(); //trigger paint event
  }
}

bool ImageEditor::hasSelection(){
  return !selRect.isNull();
}
	
QImage ImageEditor::image(){
  return fullIMG;
}

// === PRIVATE SLOTS ===
void ImageEditor::showMenu(){
  contextMenu->popup(QCursor::pos());
}

// === PUBLIC SLOTS ===
void ImageEditor::scaleUp(int val){
  qreal sf = getScaleFactor();
  sf+= ((qreal) val)/100.0;
  if(sf>2){ sf = 2.0; }
  rescaleImage(sf);
}

void ImageEditor::scaleDown(int val){
  qreal sf = getScaleFactor();
  sf-= ((qreal) val)/100.0;
  if(sf<0.1){ sf = 0.1; }
  rescaleImage(sf);
}

void ImageEditor::cropImage(){
  if(selRect.isNull()){ return; }
  qreal sf = getScaleFactor();
  QRect srect = QRect( qRound(selRect.x()/sf), qRound(selRect.y()/sf), qRound(selRect.width()/sf), qRound(selRect.height()/sf));
  fullIMG = fullIMG.copy(srect);
  scaledIMG = fullIMG.scaled( defaultSize, Qt::KeepAspectRatio,Qt::SmoothTransformation);
  selRect = QRect();
  emit selectionChanged(false);
  this->update(); //trigger paint event
}

void ImageEditor::resizeImage(){
 //TO-DO
  selRect = QRect();
  emit selectionChanged(false);
}

// === PROTECTED ===
void ImageEditor::mousePressEvent(QMouseEvent *ev){
  selRect = QRect(); //reset it
  emit selectionChanged(false);
  selPoint = ev->pos(); //widget-relative coords
}

void ImageEditor::mouseMoveEvent(QMouseEvent *ev){
  if( !this->geometry().contains(ev->pos()) ){ selRect = QRect(); }
  else if(selPoint.x() < ev->pos().x()){
    if(selPoint.y() < ev->pos().y()){
      //init point is upper-left corner
      selRect = QRect(selPoint, ev->pos());
    }else{
      //init point is lower-left corner
      selRect.setBottomLeft(selPoint);
      selRect.setTopRight(ev->pos());
    }
  }else{
    if(selPoint.y() < ev->pos().y()){
      //init point is upper-right corner
      selRect.setBottomLeft(ev->pos());
      selRect.setTopRight(selPoint);
    }else{
      //init point is lower-right corner
	selRect = QRect(ev->pos(), selPoint);
    }
  }
  this->update();
}

void ImageEditor::mouseReleaseEvent(QMouseEvent*){
  emit selectionChanged( !selRect.isNull() );
  this->update();
}

void ImageEditor::paintEvent(QPaintEvent*){
  //ensure the widget is large enough to show the whole scaled image
  if(this->size()!=scaledIMG.size()){ 
    this->setFixedSize(scaledIMG.size());
    this->update();
    return;
  }
  QPainter P(this);
  //Draw the image
    P.drawImage(QPoint(0,0), scaledIMG);
  //Now draw the selection rectangle over the top
  if(!selRect.isNull()){ P.fillRect(selRect, QBrush(Qt::CrossPattern)); }
}
