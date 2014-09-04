//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "TrayIcon.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xdamage.h>

static Damage dmgID = 0;

TrayIcon::TrayIcon(QWidget *parent) : QWidget(parent){
  AID = 0; //nothing attached yet
  IID = 0;
}

TrayIcon::~TrayIcon(){
  if(AID!=0){
    detachApp();
  }
}

WId TrayIcon::appID(){
  return AID;
}

void TrayIcon::attachApp(WId id){
  if(id==0){ return; } //nothing to attach
  else if(AID!=0){ qWarning() << "Tray Icon is already attached to a window!"; return; }
  AID = id;
  //qDebug() << "Container:" << this->winId();
  //qDebug() << " - Tray:" << AID;
  QTimer::singleShot(0,this,SLOT(slotAttach()) );
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
  LX11::UnembedWindow(tmp);
  if(dmgID!=0){
    XDamageDestroy(QX11Info::display(), dmgID);
  }
  qDebug() << " - finished app:" << tmp;
  //if(IID!=this->winId()){ LX11::DestroyWindow(IID); }
  IID = 0;
  emit AppClosed();
}

// ==============
//   PRIVATE SLOTS
// ==============
void TrayIcon::slotAttach(){
  IID = this->winId(); //embed directly into this widget
  //IID = LX11::CreateWindow( this->winId(), this->rect() ); //Create an intermediate window to be the parent
  if( LX11::EmbedWindow(AID, IID) ){
    LX11::RestoreWindow(AID); //make it visible
    //XSelectInput(QX11Info::display(), AID, StructureNotifyMask);
    dmgID = XDamageCreate( QX11Info::display(), AID, XDamageReportRawRectangles );
    updateIcon();
    qDebug() << "New System Tray App:" << AID;
    emit AppAttached();
    QTimer::singleShot(500, this, SLOT(updateIcon()) );
  }else{
    qWarning() << "Could not Embed Tray Application:" << AID;
    //LX11::DestroyWindow(IID);
    IID = 0;
    AID = 0;
    emit AppClosed();
  }
}

void TrayIcon::updateIcon(){
  if(AID==0){ return; }
  //Make sure the icon is square
  QSize icosize = this->size();
  LX11::ResizeWindow(AID,  icosize.width(), icosize.height());
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
	//qDebug() << " - Get image";
	QPixmap pix = LX11::WindowImage(AID, false);
	if(pix.isNull()){
	  //Try to grab the window directly with Qt
	  //qDebug() << " - Grab window directly";
	  pix = QPixmap::grabWindow(AID);
	}
	//qDebug() << " - Pix size:" << pix.size().width() << pix.size().height();
	//qDebug() << " - Geom:" << this->geometry().x() << this->geometry().y() << this->geometry().width() << this->geometry().height();
	painter.drawPixmap(0,0,this->width(), this->height(), pix.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation) );
    //qDebug() << " - Done";
  }
}

/*void TrayIcon::moveEvent(QMoveEvent *event){
  //Make sure the main Tray window is right underneath the widget
  //qDebug() << "Move Event:" << event->pos().x() << event->pos().y();
  LX11::MoveResizeWindow(AID, QRect( this->mapToGlobal(event->pos()), this->size()) );
  QWidget::moveEvent(event);
}*/

void TrayIcon::resizeEvent(QResizeEvent *event){
  //qDebug() << "Resize Event:" << event->size().width() << event->size().height();	
  if(AID!=0){
    LX11::ResizeWindow(AID,  event->size().width(), event->size().height());
  }
}

/*bool TrayIcon::x11Event(XEvent *event){
  qDebug() << "XEvent";
  if( event->xany.window==AID || event->type==( (int)dmgID+XDamageNotify) ){
    qDebug() << "Tray X Event:" << AID;
    this->update(); //trigger a repaint
    return true;
  }else{
    return false; //no special handling
  }
}*/
