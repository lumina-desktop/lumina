//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LPanel.h"
#include "LSession.h"
#include "panel-plugins/systemtray/LSysTray.h"

LPanel::LPanel(QSettings *file, int scr, int num, QWidget *parent) : QWidget(){
  //Take care of inputs
  qDebug() << " - Creating Panel:" << scr << num;
  bgWindow = parent; //save for later
  //Setup the widget overlay for the entire panel to provide transparency effects
  panelArea = new QWidget(this);
  QBoxLayout *tmp = new QBoxLayout(QBoxLayout::LeftToRight,this);
	tmp->setContentsMargins(0,0,0,0);
	this->setLayout(tmp);
	tmp->addWidget(panelArea);
  settings = file;
  screennum = scr;
  screen = LSession::desktop();
  PPREFIX = "panel"+QString::number(screennum)+"."+QString::number(num)+"/";
  defaultpanel = (screen->screenGeometry(screennum).x()==0 && num==0);
  //if(settings->value("defaultpanel",QString::number(screen->primaryScreen())+".0").toString()==QString::number(screennum)+"."+QString::number(num) ){ defaultpanel=true;}
  //else{defaultpanel=false; }
  horizontal=true; //use this by default initially
  //Setup the panel
  qDebug() << " -- Setup Panel";
  this->setContentsMargins(0,0,0,0);
  this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  this->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint );
  this->setFocusPolicy(Qt::NoFocus);
  this->setWindowTitle("");
  this->setAttribute(Qt::WA_X11NetWmWindowTypeDock);
  this->setAttribute(Qt::WA_AlwaysShowToolTips);
  this->setObjectName("LuminaPanelWidget");
  panelArea->setObjectName("LuminaPanelPluginWidget");
  //LX11::SetAsPanel(this->winId()); //set proper type of window for a panel since Qt can't do it
  LX11::SetAsSticky(this->winId());
  layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(1);
    //layout->setSizeConstraint(QLayout::SetFixedSize);
  panelArea->setLayout(layout);
  QTimer::singleShot(1,this, SLOT(UpdatePanel()) ); //start this in a new thread
  connect(screen, SIGNAL(resized(int)), this, SLOT(UpdatePanel()) ); //in case the screen resolution changes
}

LPanel::~LPanel(){
	
}

//===========
// PUBLIC SLOTS
//===========
void LPanel::UpdatePanel(){
  //Create/Update the panel as designated in the Settings file
  settings->sync(); //make sure to catch external settings changes
  //First set the geometry of the panel and send the EWMH message to reserve that space
  qDebug() << "Update Panel";
  QString loc = settings->value(PPREFIX+"location","").toString();
  if(loc.isEmpty() && defaultpanel){ loc="top"; }
  if(loc=="top" || loc=="bottom"){ 
    horizontal=true; 
    layout->setAlignment(Qt::AlignLeft); 
    layout->setDirection(QBoxLayout::LeftToRight);
  }else{
    horizontal=false;
    layout->setAlignment(Qt::AlignTop);
    layout->setDirection(QBoxLayout::TopToBottom);
  }
  int ht = settings->value(PPREFIX+"height", 30).toInt(); //this is technically the distance into the screen from the edge
  //qDebug() << " - set Geometry";
  int xwid = screen->screenGeometry(screennum).width();
  int xhi = screen->screenGeometry(screennum).height();
  int xloc = screen->screenGeometry(screennum).x();
  //xloc=xoffset;
  if(loc=="top"){ //top of screen
    QSize sz = QSize(xwid, ht);
    this->setMinimumSize(sz);
    this->setMaximumSize(sz);
    this->setGeometry(xloc,0,xwid, ht );
    LX11::ReservePanelLocation(this->winId(), xloc, 0, this->width(), ht, "top");
  }else if(loc=="bottom"){ //bottom of screen
    QSize sz = QSize(xwid, ht);
    this->setMinimumSize(sz);
    this->setMaximumSize(sz);
    this->setGeometry(xloc,xhi-ht,xwid, ht );
    LX11::ReservePanelLocation(this->winId(), xloc, xhi-ht, this->width(), ht, "bottom");
  }else if(loc=="left"){ //left side of screen
    QSize sz = QSize(ht, xhi);
    this->setMinimumSize(sz);
    this->setMaximumSize(sz);
    this->setGeometry(xloc,0, ht, xhi);
    LX11::ReservePanelLocation(this->winId(), xloc, 0, ht, xhi, "left");
  }else{ //right side of screen
    QSize sz = QSize(ht, xhi);
    this->setMinimumSize(sz);
    this->setMaximumSize(sz);
    this->setGeometry(xloc+xwid-ht,0,ht, xhi);
    LX11::ReservePanelLocation(this->winId(), xloc+xwid-ht, 0, ht, xhi, "right");	  
  }
  //Now update the appearance of the toolbar
  QString color = settings->value(PPREFIX+"color", "rgba(255,255,255,160)").toString();
  QString style = "QWidget#LuminaPanelPluginWidget{ background: %1; border-radius: 5px; border: 1px solid transparent; }";
  style = style.arg(color);
  panelArea->setStyleSheet(style);
  
  //Then go through the plugins and create them as necessary
  QStringList plugins = settings->value(PPREFIX+"pluginlist", QStringList()).toStringList();
  if(defaultpanel && plugins.isEmpty()){
    plugins << "userbutton" << "taskmanager" << "spacer" << "systemtray" << "clock" << "systemdashboard";
  }
  qDebug() << " - Initialize Plugins: " << plugins;
  for(int i=0; i<plugins.length(); i++){
    //Ensure this plugin has a unique ID (NOTE: this numbering does not persist between sessions)
    if(!plugins[i].contains("---")){
      int num=1;
      while( plugins.contains(plugins[i]+"---"+QString::number(this->number())+"."+QString::number(num)) ){
        num++;
      }
      plugins[i] = plugins[i]+"---"+QString::number(this->number())+"."+QString::number(num);
    }
    //See if this plugin is already there or in a different spot
    bool found = false;
    for(int p=0; p<PLUGINS.length(); p++){
      if(PLUGINS[p]->type()==plugins[i]){
        found = true; //already exists
	//Make sure the plugin layout has the correct orientation
	if(horizontal){PLUGINS[p]->layout()->setDirection(QBoxLayout::LeftToRight); }
	else{ PLUGINS[p]->layout()->setDirection(QBoxLayout::TopToBottom); }
	QTimer::singleShot(0,PLUGINS[p], SLOT( OrientationChange() ) );
	//Now check the location of the plugin in the panel
	if(p!=i){ //wrong place in the panel
	  layout->takeAt(p); //remove the item from the current location
	  layout->insertWidget(i, PLUGINS[p]); //add the item into the correct location
	  PLUGINS.move(p,i); //move the identifier in the list to match
	}
	break;
      }
    }
    if(!found){
      //New Plugin
      qDebug() << " -- New Plugin:" << plugins[i];
      LPPlugin *plug = NewPP::createPlugin(plugins[i], panelArea, horizontal);
      if(plug != 0){ 
        PLUGINS.insert(i, plug);
        layout->insertWidget(i, PLUGINS[i]);
      }else{
        //invalid plugin type
	plugins.removeAt(i); //remove this invalid plugin from the list
	i--; //make sure we don't miss the next item with the re-order
      }
    }
  }
  //Now remove any extra plugins from the end
  for(int i=plugins.length(); i<PLUGINS.length(); i++){
    qDebug() << " -- Remove Plugin: " << i;
    //If this is the system tray - stop it first
    if( PLUGINS[i]->type().startsWith("systemtray---") ){
      static_cast<LSysTray*>(PLUGINS[i])->stop();
    }
    layout->takeAt(i); //remove from the layout
    delete PLUGINS.takeAt(i); //delete the actual widget
  }
  this->update();
  this->show(); //make sure the panel is visible now
  //Now go through and send the orientation update signal to each plugin
  for(int i=0; i<PLUGINS.length(); i++){
    QTimer::singleShot(0,PLUGINS[i], SLOT(OrientationChange()));
  }
}

void LPanel::UpdateLocale(){
  //The panel itself has no text to translate, just forward the signal to all the plugins
  for(int i=0; i<PLUGINS.length(); i++){
    QTimer::singleShot(1,PLUGINS[i], SLOT(LocaleChange()));
  }
}

void LPanel::UpdateTheme(){
  //The panel itself has no theme-based icons, just forward the signal to all the plugins
  for(int i=0; i<PLUGINS.length(); i++){
    QTimer::singleShot(1,PLUGINS[i], SLOT(ThemeChange()));
  }	
}


//===========
// PROTECTED
//===========
void LPanel::paintEvent(QPaintEvent *event){
  QPainter *painter = new QPainter(this);
  //Make sure the base background of the event rectangle is the associated rectangle from the BGWindow
  QRect rec(event->rect().x(), event->rect().y(), event->rect().width(), event->rect().height()); //already in global coords? (translating to bgWindow coords crashes Lumina)
  //Need to translate that rectangle to the background image coordinates
  //qDebug() << "Rec:" << rec.x() << rec.y();
  rec.moveTo( this->mapToGlobal(rec.topLeft()) ); //Need to change to global coords for the main window
  //qDebug() << "Global Rec:" << rec.x() << rec.y() << screennum;
  rec.moveTo( rec.x()-screen->screenGeometry(screennum).x(), rec.y() );
  //qDebug() << "Adjusted Global Rec:" << rec.x() << rec.y();
  painter->drawPixmap(event->rect(), QPixmap::grabWidget(bgWindow, rec) );
  QWidget::paintEvent(event); //now pass the event along to the normal painting event
}

