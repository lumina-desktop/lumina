//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LWindow.h"

LWindow::LWindow(WId client) : QFrame(){
  activeState = LWindow::Normal;
  this->setMouseTracking(true); //need this to determine mouse location when not clicked
  this->setObjectName("LWindowFrame");
  this->setWindowFlags(Qt::Window | Qt::X11BypassWindowManagerHint); //ensure that this frame does not get a frame itself
  InitWindow(); //initially create all the child widgets
  updateAppearance(); //this loads the appearance based on window/theme settings
}

LWindow::~LWindow(){
	
}

// =================
//         PUBLIC
// =================
//Return the ID of the managed window for the current graphics system (X11/Wayland/other)
WId LWindow::clientID(){ return CID; }

bool LWindow::hasFrame(){ return this->isEnabled(); }

// =================
//        PRIVATE
// =================
void LWindow::InitWindow(){
	titleBar = new QLabel(this); //This is the "container" for all the title buttons/widgets
	  titleBar->setObjectName("TitleBar");
	title = new QLabel(this); //Shows the window title/text
	  title->setObjectName("Title");
	  title->setCursor(Qt::SizeAllCursor);
	icon = new QLabel(this); //Contains the window icon
	  icon->setObjectName("Icon");
	minB = new QToolButton(this); //Minimize Button
	  minB->setObjectName("Minimize");
	  connect(minB, SIGNAL(clicked()), this, SLOT(minClicked()) );
	maxB = new QToolButton(this); //Maximize Button
	  maxB->setObjectName("Maximize");
	  connect(maxB, SIGNAL(clicked()), this, SLOT(maxClicked()) );
	closeB = new QToolButton(this);
	  closeB->setObjectName("Close");
	  connect(closeB, SIGNAL(clicked()), this, SLOT(closeClicked()) );
	otherB = new QToolButton(this); //Button to place any other actions
	  otherB->setObjectName("Options");
	  otherB->setToolButtonPopupMode(QToolButton::InstantPopup);
	otherM = new QMenu(this); //menu of "other" actions for the window
	  otherB->setMenu(otherM);
	  connect(otherM, SIGNAL(triggered(QAction*)), this, SLOT(otherClicked(QAction*)) );
}

ModStatus LWindow::getStateAtPoint(QPoint pt){

}

void LWindow::setMouseCursor(ModStatus state){
	
}

// =================
//    PUBLIC SLOTS
// =================
void LWindow::updateAppearance(){
	
}

// =================
//    PRIVATE SLOTS
// =================
void LWindow::closeClicked(){
	
}

void LWindow::minClicked(){
	
}

void LWindow::maxClicked(){
	
}

void LWindow::otherClicked(QAction* act){
	
}

// =====================
//         PROTECTED
// =====================
void LWindow::mousePressEvent(QMouseEvent *ev){
	
}

void LWindow::mouseMoveEvent(QMouseEvent *ev){
	
}

void LWindow::mouseReleaseEvent(QMouseEvent *ev){
	
}
