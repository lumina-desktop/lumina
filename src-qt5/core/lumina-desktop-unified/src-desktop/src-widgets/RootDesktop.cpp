//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include <global-objects.h>
#include "RootDesktop.h"
#include "Panel.h"

// === PUBLIC ===
RootDesktop::RootDesktop(QWindow *) : QWidget(0, Qt::Widget | Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint){

}

RootDesktop::~RootDesktop(){

}

void RootDesktop::start(){
  //qDebug() << "Starting RootDesktop" << this->geometry() << this->isVisible();
  bgTimer = new QTimer(this);
    bgTimer->setInterval(50);
    bgTimer->setSingleShot(true);
    connect(bgTimer, SIGNAL(timeout()), this, SLOT(bgTimerUpdate()) );

  cmenu = new DesktopContextMenu(this);

  //Setup the connections to the main objects
  connect(RootDesktopObject::instance(), SIGNAL(screensChanged()), this, SLOT(on_screensChanged()) );
  connect(RootDesktopObject::instance(), SIGNAL(panelsChanged()), this, SLOT(on_panelsChanged()) );
  connect(RootDesktopObject::instance(), SIGNAL(windowsChanged()), this, SLOT(on_windowsChanged()) );
  connect(RootDesktopObject::instance(), SIGNAL(trayWindowsChanged()), this, SLOT(on_trayWindowsChanged()) );

  connect(cmenu, SIGNAL(LockSession()), RootDesktopObject::instance(), SIGNAL(lockScreen()) );
  connect(cmenu, SIGNAL(showLeaveDialog()), RootDesktopObject::instance(), SIGNAL(startLogout()) );
  connect(cmenu, SIGNAL(LaunchStandardApplication(QString)), RootDesktopObject::instance(), SIGNAL(launchApplication(QString)) );
  connect(cmenu, SIGNAL(LaunchApplication(QString)), RootDesktopObject::instance(), SIGNAL(launchApplication(QString)) );

  on_screensChanged(); //make sure this is setup right away (sets up connections
  QTimer::singleShot(0, this, SLOT(on_panelsChanged()) );
  QTimer::singleShot(2, this, SLOT(on_windowsChanged()) );
  QTimer::singleShot(4, this, SLOT(on_trayWindowsChanged()) );

  //Now start the first-run of the background change system
  cmenu->start();
  bgTimer->start();
  this->show();
}

// === PRIVATE ===
//QImage bgimage;


// === PRIVATE SLOTS ===
//RootDesktopObject connections
void RootDesktop::on_screensChanged(){
  QStringList screens = RootDesktopObject::instance()->screens();
  //qDebug() << "Screens Changed:" << lastscreens << screens;
  for(int i=0; i<screens.length(); i++){
    if(!lastscreens.contains(screens[i])){
      connect(RootDesktopObject::instance()->screen(screens[i]), SIGNAL(backgroundChanged()), this, SLOT(on_screen_bg_changed()) );
    }
  }
  on_screen_bg_changed(); //start the timer to update the backgrounds now
  lastscreens = screens; //save this for later
}

void RootDesktop::on_panelsChanged(){
  QStringList pans = RootDesktopObject::instance()->panels();
  //Now find any new panels and create them as needed
  for(int i=0; i<pans.length(); i++){
    if(lastpanels.contains(pans[i])){ continue; } //already created
    //Need to create a new panel widget (self-maintained)
     new Panel( RootDesktopObject::instance()->panel(pans[i]) );
  }
  lastpanels = pans; //save this for the next time around
}

void RootDesktop::on_windowsChanged(){

}

void RootDesktop::on_trayWindowsChanged(){

}

void RootDesktop::on_screen_bg_changed(){
  if(!bgTimer->isActive()){ bgTimer->start(); }
}

//Internal use
void RootDesktop::bgTimerUpdate(){
  //qDebug() << "bgTimerUpdate";
  //QtConcurrent::run(this, &RootDesktop::updateBG, this);
  updateBG(this);
}

void RootDesktop::updateBG(RootDesktop* obj){

  QImage tmp(obj->size(), QImage::Format_ARGB32_Premultiplied);
  QStringList scr = RootDesktopObject::instance()->screens();
  //qDebug() << "updateBG" << scr << tmp.size() << obj->geometry();
  QPainter imgpaint(&tmp);
  for(int i=0; i<scr.length(); i++){
    ScreenObject * screen = RootDesktopObject::instance()->screen(scr[i]);
    QString file = screen->background(); //in URL format
    //qDebug() << "Got BG File:" << file << QUrl(file).toLocalFile();
    QImage img(QUrl(file).toLocalFile());
    //qDebug() << " - BG File is null:" << img.isNull();
    imgpaint.drawImage(screen->geometry(), img, QRect(0,0,img.width(), img.height()) );
  }
  bgimage = tmp;
  //QTimer::singleShot(0, obj, SLOT(update()) );
  obj->update();
}

// === PROTECTED ===
void RootDesktop::paintEvent(QPaintEvent *ev){
  //qDebug() << "Paint Event:" << bgimage.isNull();
  if (!bgimage.isNull()) {
    //qDebug() << "Wallpaper paint Event:" << ev->rect();
    QPainter painter(this);
    painter.drawImage(ev->rect(), bgimage, ev->rect());
  }else{
    QWidget::paintEvent(ev);
  }
}
