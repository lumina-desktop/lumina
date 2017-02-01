//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "RootSubWindow.h"
#include <QDebug>

// === PUBLIC ===
RootSubWindow::RootSubWindow(QMdiArea *root, NativeWindow *win) : QMdiSubWindow(0){
  this->setAttribute(Qt::WA_DeleteOnClose);
  //Create the QWindow and QWidget containers for the window
  WIN = win;
  closing = false;
  WinWidget = QWidget::createWindowContainer( WIN->window(), this);
  this->setWidget(WinWidget);
  LoadProperties( QList< NativeWindow::Property>() << NativeWindow::WindowFlags << NativeWindow::Title << NativeWindow::Icon  \
			<< NativeWindow::MinSize << NativeWindow::MaxSize << NativeWindow::Size );
  //Hookup the signals/slots
  connect(this, SIGNAL(aboutToActivate()), this, SLOT(aboutToActivate()) );
  connect(WIN, SIGNAL(PropertyChanged(NativeWindow::Property, QVariant)), this, SLOT(propertyChanged(NativeWindow::Property, QVariant)));
  //Now add this window to the root QMdiArea
  root->addSubWindow(this); 
  //Make sure the visibily property only gets loaded after it is added to the root area
  propertyChanged(NativeWindow::Visible, WIN->property(NativeWindow::Visible));
}

RootSubWindow::~RootSubWindow(){

}

WId RootSubWindow::id(){
  return WIN->id();
}

// === PRIVATE ===
void RootSubWindow::LoadProperties( QList< NativeWindow::Property> list){
  for(int i=0; i<list.length(); i++){
    propertyChanged( list[i], WIN->property(list[i]) );
  }
}

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
  WIN->emit RequestActivate(WIN->id());
}

void RootSubWindow::propertyChanged(NativeWindow::Property prop, QVariant val){
  if(val.isNull()){ return; } //not the same as a default/empty value - the property has just not been set yet
  switch(prop){
	case NativeWindow::Visible:
		if(val.toBool()){ clientShown(); }
		else{ clientHidden(); }
		break;
	case NativeWindow::Title:
		this->setWindowTitle(val.toString());
		break;
	case NativeWindow::Icon:
		this->setWindowIcon(val.value< QIcon>());
		break;
	case NativeWindow::Size:
		this->resize(val.toSize());
		break;
	case NativeWindow::MinSize:
		this->setMinimumSize(val.toSize());
		break;
	case NativeWindow::MaxSize:
		this->setMaximumSize(val.toSize());
		break;
	case NativeWindow::Active:
		if(val.toBool()){ this->mdiArea()->setActiveSubWindow(this); }
		break;
	case NativeWindow::WindowFlags:
		this->setWindowFlags( val.value< Qt::WindowFlags >() );
		break;
	default:
		qDebug() << "Window Property Unused:" << prop << val;
  }
}

// === PROTECTED ===
void RootSubWindow::closeEvent(QCloseEvent *ev){
  if(!closing){
    //qDebug() << "Close Window By Button:" << WIN->id();
    ev->ignore();
    WIN->emit RequestClose(WIN->id());
  }else{
    QMdiSubWindow::closeEvent(ev);
  }
  
}
