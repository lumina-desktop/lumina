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
  badpaints = 0;
  if("1" == QString(getenv("QT_AUTO_SCREEN_SCALE_FACTOR")) ){
  scalefactor = 2; //Auto-adjust this later to the physicalDotsPerInch of the current screen
  }else{ scalefactor = 1; }
  //this->setLayout(new QHBoxLayout);
  //this->layout()->setContentsMargins(0,0,0,0);
}

TrayIcon::~TrayIcon(){
}

void TrayIcon::cleanup(){
  AID = IID = 0;
}

WId TrayIcon::appID(){
  return AID;
}

void TrayIcon::attachApp(WId id){
  if(id==0){ return; } //nothing to attach
  else if(AID!=0){ qWarning() << "Tray Icon is already attached to a window!"; return; }
  AID = id;
  //WIN = QWindow::fromWinId(AID);
  //connect(WIN, SIGNAL(
  //this->layout()->addWidget( QWidget::createWindowContainer(WIN, this) );
  IID = this->winId(); //embed directly into this widget
  dmgID = LSession::handle()->XCB->EmbedWindow(AID, IID);
  if( dmgID != 0 ){
    LSession::handle()->XCB->RestoreWindow(AID); //make it visible
    //qDebug() << "New System Tray App:" << AID;
    QTimer::singleShot(1000, this, SLOT(updateIcon()) );
  }else{
    //qWarning() << "Could not Embed Tray Application:" << AID;
    IID = 0;
    AID = 0;
  }
}

void TrayIcon::setSizeSquare(int side){
  //qDebug() << " Set Fixed Systray size:" << side;
  this->setFixedSize( QSize(side, side) );
}

// ==============
//   PUBLIC SLOTS
// ==============
void TrayIcon::detachApp(){
  if(AID==0){ return; } //already detached
  //qDebug() << "Detach App:" << AID;
  //Temporarily move the AID, so that internal slots do not auto-run
  WId tmp = AID;
  AID = 0;
  //Now detach the application window and clean up
  //qDebug() << " - Unembed";
  //WIN->setParent(0); //Reset parentage back to the main stack
  LSession::handle()->XCB->UnembedWindow(tmp);
  //qDebug() << " - finished app:" << tmp;
  IID = 0;
}

// ==============
//   PRIVATE SLOTS
// ==============
void TrayIcon::updateIcon(){
  if(AID==0){ return; }
  //Make sure the icon is square
  QSize icosize = this->size();
  LSession::handle()->XCB->ResizeWindow(AID,  icosize.width()*scalefactor, icosize.height()*scalefactor);
  QTimer::singleShot(500, this, SLOT(repaint()) ); //make sure to re-draw the window in a moment
}

// =============
//     PROTECTED
// =============
void TrayIcon::paintEvent(QPaintEvent *event){
  QWidget::paintEvent(event); //make sure the background is already painted
  if(AID!=0){
    //Update the background on the tray app
    //qDebug() << "Paint Tray Background";
    //LSession::handle()->XCB->SetWindowBackground(this, this->geometry(), AID);
    //qDebug() << "Paint Tray:" << AID;
    QPainter painter(this);
	//Now paint the tray app on top of the background
	//qDebug() << " - Draw tray:" << AID << IID << this->winId();
	//qDebug() << " - - " << event->rect().x() << event->rect().y() << event->rect().width() << event->rect().height();
	//qDebug() << " - Get image:" << AID;
	QPixmap pix = LSession::handle()->XCB->TrayImage(AID); //= WIN->icon().pixmap(this->size());

	//qDebug() << " - Pix size:" << pix.size().width() << pix.size().height();
	//qDebug() << " - Geom:" << this->geometry().x() << this->geometry().y() << this->geometry().width() << this->geometry().height();
	if(!pix.isNull()){
	  if((this->size()*scalefactor) != pix.size()){ QTimer::singleShot(10, this, SLOT(updateIcon())); return; }
	  painter.drawPixmap(0,0,this->width(), this->height(), pix.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation) );
	  badpaints = 0; //good paint
	}else{
	  badpaints++;
	  if(badpaints>5){
	    qWarning() << " - -  No Tray Icon/Image found!" << "ID:" << AID;
	    AID = 0; //reset back to nothing
	    IID = 0;
	    emit BadIcon(); //removed/destroyed in some non-valid way?
	  }
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
