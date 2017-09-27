//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "glw-base.h"
#include "glw-widget.h"
#include <QPaintEngine>
#include <QDebug>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLPaintDevice>

GLW_Base::GLW_Base(QWidget *parent, Qt::WindowFlags f) : QOpenGLWidget(parent,f){
  bg_color = QColor(Qt::black);
  mouse_over_child = 0;
  qDebug() << "Canvas supports threaded OpenGL:" <<  this->context()->supportsThreadedOpenGL();
  qDebug() << " - globally:" <<  QOpenGLContext::globalShareContext()->supportsThreadedOpenGL();
}

GLW_Base::~GLW_Base(){

}

QWidget * GLW_Base::mouseOverWidget(){
  return mouse_over_child;
}

// --- PUBLIC SLOTS ---
void GLW_Base::setBackgroundColor(QColor color){
  bg_color = color;
  this->update(); //repaint the entire widget (just in case you can see through the image)

}

void GLW_Base::setBackground(QRect geom, QImage img){
  QPainter P(&bg_img);
  P.drawImage(geom, img);
  this->update(geom);
}

void GLW_Base::repaintArea(QRect rect){
  this->update(rect);
}

void GLW_Base::setMouseOverWidget(QWidget *child){
  mouse_over_child = child;
}

// --- PROTECTED ---
void GLW_Base::mouseMoveEvent(QMouseEvent *ev){
  mouse_over_child = 0; //reset this flag - mouse is over the base right now
  QWidget::mouseMoveEvent(ev);
}

void GLW_Base::resizeEvent(QResizeEvent *ev){
  QOpenGLWidget::resizeEvent(ev);
  if(!bg_img.isNull()){
    bg_img = bg_img.scaled(ev->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  }
  emit BaseResized();
}

/*void GLW_Base::paintEvent(QPaintEvent *ev){
  QOpenGLWidget::paintEvent(ev);
}*/

void GLW_Base::paintGL(){
  //Setup the OpenGL stuff
  QRect rect = QRect(QPoint(0,0), this->size());
  //Prepare the image to be painted
  QImage img(this->size(), QImage::Format_RGBA8888);
  QPainter painter;
    painter.begin(&img);
    painter.fillRect(rect, bg_color);
  painter.end();
  QOpenGLFunctions *f = this->context()->functions();
  //f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //do native OpenGL commands here
  glDrawPixels(bg_img.width(), bg_img.height(), GL_RGBA, GL_UNSIGNED_BYTE, bg_img.bits());

  //Now do any QPainter drawing
  /*QOpenGLPaintDevice device(rect.size());
  QStylePainter painter;
    painter.begin(&device, this);
  //qDebug() << "Paint Engine type:" << painter.paintEngine()->type();
  painter.setClipRegion(rect);
  //Fill in the background color first
  painter.fillRect(rect, bg_color);
  //Now paint any background image over that
  painter.drawImage(rect, bg_img, rect, Qt::AutoColor | Qt::PreferDither | Qt::NoOpaqueDetection);
  //Now find any children widgets and paint them if they are in that area
  QObjectList child = this->children(); //Note: This is returned in stacking order (lowest -> highest)
  for(int i=0; i<child.length(); i++){
    if( !child[i]->isWidgetType() ){ continue; } //not a widget
    GLW_Widget *glww = qobject_cast<GLW_Widget*>(child[i]);
    if(glww!=0){
      if(!rect.intersected(glww->widgetRect()).isNull()){
        glww->paintYourself(&painter, &rect);
        glww->paintChildren(&painter,&rect);
      }
    }
  }
  painter.end();
  update();*/
}
