#include <QDebug>
#include <QApplication>
#include <QTimer>
#include <QColor>

#include "../glw-base.h"
#include "../glw-widget.h"

#include "colorchange.h"

int main(int argc, char** argv){
  QApplication A(argc,argv);
    qDebug() << "Creating base widget";
    GLW_Base base;
      qDebug() << "Resize base widget";
    base.resize(200,200);
      qDebug() << "Create colorchange";
    GLW_Widget wgt(&base);
      wgt.setGLBase(&base);
      wgt.setGeometry(50,50,50,50);
    colorchange CC(&base);
      qDebug() << "Start Event loop";
    base.show();
  A.exec();
    qDebug() << " - Finished";
}
