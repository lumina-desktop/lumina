//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "glw-base.h"
#include "glw-widget.h"

GLW_Base::GLW_Base(QWidget *parent, Qt::WindowFlags f) : QOpenGLWidget(parent,f){
  bg_color = QColor(Qt::black);
}

GLW_Base::~GLW_Base(){

}

// --- PUBLIC SLOTS ---
void GLW_Base::setBackgroundColor(QColor color){
  bg_color = color;
  this->update(); //repaint the entire widget (just in case you can see through the image)

}

void GLW_Base::setBackground(QRect geom, QImage img){
  QPainter P(&bg_img);
  P.drawImage(geom, img);
  this->update();
}

void GLW_Base::repaintArea(QRect rect){
  paintEvent(new QPaintEvent(rect));
}

// --- PROTECTED ---
void GLW_Base::resizeEvent(QResizeEvent *ev){
  QOpenGLWidget::resizeEvent(ev);
  if(!bg_img.isNull()){
    bg_img = bg_img.scaled(ev->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  }
  emit BaseResized();
}

void GLW_Base::paintEvent(QPaintEvent *ev){
  QStylePainter painter(this);
  painter.setClipRegion(ev->rect());
  //Fill in the background color first
  painter.fillRect(ev->rect(), bg_color);
  //Now paint any background image over that
  painter.drawImage(ev->rect(), bg_img, ev->rect(), Qt::AutoColor | Qt::PreferDither | Qt::NoOpaqueDetection);
  //Now find any children widgets and paint them if they are in that area
  QObjectList child = this->children(); //Note: This is returned in stacking order (lowest -> highest)
  for(int i=0; i<child.length(); i++){
    if( !child[i]->isWidgetType() ){ continue; } //not a widget
    GLW_Widget *glww = qobject_cast<GLW_Widget*>(child[i]);
    if(glww!=0){
      if(ev->rect().contains(glww->widgetRect())){
        glww->paintYourself(&painter, ev);
        glww->paintChildren(&painter,ev);
      }
    }
  }
}
