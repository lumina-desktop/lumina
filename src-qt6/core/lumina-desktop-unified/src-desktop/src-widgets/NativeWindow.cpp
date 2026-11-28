//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "NativeWindow.h"

#include <QWidget>
#include <QWindow>

// === PUBLIC ===
NativeWindow::NativeWindow( NativeWindowObject *obj ) : QFrame(0, Qt::Window | Qt::FramelessWindowHint){
  WIN = obj;
  createFrame();
  this->setMouseTracking(true);
  //Setup the timer object to syncronize info
  moveTimer = new QTimer(this);
  moveTimer->setSingleShot(true);
  moveTimer->setInterval(100); //1/10 second
  connect(moveTimer, SIGNAL(timeout()), this, SLOT(submitLocationChange()) ); //Let the window system know the window has moved
  resizeTimer = new QTimer(this);
  resizeTimer->setSingleShot(true);
  resizeTimer->setInterval(10); //1/10 second
  connect(resizeTimer, SIGNAL(timeout()), this, SLOT(submitSizeChange()) ); //Let the window system know the window has moved

  WIN->addFrameWinID(this->winId());
}

NativeWindow::~NativeWindow(){
  vlayout->deleteLater();
  toolbarL->deleteLater();
}

QPoint NativeWindow::relativeOrigin(){
  //Update all the margins for the frame
  QList<int> frame = WIN->property(NativeWindowObject::FrameExtents).value<QList<int> >();
  //QList<int> : [Left, Right, Top, Bottom] in pixels
  QPoint containerCorner(frame[0], frame[2]);
  //qDebug() << "Got Container Corner:" << containerCorner << "L,R,T,B:" << frame;
  return containerCorner;
  //return QPoint(0,0);
}

void NativeWindow::initProperties(){
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

  //Setup all the button connections
  connect(minB, SIGNAL(clicked()), WIN, SLOT(toggleVisibility()) );
  connect(maxB, SIGNAL(clicked()), WIN, SLOT(toggleMaximize()) );
  connect(closeB, SIGNAL(clicked()), WIN, SLOT(requestClose()) );

  //Now Perform the initial property loads
  syncWinImage();
  syncName();
  syncTitle();
  syncIcon();
  syncSticky();
  syncWinType();
  syncGeom();
  syncVisibility(true); //init visibility - force it visible to start with
  container->activateWindow();
}

//Mouse Interactivity
void NativeWindow::startMoving(){
  //If the cursor is not over this window, move it to the center of the titlebar
  QPoint curpt = QCursor::pos(); //global coords
  if(!this->geometry().contains(curpt)){
    curpt = this->mapToGlobal(titleLabel->geometry().center());
    QCursor::setPos(curpt);
  }
  //Calculate the offset
  activeState = Move;
  offset = this->mapFromGlobal(curpt);
  setMouseCursor(activeState, true); //this one is an override cursor
  //WinWidget->pause();
  this->grabMouse();
}

void NativeWindow::startResizing(){
  activeState = getStateAtPoint( this->mapFromGlobal(QCursor::pos()), true); //also have it set the offset variable
  setMouseCursor(activeState, true); //this one is an override cursor
  //WinWidget->pause();
  this->grabMouse();
}

// === PRIVATE ===
NativeWindow::ModState NativeWindow::getStateAtPoint(QPoint pt, bool setoffset){
  //Note: pt should be in widget-relative coordinates, not global
  QList<int> frame = WIN->property(NativeWindowObject::FrameExtents).value<QList<int> >();
  /*QList<int> : [Left, Right, Top, Bottom] in pixels */
  bool onLeft = (pt.x()<=frame[0]);
  bool onRight = (pt.x() >= (this->width()-frame[1])) && pt.x() <= this->width();
  bool onTop = (pt.y() < titleLabel->y() ); //be careful about this one - the top frame gets split up and the title bar put in
  bool onBottom = (pt.y() >= (this->height()-frame[3]) && (pt.y() <= this->height()) );
  if(onLeft || onRight || onTop || onBottom){
    //above the frame itself - need to figure out which quadrant it is in (8-directions)
    if(onTop){
      //One of the top options
      if(onLeft){
	if(setoffset){ offset.setX(pt.x()); offset.setY(pt.y()); } //difference from top-left corner
	return ResizeTopLeft;
      }else if(onRight){
	if(setoffset){ offset.setX(pt.x()-this->width()); offset.setY(pt.y()); } //difference from top-right corner
	return ResizeTopRight;
      }else{
	if(setoffset){ offset.setX(0); offset.setY(pt.y()); } //difference from top edge (X does not matter)
	return ResizeTop;
      }
    }else if(onBottom){
      //One of the bottom options
      if(onLeft){
	if(setoffset){ offset.setX(pt.x()); offset.setY(pt.y()-this->height()); } //difference from bottom-left corner
	return ResizeBottomLeft;
      }else if(onRight){
	if(setoffset){ offset.setX(pt.x()-this->width()); offset.setY(pt.y()-this->height()); } //difference from bottom-right corner
	return ResizeBottomRight;
      }else{
	if(setoffset){ offset.setX(0); offset.setY(pt.y()-this->height()); } //difference from bottom edge (X does not matter)
	return ResizeBottom;
      }
    }else if(onLeft){
	if(setoffset){ offset.setX(pt.x()); offset.setY(0); } //difference from left edge (Y does not matter)
	return ResizeLeft;
    }else if(onRight){
	if(setoffset){ offset.setX(pt.x()-this->width()); offset.setY(0); } //difference from right edge (Y does not matter)
	return ResizeRight;
    }else{
	return Normal;
    }
  }
  //if it gets this far just return normal
  return Normal;
}

void NativeWindow::setMouseCursor(ModState state, bool override){
  if(currentCursor==state && !override){ return; } //nothing to change
  Qt::CursorShape shape;
  switch(state){
    case Normal:
      shape = Qt::ArrowCursor;
      break;
    case Move:
      shape = Qt::SizeAllCursor;
      break;
    case ResizeTop:
      shape = Qt::SizeVerCursor;
      break;
    case ResizeTopRight:
      shape = Qt::SizeBDiagCursor;
      break;
    case ResizeRight:
      shape = Qt::SizeHorCursor;
      break;
    case ResizeBottomRight:
      shape = Qt::SizeFDiagCursor;
      break;
    case ResizeBottom:
      shape = Qt::SizeVerCursor;
      break;
    case ResizeBottomLeft:
      shape = Qt::SizeBDiagCursor;
      break;
    case ResizeLeft:
      shape = Qt::SizeHorCursor;
      break;
    case ResizeTopLeft:
      shape = Qt::SizeFDiagCursor;
      break;
  }
  if(override){
    QApplication::setOverrideCursor(QCursor(shape));
  }else{
    currentCursor = state;
    this->setCursor(shape);
  }
}

void NativeWindow::createFrame(){
  //Initialize the widgets
  closeB = new QToolButton(this);
    closeB->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    closeB->setAutoRaise(true);
    closeB->setCursor(Qt::ArrowCursor);
  minB  = new QToolButton(this);
    minB->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    minB->setAutoRaise(true);
    minB->setCursor(Qt::ArrowCursor);
  maxB  = new QToolButton(this);
    maxB->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    maxB->setAutoRaise(true);
    maxB->setCursor(Qt::ArrowCursor);
  otherB = new QToolButton(this);
    otherB->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    otherB->setAutoRaise(true);
    otherB->setCursor(Qt::ArrowCursor);
  vlayout = new QVBoxLayout();
    vlayout->setSpacing(0);
  toolbarL = new QHBoxLayout();
    toolbarL->setSpacing(0);
  qDebug() << "Create Native Embed Widget";
  container = new NativeEmbedWidget(this, WIN);
    container->widget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //vlayout.align
  titleLabel = new QLabel(this);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    titleLabel->setCursor(Qt::ArrowCursor);
  //Now put the widgets in the right places
  toolbarL->addWidget(otherB);
  toolbarL->addWidget(titleLabel);
  toolbarL->addWidget(minB);
  toolbarL->addWidget(maxB);
  toolbarL->addWidget(closeB);
  vlayout->addLayout(toolbarL);
  //vlayout->addStretch();
  vlayout->addWidget(container->widget());
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
 //Do nothing at the moment (internal compositing disabled)
}

void NativeWindow::syncName(){
  //qDebug() << "Got Name Change:" << WIN->name();
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
  //qDebug() << "Got Sticky Change:" << WIN->isSticky();
}

void NativeWindow::syncVisibility(bool init){
  if(init){
    WIN->setProperty(NativeWindowObject::Visible, true, true); //force it
  }else if(this->isVisible() != WIN->isVisible()){
    //qDebug() << "Sync Visibility:" << WIN->isVisible();
    this->setVisible(WIN->isVisible());
  }
}

void NativeWindow::syncWinType(){
  //qDebug() << "Sync Win Type";
  closeB->setVisible(WIN->showCloseButton());
  maxB->setVisible(WIN->showMaxButton());
  minB->setVisible(WIN->showMinButton());
  titleLabel->setVisible(WIN->showTitlebar());
  otherB->setVisible(WIN->showGenericButton());
  //toolbarL->setVisible(WIN->showTitlebar());

  //Update all the margins for the frame
  QList<int> frame = WIN->property(NativeWindowObject::FrameExtents).value<QList<int> >();
  /*QList<int> : [Left, Right, Top, Bottom] in pixels */
  //qDebug() << "Setup Frame Margins:" << frame;
  vlayout->setContentsMargins( frame[0], 0,frame[1], frame[3]);
  int topM = frame[2] - titleLabel->fontMetrics().height(); //figure out how much extra we have to work with
  if(topM<0){ topM = 0; }
  int botM = topM/2.0;
  toolbarL->setContentsMargins( 0, topM-botM, 0, botM);
  //QPoint containerCorner(frame[0], topM-botM);
  //WIN->emit RequestReparent(WIN->id(), this->winId(), containerCorner);
}

void NativeWindow::syncGeom(){
  qDebug() << "Sync Geometry:" << WIN->name();
  qDebug() << "  Frame:" << WIN->frameGeometry() << "Win:" << WIN->imageGeometry();
  QRect geom = WIN->frameGeometry();
  if(geom!=this->geometry()){
    this->setGeometry( geom );
  }
}

void NativeWindow::submitLocationChange(){
  emit windowMoved(WIN);
}

void NativeWindow::submitSizeChange(){
  if(!pending_geom.isNull()){
    this->setGeometry(pending_geom);
    pending_geom = QRect();
  }
}

// === PROTECTED ===
void NativeWindow::mousePressEvent(QMouseEvent *ev){
  container->activateWindow();
  this->raise();
  QFrame::mousePressEvent(ev);
  //qDebug() << "Frame Mouse Press Event";
  if(activeState != Normal){ return; } // do nothing - already in a state of grabbed mouse
  offset.setX(0); offset.setY(0);
  if(ev->button()==Qt::LeftButton){
    if(this->childAt(ev->pos())!=0){
      //Clicked on the titlebar
      startMoving();
    }else{
      //Clicked on the frame somewhere
      startResizing();
    }
  }

}

void NativeWindow::mouseMoveEvent(QMouseEvent *ev){
  //qDebug() << "Got Mouse Move Event:" << ev->pos();
  QFrame::mouseMoveEvent(ev);
  if(activeState == Normal){
    setMouseCursor( getStateAtPoint(ev->pos()) ); //just update the mouse cursor
  }else{
    //Currently in a modification state
    QRect geom = this->geometry();
    QList<int> frame = WIN->property(NativeWindowObject::FrameExtents).value<QList<int> >();
    /*QList<int> : [Left, Right, Top, Bottom] in pixels */
    QSize minsize(container->widget()->minimumSize().width() + frame[0]+frame[1], container->widget()->minimumSize().height()+frame[2]+frame[3] );
    switch(activeState){
      case Move:
        geom.moveTopLeft(ev->globalPos()-offset); //will not change size
        break;
      case ResizeTop:
        geom.setTop(ev->globalPos().y()-offset.y());
        if(geom.size().height() < minsize.height()){
	  geom.setTop(geom.y() - (minsize.height()-geom.size().height())); //reset back to min height
        }
        break;
      case ResizeTopRight:
        geom.setTopRight(ev->globalPos()-offset);
        if(geom.size().height() < minsize.height()){
	  geom.setTop(geom.y() - (minsize.height()-geom.size().height())); //reset back to min height
        }
        if(geom.size().width() < minsize.width()){
	  geom.setRight(geom.x() + minsize.width()); //reset back to min width
        }
        break;
      case ResizeRight:
        geom.setRight(ev->globalPos().x()-offset.x());
        if(geom.size().width() < minsize.width()){
	  geom.setRight(geom.x() + minsize.width()); //reset back to min width
        }
        break;
      case ResizeBottomRight:
        geom.setBottomRight(ev->globalPos()-offset);
        if(geom.size().height() < minsize.height()){
	  geom.setBottom(geom.y() + minsize.height()); //reset back to min height
        }
        if(geom.size().width() < minsize.width()){
	  geom.setRight(geom.x() + minsize.width()); //reset back to min width
        }
        break;
      case ResizeBottom:
        geom.setBottom(ev->globalPos().y()-offset.y());
        if(geom.size().height() < minsize.height()){
	  geom.setBottom(geom.y() + minsize.height()); //reset back to min height
        }
        break;
      case ResizeBottomLeft:
        geom.setBottomLeft(ev->globalPos()-offset);
        if(geom.size().height() < minsize.height()){
	  geom.setBottom(geom.y() + minsize.height()); //reset back to min height
        }
        if(geom.size().width() < minsize.width()){
	  geom.setLeft(geom.x() - (minsize.width()-geom.size().width())); //reset back to min width
        }
        break;
      case ResizeLeft:
        geom.setLeft(ev->globalPos().x()-offset.x());
        if(geom.size().width() < minsize.width()){
	  geom.setLeft(geom.x() - (minsize.width()-geom.size().width())); //reset back to min width
        }
        break;
      case ResizeTopLeft:
        geom.setTopLeft(ev->globalPos()-offset);
        if(geom.size().height() < minsize.height()){
	  geom.setTop(geom.y() - (minsize.height()-geom.size().height())); //reset back to min height
        }
        if(geom.size().width() < minsize.width()){
	  geom.setLeft(geom.x() - (minsize.width()-geom.size().width())); //reset back to min width
        }
        break;
      default:
	break;
    }
    //if( (geom.width()%2==0 && geom.height()%2==0) || activeState==Move){
      //qDebug() << " Change Window:" << this->geometry() << geom;
      if(activeState==Move){ this->setGeometry(geom); }
      else if(this->geometry()!=geom){
        pending_geom = geom;
        if(!resizeTimer->isActive()){ QTimer::singleShot(0,resizeTimer, SLOT(start()) ); }
        /*qDebug() << " Change Window Dimensions:" << this->geometry() << geom;
	 qDebug() << " - Mouse Pos:" << ev->globalPos() << ev->pos() << "Offset" << offset;
	 this->setGeometry(geom);
        qDebug() << " - Done setting geom";*/
      }
    //}
  }
}

void NativeWindow::mouseReleaseEvent(QMouseEvent *ev){
  //Check for a right-click event
  //qDebug() << "Frame Mouse Release Event";
  QFrame::mouseReleaseEvent(ev);
  if( (activeState==Normal) && (titleLabel->geometry().contains(ev->pos())) && (ev->button()==Qt::RightButton) ){
    //container->raiseWindow();//need to ensure the native window is always on top of this frame but under the menu
    otherB->emit clicked();
    return;
  }
  if(activeState!=Normal){
    //if(container->isPaused()){ container->resume(); }
    activeState = Normal;
    QApplication::restoreOverrideCursor();
    setMouseCursor( getStateAtPoint(ev->pos()) );
  }
  if(QFrame::mouseGrabber() == this){ this->releaseMouse(); }
  container->activateWindow();
  //QTimer::singleShot(0, container, SLOT(raiseWindow()) );
}

/*void NativeWindow::enterEvent(QEvent *ev){
  QFrame::enterEvent(ev);
  container->raiseWindow();
}*/
void NativeWindow::leaveEvent(QEvent *ev){
  QFrame::leaveEvent(ev);
  if(activeState == Normal){
    setMouseCursor(Normal);
  }
  //if(!QRect(QPoint(0,0),this->size()).contains( this->mapFromGlobal(QCursor::pos())) ){ container->lowerWindow(); }
}

void NativeWindow::moveEvent(QMoveEvent *ev){
  //qDebug() << "Got Move Event:" << ev->pos() << container->geometry();
  QFrame::moveEvent(ev);
  //if(!closing && !container->isPaused()){
    QTimer::singleShot(0,moveTimer, SLOT(start())); //this needs to be thread-safe
  //}
}
