//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "RootSubWindow.h"
#include <QDebug>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QScreen>

#define WIN_BORDER 5

#include <LIconCache.h>
#include <DesktopSettings.h>

// === PUBLIC ===
RootSubWindow::RootSubWindow(QWidget *root, NativeWindow *win) : QFrame(root){
  this->setAttribute(Qt::WA_DeleteOnClose);
  this->setMouseTracking(true);
  //Create the QWindow and QWidget containers for the window
  WIN = win;
  closing = false;
  initWindowFrame();
  //Hookup the signals/slots
  connect(WIN, SIGNAL(PropertiesChanged(QList<NativeWindow::Property>, QList<QVariant>)), this, SLOT(propertiesChanged(QList<NativeWindow::Property>, QList<QVariant>)));
  WinWidget->embedWindow(WIN);
  //qDebug() << "[NEW WINDOW]" << WIN->id() << WinWidget->winId() << this->winId();
  activeState = RootSubWindow::Normal;
  LoadAllProperties();
}

RootSubWindow::~RootSubWindow(){
  //qDebug() << "Visible Window Destroyed";
  WIN->deleteLater();
}

WId RootSubWindow::id(){
  return WIN->id();
}

NativeWindow* RootSubWindow::nativeWindow(){
  return WIN;
}

// === PRIVATE ===
RootSubWindow::ModState RootSubWindow::getStateAtPoint(QPoint pt, bool setoffset){
  //Note: pt should be in widget-relative coordinates, not global
  if(!WinWidget->geometry().contains(pt) && !titleBar->geometry().contains(pt)){
    //above the frame itself - need to figure out which quadrant it is in (8-directions)
    if(pt.y() < WIN_BORDER){
      //One of the top options
      if(pt.x() < this->width()/5){
	if(setoffset){ offset.setX(pt.x()); offset.setY(pt.y()); } //difference from top-left corner
	return ResizeTopLeft;
      }else if(pt.x() > (this->width()*4.0/5.0)){
	if(setoffset){ offset.setX(pt.x()-this->width()); offset.setY(pt.y()); } //difference from top-right corner
	return ResizeTopRight;
      }else{
	if(setoffset){ offset.setX(0); offset.setY(pt.y()); } //difference from top edge (X does not matter)
	return ResizeTop;
      }
    }else if(pt.y() > (this->height()-WIN_BORDER) ){
      //One of the bottom options
      if(pt.x() < this->width()/5){
	if(setoffset){ offset.setX(pt.x()); offset.setY(pt.y()-this->height()); } //difference from bottom-left corner
	return ResizeBottomLeft;
      }else if(pt.x() > (this->width()*4.0/5.0)){
	if(setoffset){ offset.setX(pt.x()-this->width()); offset.setY(pt.y()-this->height()); } //difference from bottom-right corner
	return ResizeBottomRight;
      }else{
	if(setoffset){ offset.setX(0); offset.setY(pt.y()-this->height()); } //difference from bottom edge (X does not matter)
	return ResizeBottom;
      }
    }else if(pt.x() < WIN_BORDER){
      //Left side options
      if(pt.y() < this->height()/5){
	if(setoffset){ offset.setX(pt.x()); offset.setY(pt.y()); } //difference from top-left corner
	return ResizeTopLeft;
      }else if(pt.y() > (this->height()*4.0/5.0)){
	if(setoffset){ offset.setX(pt.x()); offset.setY(pt.y()-this->height()); } //difference from bottom-left corner
	return ResizeBottomLeft;
      }else{
	if(setoffset){ offset.setX(pt.x()); offset.setY(0); } //difference from left edge (Y does not matter)
	return ResizeLeft;
      }
    }else if(pt.x() > (this->width()-WIN_BORDER) ){
      //Right side options
      if(pt.y() < this->height()/5){
	if(setoffset){ offset.setX(pt.x()-this->width()); offset.setY(pt.y()); } //difference from top-right corner
	return ResizeTopRight;
      }else if(pt.y() > (this->height()*4.0/5.0)){
	if(setoffset){ offset.setX(pt.x()-this->width()); offset.setY(pt.y()-this->height()); } //difference from bottom-right corner
	return ResizeBottomRight;
      }else{
	if(setoffset){ offset.setX(pt.x()-this->width()); offset.setY(0); } //difference from right edge (Y does not matter)
	return ResizeRight;
      }
    }else{
	return Normal;
    }
  }
  //if it gets this far just return normal
  return Normal;
}

void RootSubWindow::setMouseCursor(ModState state, bool override){
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

void RootSubWindow::initWindowFrame(){
  //qDebug() << "Create RootSubWindow Frame";
  this->setContentsMargins(0,0,0,0);
  mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
  titleBar = new QWidget(this);
    titleBar->setContentsMargins(0,0,0,0);
  titleBarL = new QHBoxLayout(titleBar);
    titleBarL->setContentsMargins(0,0,0,0);
 closeB = new QToolButton(this);
  maxB = new QToolButton(this);
  minB = new QToolButton(this);
  otherB = new QToolButton(this);
  anim  = new QPropertyAnimation(this);
    anim->setTargetObject(this);
    anim->setDuration(200); //1/5 second (appx)
  connect(anim, SIGNAL(finished()), this, SLOT(animFinished()) );
  titleLabel = new QLabel(this);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  otherM = new QMenu(this); //menu of other actions
    otherB->setMenu(otherM);
    otherB->setPopupMode(QToolButton::InstantPopup);
    otherB->setAutoRaise(true);
  WinWidget = new NativeEmbedWidget(this);
  connect(closeB, SIGNAL(clicked()), this, SLOT(triggerClose()) );
  connect(maxB, SIGNAL(clicked()), this, SLOT(toggleMaximize()) );
  connect(minB, SIGNAL(clicked()), this, SLOT(toggleMinimize()) );
  //Now assemble the frame layout based on the current settings
    titleBarL->addWidget(otherB);
    titleBarL->addWidget(titleLabel);
    titleBarL->addWidget(minB);
    titleBarL->addWidget(maxB);
    titleBarL->addWidget(closeB);
  WinWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  titleBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  mainLayout->addWidget(titleBar);
  mainLayout->addWidget(WinWidget);
  mainLayout->setAlignment(titleBar, Qt::AlignTop);
  //Setup the cursors for the buttons
  closeB->setCursor(Qt::ArrowCursor);
  minB->setCursor(Qt::ArrowCursor);
  maxB->setCursor(Qt::ArrowCursor);
  otherB->setCursor(Qt::ArrowCursor);
  titleLabel->setCursor(Qt::ArrowCursor);
  WinWidget->setCursor(Qt::ArrowCursor);
  //Now all the stylesheet options
  this->setObjectName("WindowFrame");
    closeB->setObjectName("Button_Close");
    minB->setObjectName("Button_Minimize");
    maxB->setObjectName("Button_Maximize");
    otherM->setObjectName("Menu_Actions");
    titleLabel->setObjectName("Label_Title");
  this->setStyleSheet("QFrame#WindowFrame{background-color: rgba(0,0,0,125)} QWidget#Label_Title{background-color: transparent; color: white; } QToolButton{background-color: transparent; border: 1px solid transparent; border-radius: 3px; } QToolButton::hover{background-color: rgba(255,255,255,150); } QToolButton::pressed{ background-color: white; } QToolButton::menu-arrow{ image: none; }");
  //And adjust the margins
  mainLayout->setSpacing(0);
  titleBarL->setSpacing(1);
  this->setFrameStyle(QFrame::NoFrame);
  this->setLineWidth(0);
  this->setMidLineWidth(0);
  this->setFrameRect(QRect(0,0,0,0));

  //Setup the timer object to syncronize info
  moveTimer = new QTimer(this);
  moveTimer->setSingleShot(true);
  moveTimer->setInterval(100); //1/10 second
  connect(moveTimer, SIGNAL(timeout()), WinWidget, SLOT(resyncWindow()) );

  //Now load the icons for the button
  LIconCache::instance()->loadIcon(closeB, "window-close");
  LIconCache::instance()->loadIcon(maxB, "window-maximize");
  LIconCache::instance()->loadIcon(minB, "window-minimize");
  LIconCache::instance()->loadIcon(otherB, "list");
}

void RootSubWindow::enableFrame(bool on){
  //Make the individual frame elements visible as needed
  if(on){ this->setContentsMargins(WIN_BORDER,WIN_BORDER,WIN_BORDER,WIN_BORDER); }//default border
  else{ this->setContentsMargins(0, 0, 0, 0); }
  titleBar->setVisible(on);
  //And now calculate/save the frame extents
  QList<int> extents; extents << 0 << 0 << 0 << 0; //left, right, top, bottom
  if(on){
    extents[0] = WIN_BORDER;
    extents[1] = WIN_BORDER;
    extents[2] = WIN_BORDER + titleBar->height();
    extents[3] = WIN_BORDER;
  }
  //qDebug() << "SET FRAME EXTENTS:" << extents;
  WIN->requestProperty(NativeWindow::FrameExtents, QVariant::fromValue< QList<int> >(extents) ); //save on raw window itself
  WIN->setProperty(NativeWindow::FrameExtents, QVariant::fromValue< QList<int> >(extents) ); //save to structure now
}

void RootSubWindow::enableFrame(QList<NativeWindow::Type> types){
  static QList<NativeWindow::Type> noframe;
  if(noframe.isEmpty()){ noframe << NativeWindow::T_DESKTOP << NativeWindow::T_DOCK << NativeWindow::T_TOOLBAR << NativeWindow::T_MENU << NativeWindow::T_SPLASH << NativeWindow::T_DROPDOWN_MENU << NativeWindow::T_POPUP_MENU << NativeWindow::T_TOOLTIP << NativeWindow::T_NOTIFICATION << NativeWindow::T_COMBO << NativeWindow::T_DND; }
  for(int i=0; i<types.length(); i++){
    if(noframe.contains(types[i])){ enableFrame(false); return; }
  }
  enableFrame(true);
  //Now make buttons visible as appropriate for the type
  //NativeWindow::T_UTILITY, NativeWindow::T_DIALOG, , NativeWindow::T_NORMAL
}
void RootSubWindow::LoadProperties( QList< NativeWindow::Property> list){
  QList<QVariant> vals;
  //Always ensure that visibility changes are evaluated last
  bool addvisible = false;
  for(int i=0; i<list.length(); i++){
    if(list[i] == NativeWindow::Visible){ list.removeAt(i); i--; addvisible = true; continue; }
    vals << WIN->property(list[i]);
  }
  //if(addvisible){ list << NativeWindow::Visible; vals << WIN->property(NativeWindow::Visible); }
  propertiesChanged(list, vals);
}

QRect RootSubWindow::clientGlobalGeom(){
  QRect tot = this->geometry();
  QList<int> frame = WIN->property(NativeWindow::FrameExtents).value< QList<int> >();
  //Now adjust this to take out the frame
    tot.adjust(frame[0], frame[2], -frame[1], -frame[3]);
  return tot;
}

// === PUBLIC SLOTS ===
void RootSubWindow::clientClosed(){
  //qDebug() << "Client Closed";
  closing = true;
  if(anim->state()!=QAbstractAnimation::Running){ this->close(); }
}

void RootSubWindow::LoadAllProperties(){
  QList< NativeWindow::Property> list;
  list << NativeWindow::WinTypes << NativeWindow::WinActions << NativeWindow::States
	<< NativeWindow::MinSize << NativeWindow::MaxSize << NativeWindow::Title << NativeWindow::ShortTitle
	<< NativeWindow::Icon << NativeWindow::Size << NativeWindow::GlobalPos;// << NativeWindow::Visible << NativeWindow::Active;
  LoadProperties(list);
  //WIN->requestProperty(NativeWindow::Visible, true);
}

//Button Actions - public so they can be tied to key shortcuts and stuff as well
void RootSubWindow::toggleMinimize(){
  WIN->toggleVisibility();
}

void RootSubWindow::toggleMaximize(){
  //Get the current screen that this window is on
  QList<QScreen*> screens = QApplication::screens();
  QRect rect;
  int primaryscreen = 0; //fallback value
  for(int i=0; i<screens.length(); i++){
    QRect intersect = screens[i]->geometry().intersected(this->geometry());
    if( (intersect.width()-rect.width() + intersect.height()-rect.height()) > 0){
      rect = intersect;
      primaryscreen = i;
    }
  }
  //Now that we have the screen dimensions, lets check/change the window
  rect = screens[primaryscreen]->availableGeometry();
  QList< NativeWindow::State > states = WIN->property(NativeWindow::States).value< QList< NativeWindow::State> >();
  if(rect == this->geometry() || states.contains(NativeWindow::S_MAX_VERT) || states.contains(NativeWindow::S_MAX_HORZ)){
    //Already maximized - try to restore it to the previous size/location
    if(!lastMaxGeom.isNull()){
      rect = lastMaxGeom;
    }else{
      // no last geometry - started out maximized?
      // make it half the screen size and centered on the screen
      QPoint center = rect.center();
      rect.setWidth( rect.width()/2 );
      rect.setHeight( rect.height()/2 );
      rect.moveTopLeft( center - QPoint(rect.width()/2, rect.height()/2) );
    }
    lastMaxGeom = QRect(); //clear this saved geom
  }else{
    //Not maximized yet - go ahead and make it so
    lastMaxGeom = this->geometry(); //save this for later;
  }
  //qDebug() << "Toggle Maximize:" << this->geometry() << rect;
  QString anim_type = DesktopSettings::instance()->value(DesktopSettings::Animation, "window/move", "random").toString();
  loadAnimation(anim_type, NativeWindow::Size, rect);
}

void RootSubWindow::triggerClose(){
  WIN->requestClose();
}

void RootSubWindow::toggleSticky(){
  QList< NativeWindow::State> states = WIN->property(NativeWindow::States).value< QList< NativeWindow::State > >();
  if(states.contains(NativeWindow::S_STICKY)){
    states.removeAll(NativeWindow::S_STICKY);
  }else{
    states << NativeWindow::S_STICKY;
  }
  WIN->requestProperty(NativeWindow::States, QVariant::fromValue<QList <NativeWindow::State> >(states) );
}

void RootSubWindow::activate(){
  //WinWidget->raiseWindow();
  WIN->requestProperty(NativeWindow::Active, true, true);
}

//Mouse Interactivity
void RootSubWindow::startMoving(){
  //If the cursor is not over this window, move it to the center of the titlebar
  QPoint curpt = QCursor::pos(); //global coords
  if(!this->geometry().contains(curpt)){
    curpt = this->mapToGlobal(titleBar->geometry().center());
    QCursor::setPos(curpt);
  }
  //Calculate the offset
  activeState = Move;
  offset = this->mapFromGlobal(curpt);
  setMouseCursor(activeState, true); //this one is an override cursor
  WinWidget->pause();
  this->grabMouse();
}

void RootSubWindow::startResizing(){
  activeState = getStateAtPoint( this->mapFromGlobal(QCursor::pos()), true); //also have it set the offset variable
  setMouseCursor(activeState, true); //this one is an override cursor
  WinWidget->pause();
  this->grabMouse();
}

// === PRIVATE SLOTS ===
void RootSubWindow::propertiesChanged(QList<NativeWindow::Property> props, QList<QVariant> vals){
  for(int i=0; i<props.length() && i<vals.length(); i++){
    if(vals[i].isNull()){ continue; } //not the same as a default/empty value - the property has just not been set yet
    //qDebug() << "RootSubWindow: Property Changed:" << props[i] << vals[i];
    switch(props[i]){
	case NativeWindow::Visible:
                if(!WinWidget->isPaused() && (this->isVisible()!=vals[i].toBool()) && activeState==Normal ){
		  //qDebug() << "Got Visibility Change:" << vals[i] << this->geometry() << WIN->geometry();
		  if(vals[i].toBool()){ loadAnimation( DesktopSettings::instance()->value(DesktopSettings::Animation, "window/appear", "random").toString(), NativeWindow::Visible, vals[i]); }
		  else{ loadAnimation( DesktopSettings::instance()->value(DesktopSettings::Animation, "window/disappear", "random").toString(), NativeWindow::Visible, vals[i]); }
		}
		break;
	case NativeWindow::Title:
		titleLabel->setText(vals[i].toString());
		break;
	case NativeWindow::Icon:
		//qDebug() << "Got Icon Change:" << vals[i];
		if(vals[i].value<QIcon>().isNull() ){ LIconCache::instance()->loadIcon(otherB, "list"); }
		else{ otherB->setIcon(vals[i].value<QIcon>()); }
		break;
	case NativeWindow::GlobalPos:
		if(vals[i].toPoint()!=QPoint(0,0)){
		  WinWidget->resyncWindow();
		}
		break;
	case NativeWindow::Size:
		//qDebug() << " - SIZE CHANGE";
		if(WIN->property(NativeWindow::FrameExtents).isNull() && (i<props.indexOf(NativeWindow::FrameExtents)) ){
		  //Frame not loaded yet - push this back until after the frame is set
		  props << props.takeAt(i);
		  vals << vals.takeAt(i);
		  i--;
		}else if(!WinWidget->isPaused() && activeState==Normal){
		  if(WIN->property(NativeWindow::Size).toSize() != WinWidget->size()){
                    //qDebug() << "Got Direct Geometry Change:" << WIN->geometry();
		    this->setGeometry( QRect(this->geometry().topLeft(), WIN->geometry().size()) );
		    WinWidget->resyncWindow();
		  }
		}
		break;
	case NativeWindow::MinSize:
		if(vals[i].toSize().isValid()){
		  //Just larger than titlebar, with enough space for 8 characters in the titlebar (+4 buttons)
		  //qDebug() << "Got invalid Min Size: Set a reasonable default minimum";
		  WinWidget->setMinimumSize( QSize( this->fontMetrics().height()*4 + this->fontMetrics().width("O")*10, this->fontMetrics().height()*10) );
		  WIN->setProperty(NativeWindow::MinSize, WinWidget->minimumSize());
		}else{
		  WinWidget->setMinimumSize(vals[i].toSize());
		}
		if(WIN->property(NativeWindow::Size).toSize().width() < WinWidget->minimumSize().width() \
		     || WIN->property(NativeWindow::Size).toSize().height() < WinWidget->minimumSize().height()  ){
		  WIN->setProperty(NativeWindow::Size, WinWidget->minimumSize(), true); //force this
		  //WinWidget->resize(WinWidget->minimumSize());
		}
		break;
	case NativeWindow::MaxSize:
		WinWidget->setMaximumSize(vals[i].toSize());
		break;
	case NativeWindow::Active:
		if(vals[i].toBool()){ activate(); } //WinWidget->raiseWindow(); }
		break;
	/*case NativeWindow::FrameExtents:
		qDebug() << " - FRAME CHANGE";
		if(vals[i].isNull()){
		  vals[i] = QVariant::fromValue<QList<int> >( QList<int>() << WinWidget->geometry().x() << this->width()-WinWidget->geometry().x()-WinWidget->geometry().width() << WinWidget->y() << this->height() - WinWidget->y() - WinWidget->geometry().height() );
		  WIN->setProperty(NativeWindow::FrameExtents, vals[i]);
		}
		qDebug() << "Setting Frame Extents:" << vals[i].value<QList<int> >();
		mainLayout->setContentsMargins( vals[i].value< QList<int> >().at(0),vals[i].value< QList<int> >().at(2) - titleLabel->height(),vals[i].value< QList<int> >().at(1),vals[i].value< QList<int> >().at(3));
		break;*/
	case NativeWindow::WinTypes:
		//qDebug() << "Got Window Types:" << vals[i].value< QList<NativeWindow::Type> >();
		enableFrame(vals[i].value< QList<NativeWindow::Type> >() );
		break;
	default:
		qDebug() << "Window Property Unused:" << props[i] << vals[i];
    }
  }
}

// === PROTECTED ===
void RootSubWindow::mousePressEvent(QMouseEvent *ev){
  activate();
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

void RootSubWindow::mouseMoveEvent(QMouseEvent *ev){
  QFrame::mouseMoveEvent(ev);
  if(activeState == Normal){
    setMouseCursor( getStateAtPoint(ev->pos()) ); //just update the mouse cursor
  }else{
    //Currently in a modification state
    QRect geom = this->geometry();
    QSize minsize(WinWidget->minimumSize().width() + (2*WIN_BORDER), WinWidget->minimumSize().height()+(2*WIN_BORDER)+titleBar->geometry().size().height());
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
      else{
        //qDebug() << " Change Window Dimensions:" << this->geometry() << geom;
	//qDebug() << " - Mouse Pos:" << ev->globalPos() << ev->pos() << "Offset" << offset;
	this->setGeometry(geom);
      }
    //}
  }
}

void RootSubWindow::mouseReleaseEvent(QMouseEvent *ev){
  //Check for a right-click event
  //qDebug() << "Frame Mouse Release Event";
  QFrame::mouseReleaseEvent(ev);
  if( (activeState==Normal) && (titleBar->geometry().contains(ev->pos())) && (ev->button()==Qt::RightButton) ){
    //WinWidget->raiseWindow();//need to ensure the native window is always on top of this frame but under the menu
    otherM->popup(ev->globalPos());
    return;
  }
  if(activeState!=Normal){
    if(WinWidget->isPaused()){ WinWidget->resume(); }
    activeState = Normal;
    QApplication::restoreOverrideCursor();
    setMouseCursor( getStateAtPoint(ev->pos()) );
  }
  if(QFrame::mouseGrabber() == this){ this->releaseMouse(); }
  activate();
  //QTimer::singleShot(0, WinWidget, SLOT(raiseWindow()) );
}

/*void RootSubWindow::enterEvent(QEvent *ev){
  QFrame::enterEvent(ev);
  WinWidget->raiseWindow();
}*/
/*void RootSubWindow::leaveEvent(QEvent *ev){
  QFrame::leaveEvent(ev);
  if(activeState == Normal){
    setMouseCursor(Normal);
  }
  if(!QRect(QPoint(0,0),this->size()).contains( this->mapFromGlobal(QCursor::pos())) ){ WinWidget->lowerWindow(); }
}*/

void RootSubWindow::moveEvent(QMoveEvent *ev){
  //qDebug() << "Got Move Event:" << ev->pos() << WinWidget->geometry();
  QFrame::moveEvent(ev);
  if(!closing && !WinWidget->isPaused()){
    moveTimer->start();
  }
}
