//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "RootSubWindow.h"
#include <QDebug>

// === PUBLIC ===
RootSubWindow::RootSubWindow(QMdiArea *root, WId window, Qt::WindowFlags flags) : \
	QMdiSubWindow(0, flags){
  this->setAttribute(Qt::WA_DeleteOnClose);
  //Create the QWindow and QWidget containers for the window
  closing = false;
  CID = window;
  WIN = QWindow::fromWinId(CID);
  WinWidget = QWidget::createWindowContainer( WIN, this);
  this->setWidget(WinWidget);
  //Hookup the signals/slots
  connect(this, SIGNAL(aboutToActivate()), this, SLOT(aboutToActivate()) );
  connect(WIN, SIGNAL(windowTitleChanged(const QString&)), this, SLOT(setWindowTitle(const QString&)) );
  connect(WIN, SIGNAL(heightChanged(int)), this, SLOT(adjustHeight(int) ));  
  connect(WIN, SIGNAL(widthChanged(int)), this, SLOT(adjustWidth(int) ));

  qDebug() << "Initial Window Geometry:" << WIN->geometry();
  qDebug() << "Initial Widget Geometry:" << WinWidget->geometry();
  qDebug() << "Minimums:";
  qDebug() << " - Height:" << WIN->minimumHeight();
  qDebug() << " - Width:" << WIN->minimumWidth();

  //this->resize(WinWidget->size());
  //Now add this window to the root QMdiArea
  root->addSubWindow(this); 
}

RootSubWindow::~RootSubWindow(){

}

WId RootSubWindow::id(){
  return CID;
}

// === PRIVATE ===

// === PUBLIC SLOTS ===
void RootSubWindow::clientClosed(){
  qDebug() << "Client Closed";
  closing = true;
  this->close();
}

void RootSubWindow::clientHidden(){
  qDebug() << "Client Hidden";
  this->hide();
}

void RootSubWindow::clientShown(){
  qDebug() << "Client Shown";
  this->show();
}

// === PRIVATE SLOTS ===
void RootSubWindow::aboutToActivate(){
  emit Activated(CID); //need to activate the subwindow - not the frame
  WIN->requestActivate();
}

void RootSubWindow::adjustHeight(int val){
  qDebug() << "Adjust height:" << val;
  WinWidget->resize(WinWidget->width(), val);
}

void RootSubWindow::adjustWidth(int val){
  qDebug() << "Adjust Width:" << val;
  WinWidget->resize(val, WinWidget->height());
}

// === PROTECTED ===
void RootSubWindow::closeEvent(QCloseEvent *ev){
  if(!closing){
    qDebug() << "Close Window By Button:" << CID;
    ev->ignore();
    WIN->destroy();
  }else{
    QMdiSubWindow::closeEvent(ev);
  }
  
}
