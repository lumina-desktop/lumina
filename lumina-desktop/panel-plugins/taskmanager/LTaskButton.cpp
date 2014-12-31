//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LTaskButton.h"
#include "LSession.h"

LTaskButton::LTaskButton(QWidget *parent, bool smallDisplay) : LTBWidget(parent){
  actMenu = new QMenu(this);
  winMenu = new QMenu(this);
  UpdateMenus();
  showText = !smallDisplay;
  this->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  this->setAutoRaise(false); //make sure these always look like buttons
  this->setContextMenuPolicy(Qt::CustomContextMenu);
  winMenu->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(openActionMenu()) );
  connect(this, SIGNAL(clicked()), this, SLOT(buttonClicked()) );
  connect(winMenu, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(openActionMenu()) );
  connect(winMenu, SIGNAL(triggered(QAction*)), this, SLOT(winClicked(QAction*)) );
}

LTaskButton::~LTaskButton(){
	
}

//===========
//      PUBLIC
//===========
QList<LWinInfo> LTaskButton::windows(){
  return WINLIST;
}

QString LTaskButton::classname(){
  return cname;
}

void LTaskButton::addWindow(LWinInfo win){
  WINLIST << win;
  UpdateButton();
}

void LTaskButton::rmWindow(LWinInfo win){
  for(int i=0; i<WINLIST.length(); i++){
    if(WINLIST[i].windowID() == win.windowID()){
      WINLIST.removeAt(i);
      break;
    }
  }
  UpdateButton();
}

//==========
//    PRIVATE
//==========
LWinInfo LTaskButton::currentWindow(){
  if(WINLIST.length() == 1 || cWin.windowID()==0){
    return WINLIST[0]; //only 1 window - this must be it
  }else{
    return cWin;
  }
}

//=============
//   PUBLIC SLOTS
//=============
void LTaskButton::UpdateButton(){
  if(winMenu->isVisible()){ return; } //skip this if the window menu is currently visible for now
  bool statusOnly = WINLIST.length() == LWINLIST.length();
  LWINLIST = WINLIST;
  
  winMenu->clear();
  LXCB::WINDOWSTATE showstate = LXCB::IGNORE;
  for(int i=0; i<WINLIST.length(); i++){
    if(WINLIST[i].windowID() == 0){
      WINLIST.removeAt(i);
      i--;
      continue;
    }
    if(i==0 && !statusOnly){
      //Update the button visuals from the first window
      this->setIcon(WINLIST[i].icon(noicon));
      cname = WINLIST[i].Class();
      if(cname.isEmpty()){ 
	//Special case (chrome/chromium does not register *any* information with X except window title)
	cname = WINLIST[i].text();
	if(cname.contains(" - ")){ cname = cname.section(" - ",-1); }
      }
      this->setToolTip(cname);
    }
    bool junk;
    QAction *tmp = winMenu->addAction( WINLIST[i].icon(junk), WINLIST[i].text() );
      tmp->setData(i); //save which number in the WINLIST this entry is for
    LXCB::WINDOWSTATE stat = LSession::handle()->XCB->WindowState(WINLIST[i].windowID());
    if(stat==LXCB::ATTENTION){ showstate = stat; } //highest priority
    else if( stat==LXCB::ACTIVE && showstate != LXCB::ATTENTION){ showstate = stat; } //next priority
    else if( stat==LXCB::VISIBLE && showstate != LXCB::ATTENTION && showstate != LXCB::ACTIVE){ showstate = stat; }
    else if(showstate == LXCB::INVISIBLE || showstate == LXCB::IGNORE){ showstate = stat; } //anything is the same/better
  }
  //Now setup the button appropriately
  // - visibility
  if(showstate == LXCB::IGNORE || WINLIST.length() < 1){ this->setVisible(false); }
  else{ this->setVisible(true); }
  // - functionality
  if(WINLIST.length() == 1){
    //single window
    this->setPopupMode(QToolButton::DelayedPopup);
    this->setMenu(actMenu);
    if(showText){ this->setText( this->fontMetrics().elidedText(WINLIST[0].text(), Qt::ElideRight,80) ); }
    else if(noicon){ this->setText( this->fontMetrics().elidedText(cname, Qt::ElideRight ,80) ); }
    else{ this->setText(""); }
  }else if(WINLIST.length() > 1){
    //multiple windows
    this->setPopupMode(QToolButton::InstantPopup);
    this->setMenu(winMenu);
    if(noicon || showText){ this->setText( this->fontMetrics().elidedText(cname, Qt::ElideRight ,80) +" ("+QString::number(WINLIST.length())+")" ); }
    else{ this->setText("("+QString::number(WINLIST.length())+")"); }
  }
  this->setState(showstate); //Make sure this is after the button setup so that it properly sets the margins/etc
}

void LTaskButton::UpdateMenus(){
  //Action menu is very simple right now - can expand it later
  actMenu->clear();
  actMenu->addAction( LXDG::findIcon("window-close",""), tr("Close Window"), this, SLOT(closeWindow()) );
}

//=============
//   PRIVATE SLOTS
//=============
void LTaskButton::buttonClicked(){
  if(WINLIST.length() > 1){
    winMenu->popup(QCursor::pos());
  }else{
    triggerWindow(); 
  }
}

void LTaskButton::closeWindow(){
  if(winMenu->isVisible()){ winMenu->hide(); }
  LWinInfo win = currentWindow();
  LSession::handle()->XCB->CloseWindow(win.windowID());
  //LX11::CloseWindow(win.windowID());
  cWin = LWinInfo(); //clear the current
}

void LTaskButton::triggerWindow(){
  LWinInfo win = currentWindow();
  //Check which state the window is currently in and flip it to the other
  LX11::WINDOWSTATE state = LX11::GetWindowState(win.windowID());
  if(state == LX11::ACTIVE){
    qDebug() << "Minimize Window:" << this->text();
    LX11::IconifyWindow(win.windowID());
  }else if(state == LX11::VISIBLE){
    qDebug() << "Activate Window:" << this->text();
    LX11::ActivateWindow(win.windowID());
  }else{
    qDebug() << "Restore Window:" << this->text();
    LX11::RestoreWindow(win.windowID());
  }
  cWin = LWinInfo(); //clear the current
}

void LTaskButton::winClicked(QAction* act){
  //Get the window from the action
  if(act->data().toInt() < WINLIST.length()){
    cWin = WINLIST[act->data().toInt()];
  }else{ cWin = LWinInfo(); } //clear it
  //Now trigger the window
  triggerWindow();
}

void LTaskButton::openActionMenu(){
  //Get the Window the mouse is currently over
  QAction *act = winMenu->actionAt(QCursor::pos());
  if( act != 0 && winMenu->isVisible() ){
    //Get the window from the action
    if(act->data().toInt() < WINLIST.length()){
      cWin = WINLIST[act->data().toInt()];
    }else{ cWin = LWinInfo(); } //clear it
  }
  //Now show the action menu
  actMenu->popup(QCursor::pos());
}
