//===========================================
//  Lumina-DE source code
//  Copyright (c) 2020, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================

#include <QApplication>
#include <QPixmap>
#include <QLabel>
#include <QDateTime>
#include <QPoint>
int main(int argc, char **argv){
  //Setup the application
  QApplication App(argc, argv);
    App.setAttribute(Qt::AA_UseHighDpiPixmaps);
  //Display the OSD
  QWidget splash(0, Qt::Window | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
     splash.setWindowTitle("");
     splash.setStyleSheet("background: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5, stop:0 transparent, stop:0.52 transparent, stop:0.565 rgba(82, 121, 76, 33), stop:0.65 rgba(159, 235, 148, 64), stop:0.721925 rgba(255, 238, 150, 129), stop:0.77 rgba(255, 128, 128, 204), stop:0.89 rgba(191, 128, 255, 64), stop:1 transparent);");
     splash.setWindowOpacity(0);
     splash.resize(100,100);
  //Make sure it is centered on the current screen
  QPoint center = QCursor::pos();
  splash.move(center.x()-(splash.size().width()/2), center.y()-(splash.size().height()/2));
  splash.show();
  QDateTime end = QDateTime::currentDateTime().addMSecs(1000);
  while(QDateTime::currentDateTime() < end){ App.processEvents(); }
  splash.hide();
  return 0;
}
