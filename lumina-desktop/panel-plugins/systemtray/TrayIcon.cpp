//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "TrayIcon.h"

#include <LSession.h>
#include <QScreen>
#include <LuminaX11.h>

TrayIcon::TrayIcon(QWidget *parent) : QWidget(parent){
  AID = 0; //nothing yet
  IID = 0;
  dmgID = 0;
}

TrayIcon::~TrayIcon(){
}

WId TrayIcon::appID(){
  return AID;
}

void TrayIcon::attachApp(WId id){
  if(id==0){ return; } //nothing to attach
  else if(AID!=0){ qWarning() << "Tray Icon is already attached to a window!"; return; }
  AID = id;
  IID = this->winId(); //embed directly into this widget
  dmgID = LSession::handle()->XCB->EmbedWindow(AID, IID);
  if( dmgID != 0 ){
    LSession::handle()->XCB->RestoreWindow(AID); //make it visible
    qDebug() << "New System Tray App:" << AID;
    QTimer::singleShot(1000, this, SLOT(updateIcon()) );
  }else{
    qWarning() << "Could not Embed Tray Application:" << AID;
    IID = 0;
    AID = 0;
  }
}

void TrayIcon::setSizeSquare(int side){
  this->setFixedSize( QSize(side, side) );
}

// ==============
//   PUBLIC SLOTS
// ==============
void TrayIcon::detachApp(){
  if(AID==0){ return; } //already detached
  qDebug() << "Detach App:" << AID;
  //Temporarily move the AID, so that internal slots do not auto-run
  WId tmp = AID;
  AID = 0;
  //Now detach the application window and clean up
  qDebug() << " - Unembed";
  LSession::handle()->XCB->UnembedWindow(tmp);
  qDebug() << " - finished app:" << tmp;
  IID = 0;
}

// ==============
//   PRIVATE SLOTS
// ==============
void TrayIcon::updateIcon(){
  if(AID==0){ return; }
  //Make sure the icon is square
  QSize icosize = this->size();
  LSession::handle()->XCB->ResizeWindow(AID,  icosize.width(), icosize.height());
  QTimer::singleShot(500, this, SLOT(update()) ); //make sure to re-draw the window in a moment
}

// =============
//     PROTECTED
// =============
void TrayIcon::paintEvent(QPaintEvent *event){
  QWidget::paintEvent(event); //make sure the background is already painted
  if(AID!=0){
    //qDebug() << "Paint Tray:" << AID;
    QPainter painter(this);
	//Now paint the tray app on top of the background
	//qDebug() << " - Draw tray:" << AID << IID << this->winId();
	//qDebug() << " - - " << event->rect().x() << event->rect().y() << event->rect().width() << event->rect().height();
	//qDebug() << " - Get image:" << AID;
	QPixmap pix;
	  //Try to grab the window directly with Qt
	  QList<QScreen*> scrnlist = QApplication::screens();
	  for(int i=0; i<scrnlist.length(); i++){
	      pix = scrnlist[i]->grabWindow(AID);
	      break; //stop here
	  }
	//qDebug() << " - Pix size:" << pix.size().width() << pix.size().height();
	//qDebug() << " - Geom:" << this->geometry().x() << this->geometry().y() << this->geometry().width() << this->geometry().height();
	if(!pix.isNull()){
	  if(this->size() != pix.size()){ QTimer::singleShot(10, this, SLOT(updateIcon())); }
	  painter.drawPixmap(0,0,this->width(), this->height(), pix.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation) );
	}else{
	  qWarning() << " - -  No Tray Icon/Image found!" << "ID:" << AID;
	}
    //qDebug() << " - Done";
  }
}

void TrayIcon::resizeEvent(QResizeEvent *event){
  //qDebug() << "Resize Event:" << event->size().width() << event->size().height();	
  if(AID!=0){
    LSession::handle()->XCB->ResizeWindow(AID,  event->size());
    QTimer::singleShot(500, this, SLOT(update()) ); //make sure to re-draw the window in a moment
  }
}
