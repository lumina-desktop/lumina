//===========================================
//  Lumina-DE source code
//  Copyright (c) 2020, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This function provides a simple visual ping for 8k and higher desktops to find the cursor.
// This is clearly a 1st world problem, but it's still a problem that people with ultra high 
// resolution desktops will face if they utilize a lower than normal DPI.
//===========================================

#include <QApplication>
#include <QPixmap>
#include <QLabel>
#include <QDateTime>
#include <QPoint>
#include <QScreen>
#include <QDesktopWidget>
#include <QPropertyAnimation>
#include <QThread>

int main(int argc, char **argv){
  //Setup the application
  QApplication App(argc, argv);
    App.setAttribute(Qt::AA_UseHighDpiPixmaps);
  //Display the OSD
  QLabel splash(0, Qt::Window | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint | Qt::NoDropShadowWindowHint);
     splash.setWindowTitle("");
     splash.resize(100,100);
  QWidget overlay( &splash );
    overlay.setStyleSheet("margin: 0px; border: none; background: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5, stop:0 rgba(0, 0, 0, 0), stop:0.52 rgba(0, 0, 0, 0), stop:0.565 rgba(82, 121, 76, 33), stop:0.65 rgba(159, 235, 148, 64), stop:0.721925 rgba(255, 238, 150, 129), stop:0.77 rgba(130, 205, 47, 204), stop:0.89 rgba(100, 128, 255, 64), stop:1 rgba(0, 0, 0, 0));");
    overlay.setGeometry(QRect(0,0,100,100));
  //Make sure it is centered on the current screen
  QPoint center = QCursor::pos();
  splash.move(center.x()-(splash.size().width()/2), center.y()-(splash.size().height()/2));
  splash.setPixmap(QApplication::screens().at(0)->grabWindow(QApplication::desktop()->winId(), splash.x(), splash.y(), 100,100) );
  splash.show();
  QPropertyAnimation anim(&splash,"windowOpacity");
  anim.setDuration(1200);
  anim.setStartValue(1);
  anim.setKeyValueAt(0.30, 0); anim.setKeyValueAt(0.31, 1);
  anim.setKeyValueAt(0.60, 0); anim.setKeyValueAt(0.61, 1);
  anim.setEndValue(0);
  anim.start();
  while(anim.state() != QAbstractAnimation::Stopped){ QThread::msleep(20); App.processEvents(); } //60 fps
  splash.hide();
  return 0;
}
