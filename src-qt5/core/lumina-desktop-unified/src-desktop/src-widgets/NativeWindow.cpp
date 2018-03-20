//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "NativeWindow.h"

// === PUBLIC ===
NativeWindow::NativeWindow( NativeWindowObject *obj ) : QFrame(0, Qt::Window | Qt::FramelessWindowHint){
  WIN = obj;
  createFrame();
  WIN->addFrameWinID(this->winId());
  //Setup all the property connections
  connect(WIN, SIGNAL(winImageChanged()), this, SLOT(syncWinImage()):
  connect(WIN, SIGNAL(nameChanged()), this, SLOT(syncName()):
  connect(WIN, SIGNAL(titleChanged()), this, SLOT(syncTitle()):
  connect(WIN, SIGNAL(iconChanged()), this, SLOT(syncIcon()):
  connect(WIN, SIGNAL(stickyChanged()), this, SLOT(syncSticky()):
  connect(WIN, SIGNAL(visibilityChanged()), this, SLOT(syncVisibility()):
  connect(WIN, SIGNAL(winTypeChanged()), this, SLOT(syncWinType()):
  connect(WIN, SIGNAL(geomChanged()), this, SLOT(syncGeom()):
  //Now Perform the initial property loads
  syncWinImage();
  syncName();
  syncTitle();
  syncIcon();
  syncSticky();
  syncVisibility();
  syncWinType();
  syncGeom();
}

NativeWindow::~NativeWindow(){

}

// === PRIVATE ===
void NativeWindow::createFrame(){
  //Initialize the widgets
  closeB = new QToolButton(this);
    closeB->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  minB  = new QToolButton(this);
    minB->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  maxB  = new QToolButton(this);
    maxB->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  otherB = new QToolButton(this);
    otherB->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  toolbarL = new QHBoxLayout(this);
  vlayout = new QVBoxLayout(this);
    vlayout.align
  titleLabel = new QLabel(this);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  //Now put the widgets in the right places
  toolbarL->addWidget(otherB);
  toolbarL->addWidget(titleLabel);
  toolbarL->addWidget(minB);
  toolbarL->addWidget(maxB);
  toolbarL->addWidget(closeB);
  vlayout->addLayout(toolbarL);
  vlayout->addStretch();
  this->setLayout(vlayout);

  //
}

// === PRIVATE SLOTS ===

//Property Change slots
void NativeWindow::syncWinImage(){
 //Do nothing at the moment (compositing disabled)
}

void NativeWindow::syncName(){
  qDebug() << "Got Name Change:" << WIN->name();
}

void NativeWindow::syncTitle(){
  titleLabel->setText(WIN->title());
}

void NativeWindow::syncIcon(){
  //Do not use the "WIN->icon()" function, that is the URL format for QML
  otherB->setIcon( QIcon(WIN->property(NativeWindowObject::Icon).value<QIcon>()) );
}

void NativeWindow::syncSticky(){
  qDebug() << "Got Sticky Change:" << WIN->isSticky();
}

void NativeWindow::syncVisibility(){
  this->setVisible(WIN->isVisible());
}

void NativeWindow::syncWinType(){
  closeB->setVisible(WIN->showCloseButton());
  maxB->setVisible(WIN->showMaxButton());
  minB->setVisible(WIN->showMinButton());
  titleLabel->setVisible(WIN->showTitlebar());
  otherB->setVisible(WIN->showGenericButton());
  
}

void NativeWindow::syncGeom(){

}
