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
#include <QVBoxLayout>

// === PUBLIC ===
RootSubWindow::RootSubWindow(QWidget *root, NativeWindow *win) : QFrame(root){
  this->setAttribute(Qt::WA_DeleteOnClose);
  this->setMouseTracking(true);
  //Create the QWindow and QWidget containers for the window
  WIN = win;
  closing = false;
  WinWidget = QWidget::createWindowContainer( WIN->window(), this);
  initWindowFrame();
  LoadProperties( QList< NativeWindow::Property>()  << NativeWindow::Title << NativeWindow::Icon  \
			<< NativeWindow::MinSize << NativeWindow::MaxSize << NativeWindow::Size );
  //Hookup the signals/slots
  connect(this, SIGNAL(aboutToActivate()), this, SLOT(aboutToActivate()) );
  connect(WIN, SIGNAL(PropertyChanged(NativeWindow::Property, QVariant)), this, SLOT(propertyChanged(NativeWindow::Property, QVariant)));
  //Make sure the visibily property only gets loaded after it is added to the root area
  propertyChanged(NativeWindow::Visible, WIN->property(NativeWindow::Visible));
}

RootSubWindow::~RootSubWindow(){

}

WId RootSubWindow::id(){
  return WIN->id();
}

// === PRIVATE ===
RootSubWindow::ModState RootSubWindow::getStateAtPoint(QPoint pt, bool setoffset){
  //Note: pt should be in widget-relative coordinates, not global
  if(!WinWidget->geometry().contains(pt)){
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

void RootSubWindow::setMouseCursor(ModState state, bool override){
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

void RootSubWindow::initWindowFrame(){
  mainLayout = new QVBoxLayout(this);
  titleBar = new QHBoxLayout(this);
 closeB = new QToolButton(this);
  maxB = new QToolButton(this);
  minB = new QToolButton(this);
  titleLabel = new QLabel(this);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  otherM = new QMenu(this); //menu of other actions
  connect(closeB, SIGNAL(clicked()), this, SLOT(triggerClose()) );
  connect(maxB, SIGNAL(clicked()), this, SLOT(toggleMaximize()) );
  connect(minB, SIGNAL(clicked()), this, SLOT(toggleMinimize()) );
  //Now assemble the frame layout based on the current settings
  this->setLayout(mainLayout);
    titleBar->addWidget(titleLabel);
    titleBar->addWidget(minB);
    titleBar->addWidget(maxB);
    titleBar->addWidget(closeB);
  WinWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  mainLayout->addLayout(titleBar);
  mainLayout->addWidget(WinWidget);
  //Now all the stylesheet options
  this->setObjectName("WindowFrame");
    closeB->setObjectName("Button_Close");
    minB->setObjectName("Button_Minimize");
    maxB->setObjectName("Button_Maximize");
    otherM->setObjectName("Menu_Actions");
    titleLabel->setObjectName("Label_Title");
  this->setStyleSheet("QWidget#WindowFrame{background-color: black;} QWidget#Label_Title{background-color: darkgrey; color: white; }");
  //And adjust the margins
  mainLayout->setContentsMargins(4,4,4,4);
  mainLayout->setSpacing(0);
  titleBar->setSpacing(1);
  titleBar->setContentsMargins(0,0,0,0);
}

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

//Button Actions - public so they can be tied to key shortcuts and stuff as well
void RootSubWindow::toggleMinimize(){

}

void RootSubWindow::toggleMaximize(){

}

void RootSubWindow::triggerClose(){
  WIN->emit RequestClose(WIN->id());
}

void RootSubWindow::toggleSticky(){

}

void RootSubWindow::activate(){
  WIN->emit RequestActivate(WIN->id());
}

//Mouse Interactivity
void RootSubWindow::startMoving(){

}

void RootSubWindow::startResizing(){

}



// === PRIVATE SLOTS ===
void RootSubWindow::aboutToActivate(){
  
}

void RootSubWindow::propertyChanged(NativeWindow::Property prop, QVariant val){
  if(val.isNull()){ return; } //not the same as a default/empty value - the property has just not been set yet
  qDebug() << "Set Window Property:" << prop << val;
  switch(prop){
	case NativeWindow::Visible:
		if(val.toBool()){ this->show(); }
		else{ this->hide(); }
		break;
	case NativeWindow::Title:
		titleLabel->setText(val.toString());
		break;
	case NativeWindow::Icon:
		//this->setWindowIcon(val.value< QIcon>());
		break;
	case NativeWindow::Size:
		WinWidget->resize(val.toSize());
		break;
	case NativeWindow::MinSize:
		WinWidget->setMinimumSize(val.toSize());
		break;
	case NativeWindow::MaxSize:
		WinWidget->setMaximumSize(val.toSize());
		break;
	case NativeWindow::Active:
		WinWidget->setFocus();
		break;
	/*case NativeWindow::WindowFlags:
		this->setWindowFlags( val.value< Qt::WindowFlags >() );
		break;*/
	default:
		qDebug() << "Window Property Unused:" << prop << val;
  }
}

// === PROTECTED ===
void RootSubWindow::mousePressEvent(QMouseEvent *ev){
  qDebug() << "Frame Mouse Press Event";
  offset.setX(0); offset.setY(0);
  if(activeState != Normal){ return; } // do nothing - already in a state of grabbed mouse
  this->activate();
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

void RootSubWindow::mouseMoveEvent(QMouseEvent *ev){
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

void RootSubWindow::mouseReleaseEvent(QMouseEvent *ev){
  //Check for a right-click event
  qDebug() << "Frame Mouse Release Event";
  ev->accept();
  if( (activeState==Normal) && (titleBar->geometry().contains(ev->pos())) && (ev->button()==Qt::RightButton) ){
    otherM->popup(ev->globalPos());
    return;
  }
  activeState = Normal;
  QApplication::restoreOverrideCursor();
  setMouseCursor( getStateAtPoint(ev->pos()) );
}
