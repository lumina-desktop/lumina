//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LSysTray.h"
#include "../../LSession.h"

LSysTray::LSysTray(QWidget *parent, QString id, bool horizontal) : LPPlugin(parent, id, horizontal){
  frame = new QFrame(this);
  frame->setContentsMargins(0,0,0,0);
  //frame->setStyleSheet("QFrame{ background: transparent; border: 1px solid transparent; border-radius: 3px; }");
  LI = new QBoxLayout( this->layout()->direction());
    frame->setLayout(LI);
    LI->setAlignment(Qt::AlignCenter);
    LI->setSpacing(0);
    LI->setContentsMargins(0,0,0,0);
  this->layout()->addWidget(frame);
  this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  //TrayID=0;
  upTimer = new QTimer(this);
    upTimer->setInterval(300000); //maximum time between refreshes is 5 minutes
    connect(upTimer, SIGNAL(timeout()), this, SLOT(checkAll()) );
  isRunning = false; stopping = false; checking = false; pending = false;
  QTimer::singleShot(100, this, SLOT(start()) );
  //Also do one extra check a minute or so after startup (just in case something got missed in the initial flood of registrations)
  QTimer::singleShot(90000,this, SLOT(checkAll()) ); 
  connect(LSession::handle(), SIGNAL(TrayListChanged()), this, SLOT(checkAll()) );
  connect(LSession::handle(), SIGNAL(TrayIconChanged(WId)), this, SLOT(UpdateTrayWindow(WId)) );
  connect(LSession::handle(), SIGNAL(VisualTrayAvailable()), this, SLOT(start()) );
}

LSysTray::~LSysTray(){
 if(isRunning){
   this->stop();
 }
}

void LSysTray::start(){
  if(isRunning || stopping){ return; } //already running
  isRunning = LSession::handle()->registerVisualTray(this->winId());
  qDebug() << "Visual Tray Started:" << this->type() << isRunning;
  if(isRunning){ 
    //upTimer->start();
    QTimer::singleShot(0,this, SLOT(checkAll()) ); 
  }
}

void LSysTray::stop(){
  if(!isRunning){ return; }
  stopping = true;
  upTimer->stop();
  //Now close down the system tray registry
  qDebug() << "Stop visual system tray:" << this->type();
  //LX11::closeSystemTray(TrayID);
  //TrayID = 0;
  disconnect(this); //remove any signals/slots
  isRunning = false;
  //Release all the tray applications and delete the containers
  if( !LSession::handle()->currentTrayApps(this->winId()).isEmpty() ){
    qDebug() << " - Remove tray applications";
    //This overall system tray is not closed down - go ahead and release them here	  
    for(int i=(trayIcons.length()-1); i>=0; i--){
      trayIcons[i]->detachApp();
      TrayIcon *cont = trayIcons.takeAt(i);
        LI->removeWidget(cont);
        cont->deleteLater();
    }
  }
  //Now let some other visual tray take over
  LSession::handle()->unregisterVisualTray(this->winId());
  qDebug() << "Done stopping visual tray";
}

// ========================
//    PRIVATE FUNCTIONS
// ========================
void LSysTray::checkAll(){
  if(!isRunning || stopping || checking){ pending = true; return; } //Don't check if not running at the moment
  checking = true;
  pending = false;
  //Make sure this tray should handle the windows (was not disabled in the backend)
  bool TrayRunning = LSession::handle()->registerVisualTray(this->winId());
  //qDebug() << "System Tray: Check tray apps";
  QList<WId> wins = LSession::handle()->currentTrayApps(this->winId());
  for(int i=0; i<trayIcons.length(); i++){
    int index = wins.indexOf(trayIcons[i]->appID());
    if(index < 0){
      //Tray Icon no longer exists: remove it
      qDebug() << " - Visual System Tray: Remove Icon:" << trayIcons[i]->appID();
      TrayIcon *cont = trayIcons.takeAt(i);
      cont->cleanup();
      LI->removeWidget(cont);
      cont->deleteLater();
      i--; //List size changed
      //Re-adjust the maximum widget size to account for what is left
      if(this->layout()->direction()==QBoxLayout::LeftToRight){
        this->setMaximumSize( trayIcons.length()*this->height(), 10000);
      }else{
        this->setMaximumSize(10000, trayIcons.length()*this->width());
      }
    }else{
      //Tray Icon already exists
      //qDebug() << " - SysTray: Update Icon";
      trayIcons[i]->update();
      wins.removeAt(index); //Already found - remove from the list
    }
  }
  //Now go through any remaining windows and add them
  for(int i=0; i<wins.length() && TrayRunning; i++){
    qDebug() << " - Visual System Tray: Add Icon:" << wins[i];
    TrayIcon *cont = new TrayIcon(this);
      connect(cont, SIGNAL(BadIcon()), this, SLOT(checkAll()) );
      //LSession::processEvents();
      trayIcons << cont;
      LI->addWidget(cont);
      //qDebug() << " - Update tray layout";
      if(this->layout()->direction()==QBoxLayout::LeftToRight){
        cont->setSizeSquare(this->height()-2-2*frame->frameWidth()); //horizontal tray
	this->setMaximumSize( trayIcons.length()*this->height(), 10000);
      }else{
	cont->setSizeSquare(this->width()-2-2*frame->frameWidth()); //vertical tray
	this->setMaximumSize(10000, trayIcons.length()*this->width());
      }
      //LSession::processEvents();
      //qDebug() << " - Attach tray app";
      cont->attachApp(wins[i]);
      if(cont->appID()==0){ 
	//could not attach window - remove the widget
	qDebug() << " - Invalid Tray App: Could Not Embed:"; 
	trayIcons.takeAt(trayIcons.length()-1); //Always at the end
	LI->removeWidget(cont);
	cont->deleteLater();
	continue;
      }
    LI->update(); //make sure there is no blank space in the layout
  }
  /*if(listChanged){
    //Icons got moved around: be sure to re-draw all of them to fix visuals
    for(int i=0; i<trayIcons.length(); i++){
      trayIcons[i]->update();
    }
  }*/
  //qDebug() << " - System Tray: check done";
  checking = false;
  if(pending){ QTimer::singleShot(0,this, SLOT(checkAll()) ); }
}

void LSysTray::UpdateTrayWindow(WId win){
  if(!isRunning || stopping || checking){ return; }
  for(int i=0; i<trayIcons.length(); i++){
    if(trayIcons[i]->appID()==win){
      //qDebug() << "System Tray: Update Window " << win;
      trayIcons[i]->repaint(); //don't use update() because we need an instant repaint (not a cached version)
      return; //finished now
    }
  }
  //Could not find tray in the list, run the checkall routine to make sure we are not missing any
  //qDebug() << "System Tray: Missing Window - check all";
  QTimer::singleShot(0,this, SLOT(checkAll()) );
}


