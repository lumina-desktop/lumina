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
  connect(WIN, SIGNAL(winImageChanged()), this, SLOT(syncWinImage()) );
  connect(WIN, SIGNAL(nameChanged()), this, SLOT(syncName()) );
  connect(WIN, SIGNAL(titleChanged()), this, SLOT(syncTitle()) );
  connect(WIN, SIGNAL(iconChanged()), this, SLOT(syncIcon()) );
  connect(WIN, SIGNAL(stickyChanged()), this, SLOT(syncSticky()) );
  connect(WIN, SIGNAL(visibilityChanged()), this, SLOT(syncVisibility()) );
  connect(WIN, SIGNAL(winTypeChanged()), this, SLOT(syncWinType()) );
  connect(WIN, SIGNAL(geomChanged()), this, SLOT(syncGeom()) );
  connect(WIN, SIGNAL(WindowClosed(WId)), this, SLOT(deleteLater()) );
  //Now Perform the initial property loads
  syncWinImage();
  syncName();
  syncTitle();
  syncIcon();
  syncSticky();
  syncVisibility();
  syncWinType();
  syncGeom();
  //Setup all the button connections
  connect(minB, SIGNAL(clicked()), WIN, SLOT(toggleVisibility()) );
  connect(maxB, SIGNAL(clicked()), WIN, SLOT(toggleMaximize()) );
  connect(closeB, SIGNAL(clicked()), WIN, SLOT(requestClose()) );


}

NativeWindow::~NativeWindow(){
  vlayout->deleteLater();
  toolbarL->deleteLater();
}

// === PRIVATE ===
void NativeWindow::createFrame(){
  //Initialize the widgets
  closeB = new QToolButton(this);
    closeB->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    closeB->setAutoRaise(true);
  minB  = new QToolButton(this);
    minB->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    minB->setAutoRaise(true);
  maxB  = new QToolButton(this);
    maxB->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    maxB->setAutoRaise(true);
  otherB = new QToolButton(this);
    otherB->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    otherB->setAutoRaise(true);
  vlayout = new QVBoxLayout();
    vlayout->setSpacing(0);
  toolbarL = new QHBoxLayout();
    toolbarL->setSpacing(0);

    //vlayout.align
  titleLabel = new QLabel(this);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  //contentW = new QWidget(this);
    //contentW->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  //Now put the widgets in the right places
  toolbarL->addWidget(otherB);
  toolbarL->addWidget(titleLabel);
  toolbarL->addWidget(minB);
  toolbarL->addWidget(maxB);
  toolbarL->addWidget(closeB);
  vlayout->addLayout(toolbarL);
  vlayout->addStretch();
  this->setLayout(vlayout);
  // Load the icons for the buttons
  loadIcons();
}

void NativeWindow::loadIcons(){
    closeB->setIcon( QIcon::fromTheme("window-close") );
    minB->setIcon( QIcon::fromTheme("window-minimize") );
    maxB->setIcon( QIcon::fromTheme("window-maximize") );
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
  titleLabel->setToolTip(WIN->title());
}

void NativeWindow::syncIcon(){
  //Do not use the "WIN->icon()" function, that is the URL format for QML
  otherB->setIcon( QIcon(WIN->property(NativeWindowObject::Icon).value<QIcon>()) );
}

void NativeWindow::syncSticky(){
  qDebug() << "Got Sticky Change:" << WIN->isSticky();
}

void NativeWindow::syncVisibility(){
  qDebug() << "Sync Visibility:" << WIN->isVisible();
  this->setVisible(WIN->isVisible());
}

void NativeWindow::syncWinType(){
  qDebug() << "Sync Win Type";
  closeB->setVisible(WIN->showCloseButton());
  maxB->setVisible(WIN->showMaxButton());
  minB->setVisible(WIN->showMinButton());
  titleLabel->setVisible(WIN->showTitlebar());
  otherB->setVisible(WIN->showGenericButton());
  //toolbarL->setVisible(WIN->showTitlebar());

  //Update all the margins for the frame
  QList<int> frame = WIN->property(NativeWindowObject::FrameExtents).value<QList<int> >();
  /*QList<int> : [Left, Right, Top, Bottom] in pixels */
  vlayout->setContentsMargins( frame[0], frame[1], 0, frame[3]);
  int topM = frame[2] - titleLabel->fontMetrics().height(); //figure out how much extra we have to work with
  if(topM<0){ topM = 0; }
  int botM = topM/2.0;
  toolbarL->setContentsMargins( 0, 0, topM-botM, botM);
  //QPoint containerCorner(frame[0], topM-botM);
  //WIN->emit RequestReparent(WIN->id(), this->winId(), containerCorner);
}

void NativeWindow::syncGeom(){
  qDebug() << "Sync Geometry:" << WIN->name();
  qDebug() << "  Frame:" << WIN->frameGeometry() << "Win:" << WIN->imageGeometry();
  this->setGeometry( WIN->frameGeometry() );
}
