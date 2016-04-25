//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LWindow.h"

LWindowFrame::LWindowFrame(WId client, QWidget *parent) : QFrame(parent, Qt::X11BypassWindowManagerHint){
  activeState = LWindowFrame::Normal;
  CID = client;
  lastAction = LWM::WA_NONE;
  Closing = false;
  //qDebug() << "New Window:" << CID << "Frame:" << this->winId();
  this->setMouseTracking(true); //need this to determine mouse location when not clicked
  this->setObjectName("LWindowFrame");
  this->setStyleSheet("LWindowFrame#LWindowFrame{ border: 2px solid white; border-radius:3px; } QWidget#TitleBar{background: grey; } QLabel{ color: black; }");
  InitWindow(); //initially create all the child widgets
  //LWM::SYSTEM->setupEventsForFrame(this->winId());
  updateAppearance(); //this loads the appearance based on window/theme settings
  //QApplication::processEvents();
  //Now set the frame size on this window
  SyncSize();
  SyncText();	
  this->show();
}

LWindowFrame::~LWindowFrame(){
}

// =================
//        PRIVATE
// =================
void LWindowFrame::InitWindow(){
	anim = new QPropertyAnimation(this); //For simple window animations
	  anim->setTargetObject(this);
	  anim->setDuration(ANIMTIME); //In milliseconds
	  connect(anim, SIGNAL(finished()), this, SLOT(finishedAnimation()) );
	titleBar = new QLabel(this); //This is the "container" for all the title buttons/widgets
	  titleBar->setObjectName("TitleBar");
	  titleBar->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
	  titleBar->setFocusPolicy(Qt::NoFocus);
	  titleBar->setCursor(Qt::ArrowCursor);
	title = new QLabel(this); //Shows the window title/text
	  title->setObjectName("Title");
	  title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	  title->setCursor(Qt::ArrowCursor);
	  title->setFocusPolicy(Qt::NoFocus);
	icon = new QLabel(this); //Contains the window icon
	  icon->setObjectName("Icon");
	  icon->setCursor(Qt::ArrowCursor);
	  icon->setFocusPolicy(Qt::NoFocus);
	minB = new QToolButton(this); //Minimize Button
	  minB->setObjectName("Minimize");
	  minB->setCursor(Qt::ArrowCursor);
	  minB->setFocusPolicy(Qt::NoFocus);
	  connect(minB, SIGNAL(clicked()), this, SLOT(minClicked()) );
	maxB = new QToolButton(this); //Maximize Button
	  maxB->setObjectName("Maximize");
	  maxB->setCursor(Qt::ArrowCursor);
	  maxB->setFocusPolicy(Qt::NoFocus);
	  connect(maxB, SIGNAL(clicked()), this, SLOT(maxClicked()) );
	closeB = new QToolButton(this);
	  closeB->setObjectName("Close");
	  closeB->setCursor(Qt::ArrowCursor);
	  closeB->setFocusPolicy(Qt::NoFocus);
	  connect(closeB, SIGNAL(clicked()), this, SLOT(closeClicked()) );
	otherB = new QToolButton(this); //Button to place any other actions
	  otherB->setObjectName("Options");
	  otherB->setCursor(Qt::ArrowCursor);
	  otherB->setPopupMode(QToolButton::InstantPopup);
	  otherB->setStyleSheet("QToolButton::menu-indicator{ image: none; }");
	  otherB->setFocusPolicy(Qt::NoFocus);
	otherM = new QMenu(this); //menu of "other" actions for the window
	  otherB->setMenu(otherM);
	  connect(otherM, SIGNAL(triggered(QAction*)), this, SLOT(otherClicked(QAction*)) );
	//Now assemble the titlebar
	QHBoxLayout *HL = new QHBoxLayout(this);
	HL->setContentsMargins(0,0,0,0);
	HL->addWidget(otherB);
	HL->addWidget(icon);
	HL->addWidget(title);
	HL->addWidget(minB);
	HL->addWidget(maxB);
	HL->addWidget(closeB);
	titleBar->setLayout(HL);
	QVBoxLayout *VL = new QVBoxLayout(this);
	this->setLayout(VL);
	//The WinWidget container appears shifted right/down by 1 pixel for some reason
	// Adjust the margins to account for this variation
	VL->setContentsMargins(1,1,2,2); 
	VL->setSpacing(0);
	//Have the window take the same initial size of the client window
        QRect geom = LWM::SYSTEM->WM_Window_Geom(CID);
	qDebug() << " - Load Size Hints" << "initial size:" << geom.size();
	icccm_size_hints SH = LWM::SYSTEM->WM_ICCCM_GetNormalHints(CID);
	qDebug() << " - - Got Normal Hints";
	if(!SH.isValid()){ SH = LWM::SYSTEM->WM_ICCCM_GetSizeHints(CID); }
	qDebug() << " - - Start resizing...";
	if(SH.base_width>geom.width() && SH.base_height>geom.height()){ this->resize(SH.base_width, SH.base_height); }
	else if(SH.min_width>geom.width() && SH.min_height>geom.height()){ this->resize(SH.min_width, SH.min_height); }
	else if(SH.width>geom.width() && SH.height>geom.height()){ this->resize(SH.width, SH.height); }
	else if(geom.isNull()){ this->resize(100,80); }
        else{ this->resize( geom.size() ); }
	qDebug() << " - done";
	
	//Now embed the native window into the frame
	WIN = QWindow::fromWinId(CID);
	WinWidget = QWidget::createWindowContainer( WIN, this);
	  WinWidget->setCursor(Qt::ArrowCursor); //this is just a fallback - the window itself will adjust it
	//WINBACK = new QBackingStore(WIN); //create a data backup for the widget
	
	//Now assemble te initial layout for the window (all while still invisible)
	/*VL->addWidget(titleBar);
	VL->addWidget(WinWidget);
        VL->setStretch(1,1);*/
}

LWindowFrame::ModState LWindowFrame::getStateAtPoint(QPoint pt, bool setoffset){
  //Note: pt should be in widget-relative coordinates, not global
  if(!this->layout()->geometry().contains(pt)){
    //above the frame itself - need to figure out which quadrant it is in (8-directions)
    if(pt.y() < 3){
      //One of the top options
      if(pt.x() < 3){ 
	if(setoffset){ offset.setX(pt.x()); offset.setY(pt.y()); } //difference from top-left corner
	return ResizeTopLeft;
      }else if(pt.x() > (this->width()-3)){ 
	if(setoffset){ offset.setX(this->width()-pt.x()); offset.setY(pt.y()); } //difference from top-right corner
	return ResizeTopRight;
      }else{ 
	if(setoffset){ offset.setX(0); offset.setY(pt.y()); } //difference from top edge (X does not matter)
	return ResizeTop; 
      }		    
    }else if(pt.y() > (this->height()-3) ){
      //One of the bottom options
      if(pt.x() < 3){ 
	if(setoffset){ offset.setX(pt.x()); offset.setY(this->height()-pt.y()); } //difference from bottom-left corner
	return ResizeBottomLeft;
      }else if(pt.x() > (this->width()-3)){ 
	if(setoffset){ offset.setX(this->width()-pt.x()); offset.setY(this->height()-pt.y()); } //difference from bottom-right corner
	return ResizeBottomRight;
      }else{ 
	if(setoffset){ offset.setX(0); offset.setY(this->height() - pt.y()); } //difference from bottom edge (X does not matter)
	return ResizeBottom; 
      }	
    }else{
      //One of the side options
      if(pt.x() < 3){ 
	if(setoffset){ offset.setX(pt.x()); offset.setY(0); } //difference from left edge (Y does not matter)
	return ResizeLeft;
      }else if(pt.x() > (this->width()-3) ){ 
	if(setoffset){ offset.setX(this->width()-pt.x()); offset.setY(0); } //difference from right edge (Y does not matter)
	return ResizeRight;
      }else{
	return Normal;
      }
    }
  }
  return Normal;
}

void LWindowFrame::setMouseCursor(ModState state, bool override){
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
    this->setCursor(shape);
  }
}

// ==========================
//   WINDOW INTERACTIONS
//==========================
void LWindowFrame::SyncSize(bool fromwin){ 
  //sync the window/frame geometries (generally only done before embedding the client window)
    int frame =  this->frameWidth();
    int TH = titleBar->height();
    //Now load the information about the window and adjust the frame to match
    if(fromwin){ lastGeom = LWM::SYSTEM->WM_Window_Geom(CID); }
    else{ lastGeom = this->geometry(); }
    qDebug() << "Initial Size:" << lastGeom << frame << TH;
    //Add in the frame size
    lastGeom.moveTop(lastGeom.y()-frame-TH);
    lastGeom.setHeight(lastGeom.height()+(2*frame)+TH);
    lastGeom.moveLeft(lastGeom.x()-frame);
    lastGeom.setWidth( lastGeom.width()+(2*frame));
    QList<unsigned int> margins; 
      margins << frame << frame << frame+TH << frame; //L/R/Top/Bottom
    qDebug() << " - With Frame:" << lastGeom;
    //Now adjust for a out-of-bounds location
    if(lastGeom.x() < 0){ lastGeom.moveLeft(0); }
    if(lastGeom.y() < 0){ lastGeom.moveTop(0); }
    qDebug() << " - Adjusted:" << lastGeom;
    this->setGeometry(lastGeom);
    LWM::SYSTEM->WM_Set_Frame_Extents(CID, margins);
}

void LWindowFrame::SyncText(){
  QString txt = WIN->title();
  if(txt.isEmpty()){ txt = LWM::SYSTEM->WindowName(CID); }
  if(txt.isEmpty()){ txt = LWM::SYSTEM->OldWindowName(CID); }
  if(txt.isEmpty()){ txt = LWM::SYSTEM->WindowVisibleName(CID); }
  if(txt.isEmpty()){ txt = LWM::SYSTEM->WindowIconName(CID); }
  if(txt.isEmpty()){ txt = LWM::SYSTEM->WindowVisibleIconName(CID); }
  if(txt.isEmpty()){ txt = LWM::SYSTEM->WM_ICCCM_GetClass(CID); }
  title->setText(txt);
}

//  SIMPLE ANIMATIONS
void LWindowFrame::showAnimation(LWM::WindowAction act){
  bool useanimation = (act!=lastAction);
  if(anim->state()==QAbstractAnimation::Running){ 
    qDebug() << "New Animation Event:" << act; 
    return; 
  }
  //Setup the animation routine
  if(act==LWM::Show){
    if(useanimation){
      lastGeom = this->geometry();
      //Expand out from center point
      anim->setPropertyName("geometry");
      anim->setStartValue( QRect(lastGeom.center(), QSize(0,0) ) );
      anim->setEndValue( this->geometry() );
      //Fade in gradually
      //anim->setPropertyName("windowOpacity");
      //anim->setStartValue( 0.0 );
      //anim->setEndValue( 1.0 );
    }else{
      ShowClient(true);
      this->raise();
      this->show(); //just show it right away
    }
    
  }else if(act==LWM::Hide){
    if(useanimation){
       //Collapse in on center point
      lastGeom = this->geometry();
      anim->setPropertyName("geometry");
      anim->setStartValue( QRect(this->geometry()) );
      anim->setEndValue( QRect(this->geometry().center(), QSize(0,0) ) );
    }else{
      this->hide(); //just hide it right away
    }
  }else if(act==LWM::Closed){
    //Need to clean up the container widget first to prevent XCB errors
    //qDebug() << "Window Closed:" << WIN->winId() << CID;
    if(useanimation){
       //Collapse in on center line
      lastGeom = this->geometry();
      anim->setPropertyName("geometry");
      anim->setStartValue( QRect(this->geometry()) );
      anim->setEndValue( QRect(this->geometry().x(), this->geometry().center().y(), this->width(), 0 ) );
    }else{
      CloseAll(); //just hide it right away
    }
  }
  if(useanimation){ 
    ShowClient(false);
    this->show();
    qDebug() << " - Starting Animation:" << act;
    lastAction = act;
    anim->start(); 
  };
}

void LWindowFrame::ShowClient(bool show){
  if(show && this->layout()->indexOf(WinWidget)<0 && !Closing){
    while(this->layout()->count()>0){ this->layout()->removeItem(0); }
    this->layout()->addWidget(titleBar);
    this->layout()->setAlignment(titleBar, Qt::AlignTop);
    this->layout()->addWidget(WinWidget);
    static_cast<QVBoxLayout*>(this->layout())->setStretch(1,1);
    LWM::SYSTEM->WM_ShowWindow(CID);
  }else if( !show && this->layout()->indexOf(WinWidget)>=0){
    LWM::SYSTEM->WM_HideWindow(CID);
    this->layout()->removeWidget(WinWidget);
  }
}

void LWindowFrame::finishedAnimation(){
  //Also set any final values
  qDebug() << " - Finished Animation:" << lastAction;
  switch(lastAction){
    case LWM::Show:
	ShowClient(true);
        break;
    case LWM::Closed:
    case LWM::Hide:
	this->lower();
	this->hide();
	LWM::SYSTEM->WM_HideWindow(this->winId());
    default:
	break;
  }
  if(Closing){
    qDebug() << "Emitting finished signal";
    emit Finished();
  }
}

// =================
//    PUBLIC SLOTS
// =================
void LWindowFrame::updateAppearance(){
  //Reload any button icons and such
  minB->setIcon(LXDG::findIcon("window-suppressed",""));
  maxB->setIcon(LXDG::findIcon("view-fullscreen",""));
  closeB->setIcon(LXDG::findIcon("application-exit",""));
  otherB->setIcon(LXDG::findIcon("configure",""));
}

void LWindowFrame::windowChanged(LWM::WindowAction act){
  //A window property was changed - update accordingly
  switch(act){
    case LWM::Closed:
	Closing = true;
    case LWM::Hide:
    case LWM::Show:
	showAnimation(act);
	break;
    case LWM::MoveResize:
	//Re-adjust to the new position/size of the window
	SyncSize(true);
	break;
    default:
	break; //do nothing
  }
}
// =================
//    PRIVATE SLOTS
// =================
void LWindowFrame::closeClicked(){
  qDebug() << "Closing Window" << LWM::SYSTEM->WM_ICCCM_GetClass(CID);
  //First try the close event to let the client app do cleanup/etc
  LWM::SYSTEM->WM_CloseWindow(CID);
}

void LWindowFrame::minClicked(){
  qDebug() << "Minimize Window";
  windowChanged(LWM::Hide);
}

void LWindowFrame::maxClicked(){
  if(normalGeom.isNull()){
    qDebug() << "Maximize Window";
    normalGeom = this->geometry(); //save for later
    this->showMaximized();
  }else{
    qDebug() << "Restore Window";
    this->showNormal();
    this->setGeometry(normalGeom);
    normalGeom = QRect(); //clear it
  }
}

void LWindowFrame::otherClicked(QAction* act){
  QString action = act->whatsThis();
}

void LWindowFrame::CloseAll(){
  qDebug() << " - Closing Frame";
  this->hide();
  emit Finished();
}
// =====================
//         PROTECTED
// =====================
void LWindowFrame::mousePressEvent(QMouseEvent *ev){
  qDebug() << "Frame Mouse Press Event";
  offset.setX(0); offset.setY(0);
  if(activeState != Normal){ return; } // do nothing - already in a state of grabbed mouse
  this->activateWindow();
  LWM::SYSTEM->WM_Set_Active_Window(CID);
  if(this->childAt(ev->pos())!=0){
    //Check for any non-left-click event and skip it
    if(ev->button()!=Qt::LeftButton){ return; }
    activeState = Move;
    offset.setX(ev->pos().x()); offset.setY(ev->pos().y());
  }else{
    //Clicked on the frame somewhere
    activeState = getStateAtPoint(ev->pos(), true); //also have it set the offset variable
  }
  setMouseCursor(activeState, true); //this one is an override cursor
  
}

void LWindowFrame::mouseMoveEvent(QMouseEvent *ev){
  ev->accept();
  if(activeState == Normal){
    setMouseCursor( getStateAtPoint(ev->pos()) ); //just update the mouse cursor

  }else{
    //Currently in a modification state
    QRect geom = this->geometry();
    switch(activeState){
      case Move:
        geom.moveTopLeft(ev->globalPos()-offset); //will not change size
        break;
      case ResizeTop:
        geom.setTop(ev->globalPos().y()-offset.y());
        break;
      case ResizeTopRight:
        geom.setTopRight(ev->globalPos()-offset);
        break;
      case ResizeRight:
        geom.setRight(ev->globalPos().x()-offset.x());
        break;
      case ResizeBottomRight:
        geom.setBottomRight(ev->globalPos()-offset);
        break;
      case ResizeBottom:
        geom.setBottom(ev->globalPos().y()-offset.y());
        break;
      case ResizeBottomLeft:
        geom.setBottomLeft(ev->globalPos()-offset);
        break;
      case ResizeLeft:
        geom.setLeft(ev->globalPos().x()-offset.x());
        break;
      case ResizeTopLeft:
        geom.setTopLeft(ev->globalPos()-offset);
        break;	 
      default:
	break;
    }
    this->setGeometry(geom);
  }
}

void LWindowFrame::mouseReleaseEvent(QMouseEvent *ev){
  //Check for a right-click event
  qDebug() << "Frame Mouse Release Event";
  ev->accept();
  if( (activeState==Normal) && (this->childAt(ev->pos())==titleBar) && (ev->button()==Qt::RightButton) ){
    otherM->popup(ev->globalPos());
    return;
  }
  activeState = Normal;
  QApplication::restoreOverrideCursor();
  setMouseCursor( getStateAtPoint(ev->pos()) );
}
