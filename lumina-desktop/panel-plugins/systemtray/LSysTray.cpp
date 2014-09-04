//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LSysTray.h"
#include "../../LSession.h"

#include <LuminaX11.h>
//X includes (these need to be last due to Qt compile issues)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xdamage.h>

//Static variables for damage detection (tray update notifications)
static int dmgEvent = 0;
static int dmgError = 0;

LSysTray::LSysTray(QWidget *parent, QString id, bool horizontal) : LPPlugin(parent, id, horizontal){
  frame = new QFrame(this);
  frame->setContentsMargins(0,0,0,0);
  //frame->setStyleSheet("QFrame{ background: transparent; border: 1px solid transparent; border-radius: 5px; }");
  LI = new QBoxLayout( this->layout()->direction());
    frame->setLayout(LI);
    LI->setAlignment(Qt::AlignCenter);
    LI->setSpacing(1);
    LI->setContentsMargins(0,0,0,0);
  this->layout()->addWidget(frame);
  this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  TrayID=0;
  upTimer = new QTimer(this);
    upTimer->setInterval(300000); //maximum time between refreshes is 5 minutes
    connect(upTimer, SIGNAL(timeout()), this, SLOT(checkAll()) );
  isRunning = false;
  start();
}

LSysTray::~LSysTray(){
 if(isRunning){
   this->stop();
 }
}

void LSysTray::start(){
  if(TrayID!=0){ return; } //already running
  //Make sure we catch all events right away
  connect(LSession::instance(),SIGNAL(aboutToQuit()),this,SLOT(closeAll()) );
  connect(LSession::instance(),SIGNAL(TrayEvent(XEvent*)), this, SLOT(checkXEvent(XEvent*)) );
  isRunning = true;
  TrayID = LX11::startSystemTray(0); //LSession::desktop()->screenNumber(this));
  if(TrayID!=0){
    XSelectInput(QX11Info::display(), TrayID, InputOutput); //make sure TrayID events get forwarded here
    XDamageQueryExtension( QX11Info::display(), &dmgEvent, &dmgError);
    //Now connect the session logout signal to the close function
    qDebug() << "System Tray Started Successfully";
    upTimer->start();
    //QTimer::singleShot(100, this, SLOT(initialTrayIconDetect()) );
  }else{
    disconnect(this);
  }
  isRunning = (TrayID!=0);
}

void LSysTray::stop(){
  if(!isRunning){ return; }
  upTimer->stop();
  //Now close down the system tray registry
  qDebug() << "Stop system Tray";
  LX11::closeSystemTray(TrayID);
  TrayID = 0;
  disconnect(this); //remove any signals/slots
  isRunning = false;
  //Release all the tray applications and delete the containers
  qDebug() << " - Remove tray applications";
  for(int i=(trayIcons.length()-1); i>=0; i--){
    trayIcons[i]->detachApp();
    TrayIcon *cont = trayIcons.takeAt(i);
      LI->removeWidget(cont);
      delete cont;
  }
  qDebug() << "Done stopping system tray";
}

// ========================
//    PRIVATE FUNCTIONS
// ========================
void LSysTray::checkXEvent(XEvent *event){
  if(!isRunning){ return; }
  switch(event->type){
  // -------------------------
    case ClientMessage:
    	//Only check if the client is the system tray, otherwise ignore
    	if(event->xany.window == TrayID){
    	  //qDebug() << "SysTray: ClientMessage";
	    switch(event->xclient.data.l[1]){
		case SYSTEM_TRAY_REQUEST_DOCK:
		  addTrayIcon(event->xclient.data.l[2]); //Window ID
		  break;
		//case SYSTEM_TRAY_BEGIN_MESSAGE:
		  //Let the window manager handle the pop-up messages for now
		  //break;    	    
		//case SYSTEM_TRAY_CANCEL_MESSAGE:
		  //Let the window manager handle the pop-up messages for now
		  //break;
	    }
    	}
    	break;
    case SelectionClear:
    	if(event->xany.window == TrayID){
    	  //qDebug() << "SysTray: Selection Clear";
    	  this->stop(); //de-activate this system tray (release all embeds)
    	}
    	break;
    case DestroyNotify:
	//qDebug() << "SysTray: DestroyNotify";
        removeTrayIcon(event->xany.window); //Check for removing an icon
        break;
    
    case ConfigureNotify:
	for(int i=0; i<trayIcons.length(); i++){
	  if(event->xany.window==trayIcons[i]->appID()){
	    //qDebug() << "SysTray: Configure Event" << trayIcons[i]->appID();
	    trayIcons[i]->update(); //trigger a repaint event
	    break;
	  }
	}
    default:
	if(event->type == dmgEvent+XDamageNotify){
	  WId ID = reinterpret_cast<XDamageNotifyEvent*>(event)->drawable;	
	  //qDebug() << "SysTray: Damage Event";
	  for(int i=0; i<trayIcons.length(); i++){
	    if(ID==trayIcons[i]->appID()){ 
	      //qDebug() << "SysTray: Damage Event" << ID;
	      trayIcons[i]->update(); //trigger a repaint event
	      break;
	    }
	  }
        }

  }//end of switch over event type
}	

void LSysTray::closeAll(){
  //Actually close all the tray apps (not just unembed)
    //This is used when the desktop is shutting everything down
  for(int i=0; i<trayIcons.length(); i++){
    LX11::CloseWindow(trayIcons[i]->appID());
  }
  
}

void LSysTray::checkAll(){
  for(int i=0; i<trayIcons.length(); i++){
    trayIcons[i]->update();
  }	  
}

void LSysTray::initialTrayIconDetect(){
  // WARNING: This is still experimental and should be disabled by default!!
  QList<WId> wins = LX11::findOrphanTrayWindows();
  for(int i=0; i<wins.length(); i++){
    //addTrayIcon(wins[i]);
    qDebug() << "Initial Tray Window:" << wins[i] << LX11::WindowClass(wins[i]);
  }
}

void LSysTray::addTrayIcon(WId win){
  if(win == 0 || !isRunning){ return; }
  //qDebug() << "System Tray: Add Tray Icon:" << win;
  bool exists = false;
  for(int i=0; i<trayIcons.length(); i++){
    if(trayIcons[i]->appID() == win){ exists=true; break; }
  }
  if(!exists){
    //qDebug() << " - New Icon Window:" << win;
    TrayIcon *cont = new TrayIcon(this);
      QCoreApplication::processEvents();
      connect(cont, SIGNAL(AppClosed()), this, SLOT(trayAppClosed()) );
      connect(cont, SIGNAL(AppAttached()), this, SLOT(updateStatus()) );
      trayIcons << cont;
      LI->addWidget(cont);
      //qDebug() << " - Update tray layout";
      if(this->layout()->direction()==QBoxLayout::LeftToRight){
        cont->setSizeSquare(this->height()-2*frame->frameWidth()); //horizontal tray
	this->setMaximumSize( trayIcons.length()*this->height(), 10000);
      }else{
	cont->setSizeSquare(this->width()-2*frame->frameWidth()); //vertical tray
	this->setMaximumSize(10000, trayIcons.length()*this->width());
      }
      LSession::processEvents();
      //qDebug() << " - Attach tray app";
      cont->attachApp(win);
    LI->update(); //make sure there is no blank space
  }
}

void LSysTray::removeTrayIcon(WId win){
  if(win==0 || !isRunning){ return; }
  for(int i=0; i<trayIcons.length(); i++){
    if(trayIcons[i]->appID()==win){
      //qDebug() << " - Remove Icon Window:" << win;
      //Remove it from the layout and keep going
      TrayIcon *cont = trayIcons.takeAt(i);
      LI->removeWidget(cont);
      delete cont;
      i--; //make sure we don't miss an item when we continue
      QCoreApplication::processEvents();
    }
  }
  //Re-adjust the maximum widget size to account for what is left
  if(this->layout()->direction()==QBoxLayout::LeftToRight){
    this->setMaximumSize( trayIcons.length()*this->height(), 10000);
  }else{
    this->setMaximumSize(10000, trayIcons.length()*this->width());
  }
  LI->update(); //update the layout (no gaps)
  this->update(); //update the main widget appearance
}

void LSysTray::updateStatus(){
  qDebug() << "System Tray: Client Attached";
  LI->update(); //make sure there is no blank space
  //qDebug() << " - Items:" << trayIcons.length();
}

void LSysTray::trayAppClosed(){
  if(!isRunning){ return; }
  for(int i=0;  i<trayIcons.length(); i++){
    if(trayIcons[i]->appID() == 0){
      qDebug() << "System Tray: Removing icon";
      TrayIcon *cont = trayIcons.takeAt(i);
      LI->removeWidget(cont);
      delete cont;
      QCoreApplication::processEvents();
    }
  }
  //Re-adjust the maximum widget size
  if(this->layout()->direction()==QBoxLayout::LeftToRight){
    this->setMaximumSize( trayIcons.length()*this->height(), 10000);
  }else{
    this->setMaximumSize(10000, trayIcons.length()*this->width());
  }
  LI->update(); //update the layout (no gaps)
  this->update();	
}

