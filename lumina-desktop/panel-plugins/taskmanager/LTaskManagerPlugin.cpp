//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LTaskManagerPlugin.h"
#include "../../LSession.h"

LTaskManagerPlugin::LTaskManagerPlugin(QWidget *parent, QString id, bool horizontal) : LPPlugin(parent, id, horizontal){
  updating=false;
  timer = new QTimer(this);
	timer->setSingleShot(true);
	timer->setInterval(10); // 1/100 second
	connect(timer, SIGNAL(timeout()), this, SLOT(UpdateButtons()) ); 
  connect(LSession::instance(), SIGNAL(WindowListEvent()), this, SLOT(checkWindows()) );
  this->layout()->setContentsMargins(0,0,0,0);
  QTimer::singleShot(0,this, SLOT(UpdateButtons()) ); //perform an initial sync
  //QTimer::singleShot(100,this, SLOT(OrientationChange()) ); //perform an initial sync
}

LTaskManagerPlugin::~LTaskManagerPlugin(){
	
}

//==============
//    PRIVATE SLOTS
//==============
void LTaskManagerPlugin::UpdateButtons(){
  if(updating){ timer->start(); return; } //check again in a moment
  //Make sure this only runs one at a time
  updating=true;
  //Get the current window list
  QList<WId> winlist = LX11::WindowList();
  //qDebug() << "Update Buttons:" << winlist;
  //Now go through all the current buttons first
  for(int i=0; i<BUTTONS.length(); i++){
    //Get the windows managed in this button
    QList<LWinInfo> WI = BUTTONS[i]->windows();
    bool updated=false;
    for(int w=0; w<WI.length(); w++){
      if( winlist.contains( WI[w].windowID() ) ){
        //Still current window - update it later
	winlist.removeAll(WI[w].windowID()); //remove this window from the list since it is done
      }else{
	//Window was closed - remove it
	if(WI.length()==1){
	  //Remove the entire button
	  this->layout()->takeAt(i); //remove from the layout
	  delete BUTTONS.takeAt(i);
	  i--;
	  updated = true; //prevent updating a removed button
	  break; //break out of the button->window loop
	}else{
	  //qDebug() << "Remove Window:" << WI[w].windowID() << "Button:" << w;
	  BUTTONS[i]->rmWindow(WI[w]); // one of the multiple windows for the button
	  WI.removeAt(w); //remove this window from the list
	  w--;
	}
	updated=true; //button already changed
      }
    }
    if(!updated){
      //qDebug() << "Update Button:" << i;
      QTimer::singleShot(1,BUTTONS[i], SLOT(UpdateButton()) ); //keep moving on
    }
  }
  //Now go through the remaining windows
  for(int i=0; i<winlist.length(); i++){
    //New windows, create buttons for each (add grouping later)
    //Check for a button that this can just be added to
    QString ctxt = LX11::WindowClass(winlist[i]);
    bool found = false;
    for(int b=0; b<BUTTONS.length(); b++){
      if(BUTTONS[b]->classname()== ctxt){
        found = true;
	//qDebug() << "Add Window to Button:" << b;
	BUTTONS[b]->addWindow(winlist[i]);
	break;
      }
    }
    if(!found){
      //No group, create a new button
      //qDebug() << "New Button";
      LTaskButton *but = new LTaskButton(this);
        but->addWindow( LWinInfo(winlist[i]) );
	if(this->layout()->direction()==QBoxLayout::LeftToRight){
	    but->setIconSize(QSize(this->height(), this->height()));
	}else{
	    but->setIconSize(QSize(this->width(), this->width()));
	}
      this->layout()->addWidget(but);
      BUTTONS << but;
    }
  }
  updating=false; //flag that we are done updating the buttons
}

void LTaskManagerPlugin::checkWindows(){
  timer->start();
}