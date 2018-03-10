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
