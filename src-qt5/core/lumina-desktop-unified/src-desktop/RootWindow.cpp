//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "RootWindow.h"

RootWindow::RootWindow(){
  root_win = QWindow::fromWinId( QX11Info::appRootWindow() ); //
  root_view = new QQuickView(root_win); //make it a child of the root window
  root_obj = RootDesktopObject::instance();
  syncRootSize();
  connect(root_win, SIGNAL(widthChanged(int)), this, SLOT(syncRootSize()) );
  connect(root_win, SIGNAL(heightChanged(int)),this, SLOT(syncRootSize()) );
  //Now setup the QQuickView
  root_view->setResizeMode(QQuickView::SizeRootObjectToView);
  root_view->engine()->rootContext()->setContextProperty("RootObject", root_obj);
  root_view->setSource(QUrl("qrc:///qml/RootDesktop"));
  root_view->show();
}

RootWindow::~RootWindow(){
  root_view->deleteLater();
  root_obj->deleteLater();
}

void RootWindow::syncRootSize(){
  if(root_win->width() != root_view->width() || root_win->height() != root_view->height()){
    root_view->setGeometry(0, 0, root_win->width(), root_win->height() );
    emit RootResized(root_view->geometry());
  }
}
