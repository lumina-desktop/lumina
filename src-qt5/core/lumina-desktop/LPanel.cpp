//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LPanel.h"
#include "LSession.h"
#include "panel-plugins/systemtray/LSysTray.h"

#define DEBUG 0

LPanel::LPanel(QSettings *file, int scr, int num, QWidget *parent) : QWidget(){
  //Take care of inputs
  this->setMouseTracking(true);
  if(DEBUG){ qDebug() << " - Creating Panel:" << scr << num; }
  bgWindow = parent; //save for later
  //Setup the widget overlay for the entire panel to provide transparency effects
  panelArea = new QWidget(this);
  QBoxLayout *tmp = new QBoxLayout(QBoxLayout::LeftToRight);
	tmp->setContentsMargins(0,0,0,0);
	this->setLayout(tmp);
	tmp->addWidget(panelArea);
  settings = file;
  screennum = scr;
  panelnum = num; //save for later
  screen = LSession::desktop();
  PPREFIX = "panel"+QString::number(screennum)+"."+QString::number(num)+"/";
  defaultpanel = (LSession::handle()->screenGeom(screennum).x()==0 && num==0);
  horizontal=true; //use this by default initially
  hidden = false; //use this by default
  //Setup the panel
  if(DEBUG){ qDebug() << " -- Setup Panel"; }
  this->setContentsMargins(0,0,0,0);
  this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  //panels cannot get keyboard focus otherwise it upsets the task manager window detection
  this->setAttribute(Qt::WA_X11DoNotAcceptFocus);
  this->setAttribute(Qt::WA_X11NetWmWindowTypeDock);
  this->setAttribute(Qt::WA_AlwaysShowToolTips);
  this->setWindowFlags(Qt::FramelessWindowHint | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
  //this->setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint);

  this->setWindowTitle("LuminaPanel");
  this->setObjectName("LuminaPanelBackgroundWidget");
  this->setStyleSheet("QToolButton::menu-indicator{ image: none; }");
  panelArea->setObjectName("LuminaPanelColor");
  layout = new QBoxLayout(QBoxLayout::LeftToRight);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(1);
    //layout->setSizeConstraint(QLayout::SetFixedSize);
  panelArea->setLayout(layout);
  //Set special window flags on the panel for proper usage
  this->show();
  LSession::handle()->XCB->SetAsPanel(this->winId());
  LSession::handle()->XCB->SetAsSticky(this->winId());
  
  QTimer::singleShot(1,this, SLOT(UpdatePanel()) ); //start this in a new thread
  //connect(screen, SIGNAL(resized(int)), this, SLOT(UpdatePanel()) ); //in case the screen resolution changes
}

LPanel::~LPanel(){
	
}

void LPanel::prepareToClose(){
  //Go through and remove all the plugins
  for(int i=0; i<PLUGINS.length(); i++){
    PLUGINS[i]->AboutToClose(); //any last cleanup for this plugin
    layout->takeAt(i); //remove from the layout
    PLUGINS.takeAt(i)->deleteLater(); //delete the actual widget
    LSession::processEvents();
    i--; //need to back up one space to not miss another plugin
  }	 
  this->hide();
}

void LPanel::scalePanel(double xscale, double yscale){
  int ht = settings->value(PPREFIX+"height", 30).toInt(); //this is technically the distance into the screen from the edge
  QString loc = settings->value(PPREFIX+"location","").toString().toLower();
  if(loc=="top" || loc=="bottom"){
    ht = qRound(ht*yscale);
  }else{
    ht = qRound(ht*xscale);
  }
  settings->setValue(PPREFIX+"height", ht);
  settings->sync();
  QTimer::singleShot(0, this, SLOT(UpdatePanel()) );
}

//===========
// PUBLIC SLOTS
//===========
void LPanel::UpdatePanel(bool geomonly){
  //Create/Update the panel as designated in the Settings file
  settings->sync(); //make sure to catch external settings changes
  //First set the geometry of the panel and send the EWMH message to reserve that space
  if(DEBUG){ qDebug() << "Update Panel: Geometry only=" << geomonly << "Screen Size:" << LSession::handle()->screenGeom(screennum); }
  hidden = settings->value(PPREFIX+"hidepanel",false).toBool();
  QString loc = settings->value(PPREFIX+"location","").toString().toLower();
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
  if(ht<=1){ ht = 30; } //some kind of error in the saved height - use the default value
  int hidesize = qRound(ht*0.01); //use 1% of the panel size
  if(hidesize<2){ hidesize=2; } //minimum of 2 pixels (need space for the mouse to go over it)
  if(hidden){ viswidth = hidesize; }
  else{ viswidth = ht; }
  if(DEBUG){ qDebug() << "Hidden Panel size:" << hidesize << "pixels"; }
  //qDebug() << " - set Geometry";
  int xwid = LSession::handle()->screenGeom(screennum).width();
  int xhi = LSession::handle()->screenGeom(screennum).height();
  int xloc = LSession::handle()->screenGeom(screennum).x();
  double panelPercent = settings->value(PPREFIX+"lengthPercent",100).toInt();
  if(panelPercent<1 || panelPercent>100){ panelPercent = 100; }
  panelPercent = panelPercent/100.0;
  QString panelPinLoc = settings->value(PPREFIX+"pinLocation","center").toString().toLower(); //[left/right/center] possible values (assume center otherwise)
  if(DEBUG){ qDebug() << " - Panel settings:" << QString::number(panelPercent)+QString("%") << panelPinLoc << loc; }
  //xloc=xoffset;
  if(loc=="top"){ //top of screen
    QSize sz = QSize(xwid*panelPercent, ht);
    if(panelPinLoc=="left"){} // no change to xloc
    else if(panelPinLoc=="right"){ xloc = xloc+xwid-sz.width(); }
    else{ xloc = xloc+((xwid-sz.width())/2) ; } //centered
    //qDebug() << " - Panel Sizing:" << xloc << sz;
    this->setMinimumSize(sz);
    this->setMaximumSize(sz);
    this->setGeometry(xloc,0,sz.width(), sz.height());
    //qDebug() << " - Reserve Panel Localation";
    if(!hidden){ LSession::handle()->XCB->ReserveLocation(this->winId(), this->geometry(), "top"); }
    else{ 
     LSession::handle()->XCB->ReserveLocation(this->winId(), QRect(xloc, 0, this->width(), hidesize), "top");
      hidepoint = QPoint(xloc, hidesize-ht);
      showpoint = QPoint(xloc, 0);
      this->move(hidepoint); //Could bleed over onto the screen above
    }
  }else if(loc=="bottom"){ //bottom of screen
    QSize sz = QSize(xwid*panelPercent, ht);
    if(panelPinLoc=="left"){} // no change to xloc
    else if(panelPinLoc=="right"){ xloc = xloc+xwid-sz.width(); }
    else{ xloc = xloc+((xwid-sz.width())/2) ; } //centered
    this->setMinimumSize(sz);
    this->setMaximumSize(sz);
    this->setGeometry(xloc,xhi-ht,sz.width(), ht );
    if(!hidden){ LSession::handle()->XCB->ReserveLocation(this->winId(), this->geometry(), "bottom"); }
    else{ 
      LSession::handle()->XCB->ReserveLocation(this->winId(), QRect(xloc, xhi-hidesize, this->width(), hidesize), "bottom"); 
      hidepoint = QPoint(xloc, xhi-hidesize);
      showpoint = QPoint(xloc, xhi-ht);
      this->move(hidepoint); //Could bleed over onto the screen below
    }
  }else if(loc=="left"){ //left side of screen
    QSize sz = QSize(ht, xhi*panelPercent);
    int yloc = 0;
    if(panelPinLoc=="left"){} //this is actually the top (left of center in length dimension)
    else if(panelPinLoc=="right"){ yloc = yloc+xhi-sz.height(); }
    else{ yloc = yloc+((xhi-sz.height())/2) ; } //centered
    this->setMinimumSize(sz);
    this->setMaximumSize(sz);
    this->setGeometry(xloc,yloc, ht, sz.height());
    if(!hidden){ LSession::handle()->XCB->ReserveLocation(this->winId(), this->geometry(), "left"); }
    else{ 
      LSession::handle()->XCB->ReserveLocation(this->winId(), QRect(xloc, yloc, hidesize, sz.height()), "left"); 
      hidepoint = QPoint(xloc-ht+hidesize, yloc);
      showpoint = QPoint(xloc, yloc);
      this->move(hidepoint); //Could bleed over onto the screen left
    }
  }else{ //right side of screen
    QSize sz = QSize(ht, xhi*panelPercent);
    int yloc = 0;
    if(panelPinLoc=="left"){} //this is actually the top (left of center in length dimension)
    else if(panelPinLoc=="right"){ yloc = yloc+xhi-sz.height(); }
    else{ yloc = yloc+((xhi-sz.height())/2) ; } //centered
    this->setMinimumSize(sz);
    this->setMaximumSize(sz);
    this->setGeometry(xloc+xwid-ht,yloc,ht, sz.height());
    if(!hidden){ LSession::handle()->XCB->ReserveLocation(this->winId(), this->geometry(), "right"); }  
    else{ 
      LSession::handle()->XCB->ReserveLocation(this->winId(), QRect(xloc+xwid-hidesize, yloc, hidesize, sz.height()), "right"); 
      hidepoint = QPoint(xloc+xwid-hidesize, yloc);
      showpoint = QPoint(xloc+xwid-ht, yloc);
      this->move(hidepoint); //Could bleed over onto the screen right
    }
  }
  if(DEBUG){ qDebug() << " - Done with panel geometry"; }
  if(geomonly){ return; }
  //Now update the appearance of the toolbar
  if(settings->value(PPREFIX+"customColor", false).toBool()){
    QString color = settings->value(PPREFIX+"color", "rgba(255,255,255,160)").toString();
    QString style = "QWidget#LuminaPanelColor{ background: %1; border-radius: 3px; border: 1px solid %1; }";
    style = style.arg(color);
    panelArea->setStyleSheet(style);
  }else{ 
    panelArea->setStyleSheet(""); //clear it and use the one from the theme
  }
  
  //Then go through the plugins and create them as necessary
  QStringList plugins = settings->value(PPREFIX+"pluginlist", QStringList()).toStringList();
  /*if(defaultpanel && plugins.isEmpty()){
    plugins << "userbutton" << "taskmanager" << "spacer" << "systemtray" << "clock" << "systemdashboard";
  }*/
  if(DEBUG){ qDebug() << " - Initialize Plugins: " << plugins; }
  for(int i=0; i<plugins.length(); i++){
    //Ensure this plugin has a unique ID (NOTE: this numbering does not persist between sessions)
    if(!plugins[i].contains("---")){
      int num=1;
      while( plugins.contains(plugins[i]+"---"+QString::number(screennum)+"."+QString::number(this->number())+"."+QString::number(num)) ){
        num++;
      }
      
      plugins[i] = plugins[i]+"---"+QString::number(screennum)+"."+QString::number(this->number())+"."+QString::number(num);
      //qDebug() << "Adjust Plugin ID:" << plugins[i];
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
      if(DEBUG){ qDebug() << " -- New Plugin:" << plugins[i] << i; }
      LPPlugin *plug = NewPP::createPlugin(plugins[i], panelArea, horizontal);
      if(plug != 0){ 
        PLUGINS.insert(i, plug);
        layout->insertWidget(i, PLUGINS[i]);
	connect(plug, SIGNAL(MenuClosed()), this, SLOT(checkPanelFocus()));
      }else{
        //invalid plugin type
	plugins.removeAt(i); //remove this invalid plugin from the list
	i--; //make sure we don't miss the next item with the re-order
      }
    }
    LSession::processEvents();
  }
  //Now remove any extra plugins from the end
  //qDebug() << "plugins:" << plugins;
  //qDebug() << "PLUGINS length:" << PLUGINS.length();
  for(int i=0; i<PLUGINS.length(); i++){
    if(plugins.contains(PLUGINS[i]->type())){ continue; } //good plugin - skip it
    if(DEBUG){ qDebug() << " -- Remove Plugin: " << PLUGINS[i]->type(); }
    //If this is the system tray - stop it first
    if( PLUGINS[i]->type().startsWith("systemtray---") ){
      static_cast<LSysTray*>(PLUGINS[i])->stop();
    }
    layout->takeAt(i); //remove from the layout
    PLUGINS.takeAt(i)->deleteLater(); //delete the actual widget
    LSession::processEvents();
    i--; //need to back up one space to not miss another plugin
  }
  this->update();
  this->show(); //make sure the panel is visible now
  if(hidden){ this->move(hidepoint); }
  //Now go through and send the orientation update signal to each plugin
  for(int i=0; i<PLUGINS.length(); i++){
    QTimer::singleShot(0,PLUGINS[i], SLOT(OrientationChange()));
  }
  LSession::processEvents();
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

// ===================
//     PRIVATE SLOTS
// ===================
void LPanel::checkPanelFocus(){
  if( !this->geometry().contains(QCursor::pos()) ){
    //Move the panel back to it's "hiding" spot
    if(hidden){ this->move(hidepoint); this->update(); }
    //Re-active the old window
    if(LSession::handle()->activeWindow()!=0){
      LSession::handle()->XCB->ActivateWindow(LSession::handle()->activeWindow());
    }
  }
  
}

//===========
// PROTECTED
//===========
void LPanel::paintEvent(QPaintEvent *event){
  QPainter *painter = new QPainter(this);
  //qDebug() << "Paint Tray:";
  //Make sure the base background of the event rectangle is the associated rectangle from the BGWindow
  QRect rec = this->geometry(); //start with the global geometry of the panel
  //Need to translate that rectangle to the background image coordinates
  //qDebug() << " - Rec:" << rec << hidden << this->geometry();
  rec.moveTo( rec.x()-LSession::handle()->screenGeom(screennum).x(), rec.y() );
  //qDebug() << " - Adjusted Global Rec:" << rec;
  painter->drawPixmap(QRect(0,0,this->width(), this->height()), bgWindow->grab(rec) );
  QWidget::paintEvent(event); //now pass the event along to the normal painting event
}

void LPanel::enterEvent(QEvent *event){
  //qDebug() << "Panel Enter Event:";
  if(hidden){
    //Move the panel out so it is fully available
    this->move(showpoint);
    this->update();
  }
  this->activateWindow();
  event->accept(); //just to quiet the compile warning
}

void LPanel::leaveEvent(QEvent *event){
  /*qDebug() << "Panel Leave Event:";
  qDebug() << "Panel Geom:" << this->geometry().x() << this->geometry().y() << this->geometry().width() << this->geometry().height() ;
  QPoint pt = QCursor::pos();
  qDebug() << "Mouse Point (global):" << pt.x() << pt.y();
  //pt = this->mapFromGlobal(pt);
  //qDebug() << "Mouse Point (local):" << pt.x() << pt.y();
  qDebug() << "Contained:" << this->geometry().contains(pt);*/
  checkPanelFocus();
  event->accept(); //just to quiet the compile warning
}

