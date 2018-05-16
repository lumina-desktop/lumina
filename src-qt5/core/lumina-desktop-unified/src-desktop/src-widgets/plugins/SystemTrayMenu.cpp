//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "SystemTrayMenu.h"

#include <global-objects.h>

SystemTrayMenu::SystemTrayMenu() : QMenu() {
  this->setWindowFlags(Qt::Popup | Qt::WindowTransparentForInput);
  widget = new QWidget();
  layout = new QGridLayout(widget);
  WA = new QWidgetAction(this);
    WA->setDefaultWidget(widget);
  this->addAction(WA);
  connect(RootDesktopObject::instance(), SIGNAL(trayWindowsChanged()), this, SLOT(trayWindowsChanged()) );
  QTimer::singleShot(50, this, SLOT(trayWindowsChanged()) ); //first-time load of tray windows
}

SystemTrayMenu::~SystemTrayMenu(){
  widget->deleteLater();
}

int SystemTrayMenu::numTrayIcons(){
  return TIcons.count();
}

void SystemTrayMenu::trayWindowsChanged(){
  //Clear all the tray icons
  for(int i=0; i<TIcons.length(); i++){ layout->removeWidget(TIcons[i]); TIcons[i]->deleteLater(); }
  TIcons.clear();
  //Now generate all the tray icons
  QList<NativeWindowObject*> wins = RootDesktopObject::instance()->trayWindowObjects();
  for(int i=0; i<wins.length(); i++){
    TrayIcon *tmp = new TrayIcon(this, wins[i]);
	connect(this, SIGNAL(aboutToShow()), tmp, SLOT(aboutToShow()) );
    TIcons << tmp;
    layout->addWidget(tmp, i/3, i%3); //3 columns of icons
  }
  emit hasTrayIcons( numTrayIcons()>0 );
}

// ================
//    TrayIcon
// ================
TrayIcon::TrayIcon(QWidget *parent, NativeWindowObject *win) : QLabel(parent){
  WIN = win;
  connect(win, SIGNAL(iconChanged()), this, SLOT(updateIcon()) );
}

TrayIcon::~TrayIcon(){

}

void TrayIcon::updateIcon(){
  //Just set a hard 48x48 pixel size for now
  // It is small enough that most apps support it, but large enough to be visible on high-DPI screens
  this->setPixmap( WIN->property(NativeWindowObject::Icon).value<QIcon>().pixmap(QSize(48,48)) );
}

void TrayIcon::aboutToShow(){
  WIN->setGeometryNow( QRect(this->mapToGlobal(this->geometry().topLeft()), QSize(48,48)) );
}
