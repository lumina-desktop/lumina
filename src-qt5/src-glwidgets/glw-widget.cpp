//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "glw-widget.h"
#include <QDebug>

// --- PUBLIC ---
GLW_Widget::GLW_Widget(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f){
  glw_base = 0;
  draggable = false;
  drag_offset = QPoint();
}

GLW_Widget::~GLW_Widget(){

}

QRect GLW_Widget::widgetRect(){
  QPoint pos = this->mapTo(glw_base, QPoint(0,0));
  return QRect(pos, this->size());
}

bool GLW_Widget::mouseOverWidget(){
  return (glw_base->mouseOverWidget()==this);
}

void GLW_Widget::setGLBase(GLW_Base *base){
  if(glw_base!=0){ this->disconnect(glw_base, SLOT(repaintArea(QRect))); }
  glw_base=base;
  connect(this, SIGNAL(repaintArea(QRect)), glw_base, SLOT(repaintArea(QRect)) );
}

void GLW_Widget::paintYourself(QStylePainter *painter, QPaintEvent *ev){
  QRect rect = widgetRect();
  rect = rect.intersected(ev->rect());
  QColor color( mouseOverWidget() ? Qt::gray : Qt::yellow);
  //if(this->windowOpacity()!=1.0){ qDebug() << "Opacity:" << this->windowOpacity(); }
  //color.setAlpha( qRound(this->windowOpacity()*255) );
  color.setAlpha( 125);
  painter->fillRect(rect, color);
}

void GLW_Widget::paintChildren(QStylePainter *painter, QPaintEvent *ev){
  QObjectList child = this->children(); //Note: This is returned in stacking order (lowest -> highest)
  for(int i=0; i<child.length(); i++){
    if( !child[i]->isWidgetType() ){ continue; } //not a widget
    GLW_Widget *glww = qobject_cast<GLW_Widget*>(child[i]);
    if(glww!=0){
      if(ev->rect().contains(glww->widgetRect())){
        glww->paintYourself(painter, ev);
        glww->paintChildren(painter,ev);
      }
    }
  }
}


// --- PROTECTED ---
void GLW_Widget::enterEvent(QEvent*){
  if(!mouseOverWidget()){ glw_base->setMouseOverWidget(this); }
  this->update();
}

void GLW_Widget::leaveEvent(QEvent*){
  if(mouseOverWidget()){ glw_base->setMouseOverWidget(0); }
  this->update();
}

void GLW_Widget::mousePressEvent(QMouseEvent *ev){
  if(!draggable){ QWidget::mousePressEvent(ev); return; }
  if(ev->button()==Qt::LeftButton){
    drag_offset = ev->pos();
  }
}

void GLW_Widget::mouseReleaseEvent(QMouseEvent *ev){
  if(!draggable){ QWidget::mouseReleaseEvent(ev); return; }
  if(ev->button()==Qt::LeftButton){
    if(drag_offset!=ev->pos()){ emit doneDragging(); }
     drag_offset = QPoint(); } //reset offset
}

void GLW_Widget::mouseMoveEvent(QMouseEvent *ev){
  if(!mouseOverWidget()){ glw_base->setMouseOverWidget(this); }
  if(!draggable){ QWidget::mouseMoveEvent(ev); return; }
  if(!drag_offset.isNull()){
    this->move( this->mapTo(glw_base, ev->pos()-drag_offset) );
  }
}


void GLW_Widget::moveEvent(QMoveEvent *ev){
  QRect oldgeom = QRect(ev->oldPos(), this->size());
  QRect newgeom = QRect(ev->pos(), this->size());
  emit repaintArea( oldgeom.united(newgeom) );
}

void GLW_Widget::resizeEvent(QResizeEvent *ev){
  if(glw_base==0){ return; }
  QPoint pos = this->mapTo(glw_base, QPoint(0,0));
  QSize sz = ev->oldSize();
  if(ev->size().width() > sz.width()){ sz.setWidth(ev->size().width()); }
  if(ev->size().height() > sz.height()){ sz.setHeight(ev->size().height()); }
  emit repaintArea(QRect(pos, sz));
}

void GLW_Widget::paintEvent(QPaintEvent *ev){
  //qDebug() << "Got paint event:" << ev->rect();
  QPoint pos = this->mapTo(glw_base, ev->rect().topLeft());
  emit repaintArea(QRect(pos, ev->rect().size()) );
  return;
}
