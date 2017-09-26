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
  this->setMouseTracking(true);
}

GLW_Widget::~GLW_Widget(){

}

QRect GLW_Widget::widgetRect(){
  QPoint pos = this->mapTo(glw_base, QPoint(0,0));
  return QRect(pos, this->size());
}

bool GLW_Widget::mouseOverWidget(){
  QPoint pos = this->mapFromGlobal(QCursor::pos());
  QRect geom(QPoint(0,0), this->size());
  return geom.contains(pos);

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
  color.setAlpha(125);
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
