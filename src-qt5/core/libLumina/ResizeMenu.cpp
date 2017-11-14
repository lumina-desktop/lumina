//===========================================
//  Lumina-DE source code
//  Copyright (c) 2013-2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "ResizeMenu.h"
#include <QDebug>

// =======================
//      RESIZEMENU CLASS
// =======================
ResizeMenu::ResizeMenu(QWidget *parent) : QMenu(parent){
  this->setContentsMargins(1,1,1,1);
  this->setMouseTracking(true);
  resizeSide = NONE;
  cAct = new QWidgetAction(this);
  contents = 0;
  connect(this, SIGNAL(aboutToShow()), this, SLOT(clearFlags()) );
  connect(this, SIGNAL(aboutToHide()), this, SLOT(clearFlags()) );
  connect(cAct, SIGNAL(hovered()), this, SLOT(clearFlags()) );
}

ResizeMenu::~ResizeMenu(){

}

void ResizeMenu::setContents(QWidget *con){
  this->clear();
  cAct->setDefaultWidget(con);
  this->addAction(cAct);
  contents = con; //save for later
  contents->setCursor(Qt::ArrowCursor);
  resyncSize();
}

void ResizeMenu::resyncSize(){
  if(contents==0){ return; }
  qDebug() << "Resync Size:" << this->size() << contents->size();
  this->resize(contents->size());
  qDebug() << " - after menu resize:" << this->size() << contents->size();
  emit MenuResized(this->size());
}

void ResizeMenu::mouseMoveEvent(QMouseEvent *ev){
  QRect geom = this->geometry();
  //Note: The exact position does not matter as much as the size
  //  since the window will be moved again the next time it is shown
  // The "-2" in the sizing below accounts for the menu margins
  QPoint gpos = this->mapToGlobal(ev->pos());
  bool handled = false;
  switch(resizeSide){
    case TOP:
	if(gpos.y() >= geom.bottom()-1){ break; }
	geom.setTop(gpos.y());
        this->setGeometry(geom);
        if(contents!=0){ contents->setFixedSize(QSize(geom.width()-2, geom.height()-2));}
	handled = true;
        break;
    case BOTTOM:
	if(gpos.y() <= geom.top()+1){ break; }
	geom.setBottom( gpos.y());
        this->setGeometry(geom);
        if(contents!=0){ contents->setFixedSize(QSize(geom.width()-2, geom.height()-2));}
	handled = true;
        break;
    case LEFT:
	if(gpos.x() >= geom.right()-1){ break; }
	geom.setLeft(gpos.x());
        this->setGeometry(geom);
        if(contents!=0){ contents->setFixedSize(QSize(geom.width()-2, geom.height()-2));}
	handled = true;
        break;
    case RIGHT:
	if(gpos.x() <= geom.left()+1){ break; }
	geom.setRight(gpos.x());
        this->setGeometry(geom);
        if(contents!=0){ contents->setFixedSize(QSize(geom.width()-2, geom.height()-2));}
	handled = true;
        break;
    default: //NONE
	//qDebug() << " - Mouse At:" << ev->pos();
	//Just adjust the mouse cursor which is shown
        if(ev->pos().x()<=1 && ev->pos().x() >= -1){ this->setCursor(Qt::SizeHorCursor); }
        else if(ev->pos().x() >= this->width()-1 && ev->pos().x() <= this->width()+1){  this->setCursor(Qt::SizeHorCursor); }
        else if(ev->pos().y()<=1 && ev->pos().y() >= -1){ this->setCursor(Qt::SizeVerCursor); }
        else if(ev->pos().y() >= this->height()-1 && ev->pos().y() <= this->height()+1){ this->setCursor(Qt::SizeVerCursor); }
	else{ this->setCursor(Qt::ArrowCursor); }
  }
  if(!handled){ QMenu::mouseMoveEvent(ev); }  //do normal processing as well
}

void ResizeMenu::mousePressEvent(QMouseEvent *ev){
  bool used = false;
  if(ev->buttons().testFlag(Qt::LeftButton) && resizeSide==NONE){
    //qDebug() << "Mouse Press Event:" <<  ev->pos() << resizeSide;
    if(ev->pos().x()<=1 && ev->pos().x() >= -1){resizeSide = LEFT; used = true;}
    else if(ev->pos().x() >= this->width()-1 && ev->pos().x() <= this->width()+1){ resizeSide = RIGHT; used = true;}
    else if(ev->pos().y()<=1 && ev->pos().y() >= -1){ resizeSide = TOP; used = true; }
    else if(ev->pos().y() >= this->height()-1 && ev->pos().y() <= this->height()+1){ resizeSide = BOTTOM; used = true; }
  }
  if(used){ ev->accept(); this->grabMouse(); }
  else{ QMenu::mousePressEvent(ev); } //do normal processing
}

void ResizeMenu::mouseReleaseEvent(QMouseEvent *ev){
  this->releaseMouse();
  if(ev->button() == Qt::LeftButton && resizeSide!=NONE ){
    //qDebug() << "Mouse Release Event:" <<  ev->pos() << resizeSide;
    resizeSide = NONE;
    emit MenuResized(contents->size());
    ev->accept();
  }else{
    QMenu::mouseReleaseEvent(ev);  //do normal processing
  }
}
