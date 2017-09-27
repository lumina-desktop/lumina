#include <QDebug>
#include <QApplication>
#include <QTimer>
#include <QColor>
#include <QPropertyAnimation>
#include "../glw-base.h"
#include "../glw-widget.h"

#include "colorchange.h"

int main(int argc, char** argv){

    QSurfaceFormat fmt;
    fmt.setRenderableType(QSurfaceFormat::OpenGL); //OpenGL, OpenGLES, OpenVG
    fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    QSurfaceFormat::setDefaultFormat(fmt);

  QApplication A(argc,argv);

    qDebug() << "Creating base widget";
    GLW_Base base;
      qDebug() << "Resize base widget";
    base.resize(1024,768);
      qDebug() << "Create Other widgets";
    GLW_Widget wgt1(&base);
      wgt1.setGLBase(&base);
      wgt1.setGeometry(50,50,50,50);
      wgt1.setDraggable(true);
    GLW_Widget wgt(&base);
      wgt.setGLBase(&base);
      wgt.setGeometry(75,50,50,50);
      QPropertyAnimation anim(&wgt);
      anim.setTargetObject(&wgt);
      anim.setPropertyName("geometry");
      //anim.setStartValue(QRect(-50,-50,50,50));
      //anim.setEndValue(QRect(200,200,50,50));
      anim.setStartValue(QRect(base.width()/2,base.height()/2,0,0));
      anim.setEndValue(QRect(0,0,base.width(),base.height()));
      anim.setDuration(1500);
      anim.setLoopCount(-1);
      anim.start();
    /*QPushButton but(&base);
      but.setText("button");
      but.setGeometry(100,100,50,50);
      but.setStyleSheet("background: rgba(0,150,0,125)");*/
    colorchange CC(&base);
      qDebug() << "Start Event loop";
    base.show();
  A.exec();
    qDebug() << " - Finished";
}
