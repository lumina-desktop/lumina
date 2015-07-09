//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LDesktop.h"
#include "LSession.h"

#include <LuminaOS.h>
#include <LuminaX11.h>
#include "LWinInfo.h"

#define DEBUG 1

LDesktop::LDesktop(int deskNum, bool setdefault) : QObject(){

  DPREFIX = "desktop-"+QString::number(deskNum)+"/";
  desktopnumber = deskNum;
  desktop = QApplication::desktop();
  defaultdesktop = setdefault; //(desktop->screenGeometry(desktopnumber).x()==0);
  desktoplocked = true;
  issyncing = bgupdating = false;
  usewinmenu=false;

  //Setup the internal variables
  settings = new QSettings(QSettings::UserScope, "LuminaDE","desktopsettings", this);
  //qDebug() << " - Desktop Settings File:" << settings->fileName();
  if(!QFile::exists(settings->fileName())){ settings->setValue(DPREFIX+"background/filelist",QStringList()<<"default"); settings->sync(); }
  bgWindow = 0;
  bgDesktop = 0;
  QTimer::singleShot(1,this, SLOT(InitDesktop()) );

}

LDesktop::~LDesktop(){
  delete deskMenu;
  //delete appmenu;
  delete winMenu;
  delete bgWindow;
  delete workspacelabel;
  delete wkspaceact;
}

int LDesktop::Screen(){
  return desktopnumber;
}

void LDesktop::show(){
  if(bgWindow!=0){ bgWindow->show(); }
  if(bgDesktop!=0){ bgDesktop->show(); }
  for(int i=0; i<PANELS.length(); i++){ PANELS[i]->show(); }
}

/*void LDesktop::hide(){
  if(bgWindow!=0){ bgWindow->hide(); }
  if(bgDesktop!=0){ bgDesktop->hide(); }
  for(int i=0; i<PANELS.length(); i++){ PANELS[i]->hide(); }
}*/

void LDesktop::prepareToClose(){
  //Get any panels ready to close
  issyncing = true; //Stop handling any watcher events
  for(int i=0; i<PANELS.length(); i++){ PANELS[i]->prepareToClose(); delete PANELS.takeAt(i); i--; }
  //Now close down any desktop plugins
  desktoplocked = true; //make sure that plugin settings are preserved during removal
  //Remove all the current containers
  QList<QMdiSubWindow*> wins = bgDesktop->subWindowList();
  for(int i=0; i<wins.length(); i++){
    wins[i]->setWhatsThis(""); //clear this so it knows it is being temporarily removed
    bgDesktop->removeSubWindow(wins[i]->widget()); //unhook plugin from container
    bgDesktop->removeSubWindow(wins[i]); //remove container from screen
    delete wins[i]; //delete old container
  }
  for(int i=0; i<PLUGINS.length(); i++){delete PLUGINS.takeAt(i); i--; }
}

WId LDesktop::backgroundID(){
  if(bgWindow!=0){ return bgWindow->winId(); }
  else{ return QX11Info::appRootWindow(); }
}

QRect LDesktop::availableScreenGeom(){
  //Return a QRect containing the (global) screen area that is available (not under any panels)
  if(bgDesktop!=0){
    return globalWorkRect; //saved from previous calculations
  }else{
    return desktop->screenGeometry(desktopnumber);
  }	  
}

void LDesktop::SystemLogout(){
  LSession::handle()->systemWindow();
}

void LDesktop::SystemTerminal(){
  LSession::handle()->sessionSettings()->sync(); //make sure it is up to date
  QString term = LSession::handle()->sessionSettings()->value("default-terminal","xterm").toString();
  if(term.endsWith(".desktop")){ term = "lumina-open \""+term+"\""; }
  LSession::LaunchApplication(term);
}

void LDesktop::SystemFileManager(){
  //Just open the home directory
  QString fm =  "lumina-open \""+QDir::homePath()+"\"";
  //QString fm = LSession::handle()->sessionSettings()->value("default-filemanager","lumina-fm").toString();
  //if(fm.endsWith(".desktop")){ fm = "lumina-open \""+fm+"\""; }
  LSession::LaunchApplication(fm);
}

void LDesktop::SystemApplication(QAction* act){
  if(!act->whatsThis().isEmpty() && act->parent()==deskMenu){
    LSession::LaunchApplication("lumina-open \""+act->whatsThis()+"\"");
  }
}

void LDesktop::checkResolution(){
  //Compare the current screen resolution with the last one used/saved and adjust config values *only*
  //NOTE: This is only run the first time this desktop is created (before loading all the interface) - not on each desktop change
  int oldWidth = settings->value(DPREFIX+"screen/lastWidth",-1).toInt();
  int oldHeight = settings->value(DPREFIX+"screen/lastHeight",-1).toInt();
  QRect scrn = LSession::desktop()->screenGeometry(desktopnumber);
  issyncing = true;
  settings->setValue(DPREFIX+"screen/lastWidth",scrn.width());
  settings->setValue(DPREFIX+"screen/lastHeight",scrn.height());

  if(oldWidth<1 || oldHeight<1 || scrn.width()<1 || scrn.height()<1){
    //nothing to do - something invalid
  }else if(scrn.width()==oldWidth && scrn.height()==oldHeight){
    //nothing to do - same as before
  }else{
    //Calculate the scale factor between the old/new sizes in each dimension 
    //  and forward that on to all the interface elements
    double xscale = scrn.width()/((double) oldWidth);
    double yscale = scrn.height()/((double) oldHeight);
    if(DEBUG){
      qDebug() << "Screen Resolution Changed:" << desktopnumber;
      qDebug() << " - Old:" << QString::number(oldWidth)+"x"+QString::number(oldHeight);
      qDebug() << " - New:" << QString::number(scrn.width())+"x"+QString::number(scrn.height());
      qDebug() << " - Scale Factors:" << xscale << yscale;
    }
    //Update any panels in the config file
    for(int i=0; i<4; i++){
      QString PPREFIX = "panel"+QString::number(desktopnumber)+"."+QString::number(i)+"/";
      int ht = settings->value(PPREFIX+"height",-1).toInt();
      if(ht<1){ continue; } //no panel height defined
      QString loc = settings->value(PPREFIX+"location","top").toString().toLower();
      if(loc=="top" || loc=="bottom"){
        settings->setValue(PPREFIX+"height", (int) ht*yscale); //vertical dimension
      }else{
        settings->setValue(PPREFIX+"height", (int) ht*xscale); //horizontal dimension
      }
    }
    //Update any desktop plugins
    QStringList plugs = settings->value(DPREFIX+"pluginlist").toStringList();
    QFileInfoList files = LSession::handle()->DesktopFiles();
    for(int i=0; i<files.length(); i++){
      plugs << "applauncher::"+files[i].absoluteFilePath()+"---"+DPREFIX;
    }
    //QString pspath = QDir::homePath()+"/.lumina/desktop-plugins/%1.conf";
    QSettings *DP = LSession::handle()->DesktopPluginSettings();
    QStringList keys = DP->allKeys();
    for(int i=0; i<plugs.length(); i++){
      QStringList filter = keys.filter(plugs[i]);
      for(int j=0; j<filter.length(); j++){
        //Has existing settings - need to adjust it
	  if(filter[j].endsWith("location/height")){ DP->setValue( filter[j], qRound(DP->value(filter[j]).toInt()*yscale) ); }
	  if(filter[j].endsWith("location/width")){ DP->setValue( filter[j], qRound(DP->value(filter[j]).toInt()*xscale) ); }
	  if(filter[j].endsWith("location/x")){ DP->setValue( filter[j], qRound(DP->value(filter[j]).toInt()*xscale) ); }
	  if(filter[j].endsWith("location/y")){ DP->setValue( filter[j], qRound(DP->value(filter[j]).toInt()*yscale) ); }
	  if(filter[j].endsWith("IconSize")){ DP->setValue( filter[j], qRound(DP->value(filter[j]).toInt()*yscale) ); }
	  if(filter[j].endsWith("iconsize")){ DP->setValue( filter[j], qRound(DP->value(filter[j]).toInt()*yscale) ); }
      }
    }
    DP->sync(); //make sure it gets saved to disk right away
    
  }
  issyncing = false;
}

LDPluginContainer* LDesktop::CreateDesktopPluginContainer(LDPlugin *plug){
  //Verify that a container does not already exist for this plugin
  QList<QMdiSubWindow*> wins = bgDesktop->subWindowList();
  for(int i=0; i<wins.length(); i++){
    if(wins[i]->whatsThis()==plug->ID()){ return 0; }
  }
  //Create a new plugin container
  LDPluginContainer *win = new LDPluginContainer(plug, desktoplocked);
  if(desktoplocked){ 
	  bgDesktop->addSubWindow(win, Qt::Tool | Qt::FramelessWindowHint);
  }else{ bgDesktop->addSubWindow(win, Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint); }
  win->loadInitialPosition();
  if(DEBUG){ 
    qDebug() << "Initial DP Geom:" << plug->geometry();
    qDebug() << "  - Container Geom:" << win->geometry();
  }
  win->show();
  plug->update();
  win->update();
  bgDesktop->update();
  QApplication::processEvents();
  QApplication::processEvents();
	  
  connect(win, SIGNAL(PluginRemoved(QString)), this, SLOT(DesktopPluginRemoved(QString)) );
  return win;
}

QPoint LDesktop::findNewPluginLocation(QRegion avail, QSize winsize){
  //This just searches through the region of available space until it find the first location where it
  //  will fit without overlapping anything else (scanning left->right, top->bottom)
  //return QPoint(-1,-1); //just for testing
  QRect bounds = avail.boundingRect();
  qDebug() << "Bounds:" << bounds;
  if(bounds.width()<winsize.width() || bounds.height()<winsize.height()){ return QPoint(-1,-1); }

  QPoint pt = bounds.topLeft(); //start in upper-left corner
  bool found = false;
  if(DEBUG){ qDebug() << "Check Availability:" << bounds << winsize; }
  while(pt.y()+winsize.height() < bounds.bottom() && !found){
    int dy = winsize.height()/2;
    while(pt.x()+winsize.width() < bounds.right() && !found){
      //Check the horizontal position (incrementing as necessary)
      QRect inter = avail.intersected(QRect(pt, winsize)).boundingRect();
      if(DEBUG){ qDebug() << "Check X:" << pt << " - Inter:" << inter; }
      if(inter.size() == winsize && avail.contains(inter) ){ found = true; } //use this point
      else{
	int dx = winsize.width() - inter.width();
        if(dx>0 && inter.left() > pt.x()){ pt.setX( inter.left() ); }
	else if(inter.width()==0){ pt.setX( pt.x()+winsize.width() ); }
	else{ pt.setX( pt.x()+inter.width() ); }
	//Also adjust the dy value to the smallest amount
	int ddy = inter.height() - winsize.height();
	if(ddy < dy && ddy>0){ dy = ddy; }
      }
     
    }
    if(!found){
      //Nothing in the horizontal direction - increment the vertical dimension
      pt.setX( bounds.left() ); //reset back to the left-most edge
      pt.setY( pt.y()+dy );
      if(DEBUG){ qDebug() << "Check Y:" << pt << dy; }
    }
  }
  //qDebug() << "Found Point:" << found << pt;
  if(!found){ return QPoint(-1,-1); } //no space found - return an invalid point
  else{ return pt; }
}

// =====================
//     PRIVATE SLOTS
// =====================
void LDesktop::InitDesktop(){
  //This is called *once* during the main initialization routines
  checkResolution(); //Adjust the desktop config file first (if necessary)
  if(DEBUG){ qDebug() << "Init Desktop:" << desktopnumber; }
    connect(desktop, SIGNAL(resized(int)), this, SLOT(UpdateGeometry(int)));
  if(DEBUG){ qDebug() << "Desktop #"<<desktopnumber<<" -> "<< desktop->screenGeometry(desktopnumber).x() << desktop->screenGeometry(desktopnumber).y() << desktop->screenGeometry(desktopnumber).width() << desktop->screenGeometry(desktopnumber).height(); }
  deskMenu = new QMenu(0);
    connect(deskMenu, SIGNAL(triggered(QAction*)), this, SLOT(SystemApplication(QAction*)) );
  winMenu = new QMenu(0);
    winMenu->setTitle(tr("Window List"));
    winMenu->setIcon( LXDG::findIcon("preferences-system-windows","") );
  connect(winMenu, SIGNAL(triggered(QAction*)), this, SLOT(winClicked(QAction*)) );
  workspacelabel = new QLabel(0);
    workspacelabel->setAlignment(Qt::AlignCenter);
  wkspaceact = new QWidgetAction(0);
    wkspaceact->setDefaultWidget(workspacelabel);
  bgtimer = new QTimer(this);
    bgtimer->setSingleShot(true);
    connect(bgtimer, SIGNAL(timeout()), this, SLOT(UpdateBackground()) );
  //watcher = new QFileSystemWatcher(this);
    connect(QApplication::instance(), SIGNAL(DesktopConfigChanged()), this, SLOT(SettingsChanged()) );
    connect(QApplication::instance(), SIGNAL(DesktopFilesChanged()), this, SLOT(UpdateDesktop()) );
    //watcher->addPath(settings->fileName());
    //connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(SettingsChanged()) );

  if(DEBUG){ qDebug() << "Create bgWindow"; }
  bgWindow = new QWidget();
	bgWindow->setObjectName("bgWindow");
	bgWindow->setContextMenuPolicy(Qt::CustomContextMenu);
  	bgWindow->setWindowFlags(Qt::FramelessWindowHint);
	LX11::SetAsDesktop(bgWindow->winId());
	bgWindow->setGeometry(desktop->screenGeometry(desktopnumber));
	connect(bgWindow, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowMenu()) );
  if(DEBUG){ qDebug() << "Create bgDesktop"; }
  bgDesktop = new QMdiArea(bgWindow);
	//Make sure the desktop area is transparent to show the background
        bgDesktop->setBackground( QBrush(Qt::NoBrush) );
	bgDesktop->setStyleSheet( "QMdiArea{ border: none; background: transparent;}" );
  if(DEBUG){ qDebug() << " - Desktop Init Done:" << desktopnumber; }
  //Start the update processes
  QTimer::singleShot(10,this, SLOT(UpdateMenu()) );
  QTimer::singleShot(0,this, SLOT(UpdateBackground()) );
  QTimer::singleShot(1,this, SLOT(UpdateDesktop()) );
  QTimer::singleShot(2,this, SLOT(UpdatePanels()) );
}

void LDesktop::SettingsChanged(){
  if(issyncing){ return; } //don't refresh for internal modifications to the 
  issyncing = true;
  qDebug() << "Found Settings Change:" << desktopnumber;
  settings->sync(); //make sure to sync with external settings changes
  UpdateBackground();
  UpdateDesktop();
  UpdatePanels();
  UpdateMenu();
  issyncing = false;
  QTimer::singleShot(100, this, SLOT(UnlockSettings()) ); //give it a few moments to settle before performing another sync
}

void LDesktop::UpdateMenu(bool fast){
  if(DEBUG){ qDebug() << " - Update Menu:" << desktopnumber; }
  //Put a label at the top
  int num = LX11::GetCurrentDesktop();
  if(DEBUG){ qDebug() << "Found workspace number:" << num; }
  if(num < 0){ workspacelabel->setText( "<b>"+tr("Lumina Desktop")+"</b>"); }
  else{ workspacelabel->setText( "<b>"+QString(tr("Workspace %1")).arg(QString::number(num+1))+"</b>"); }
  if(fast && usewinmenu){ UpdateWinMenu(); }
  if(fast){ return; } //already done
  deskMenu->clear(); //clear it for refresh
  deskMenu->addAction(wkspaceact);
  deskMenu->addSeparator();
  //Now load the user's menu setup and fill the menu
  QStringList items = settings->value("menu/itemlist", QStringList()<< "terminal" << "filemanager" <<"applications" << "line" << "settings" ).toStringList();
  usewinmenu=false;
  for(int i=0; i<items.length(); i++){
    if(items[i]=="terminal"){ deskMenu->addAction(LXDG::findIcon("utilities-terminal",""), tr("Terminal"), this, SLOT(SystemTerminal()) ); }
    else if(items[i]=="filemanager"){ deskMenu->addAction( LXDG::findIcon("Insight-FileManager",""), tr("Browse System"), this, SLOT(SystemFileManager()) ); }
    else if(items[i]=="applications"){ deskMenu->addMenu( LSession::handle()->applicationMenu() ); }
    else if(items[i]=="line"){ deskMenu->addSeparator(); }
    else if(items[i]=="settings"){ deskMenu->addMenu( LSession::handle()->settingsMenu() ); }
    else if(items[i]=="windowlist"){ deskMenu->addMenu( winMenu); usewinmenu=true;}
    else if(items[i].startsWith("app::::") && items[i].endsWith(".desktop")){
      //Custom *.desktop application
      QString file = items[i].section("::::",1,1).simplified();
      bool ok = false;
      XDGDesktop xdgf = LXDG::loadDesktopFile(file, ok);
      if(ok){
        deskMenu->addAction( LXDG::findIcon(xdgf.icon,""), xdgf.name)->setWhatsThis(file);
	}else{
	  qDebug() << "Could not load application file:" << file;
	}
    }
  }
  //Now add the system quit options
  deskMenu->addSeparator();
  if(!desktoplocked){
    deskMenu->addAction(LXDG::findIcon("document-encrypt",""),tr("Lock Desktop"), this, SLOT(ToggleDesktopLock()) );
    deskMenu->addAction(LXDG::findIcon("snap-orthogonal",""),tr("Snap Plugins to Grid"), this, SLOT(AlignDesktopPlugins()) );
    deskMenu->addAction(LXDG::findIcon("shapes",""), tr("Tile Plugins"), bgDesktop, SLOT(tileSubWindows()) );
    deskMenu->addAction(LXDG::findIcon("window-duplicate",""), tr("Cascade Plugins"), bgDesktop, SLOT(cascadeSubWindows()) );
  }else{ deskMenu->addAction(LXDG::findIcon("document-decrypt",""),tr("Unlock Desktop"), this, SLOT(ToggleDesktopLock()) ); }
  deskMenu->addSeparator();
  deskMenu->addAction(LXDG::findIcon("system-log-out",""), tr("Log Out"), this, SLOT(SystemLogout()) );
}

void LDesktop::UpdateWinMenu(){
  winMenu->clear();
  //Get the current list of windows
  QList<WId> wins = LX11::WindowList();	
  //Now add them to the menu
  for(int i=0; i<wins.length(); i++){
    LWinInfo info(wins[i]);
    bool junk;
    QAction *act = winMenu->addAction( info.icon(junk), info.text() );
      act->setData( QString::number(wins[i]) );
  }
}

void LDesktop::winClicked(QAction* act){
  LX11::ActivateWindow( act->data().toString().toULong() );	
}

void LDesktop::UpdateDesktop(){
  if(DEBUG){ qDebug() << " - Update Desktop Plugins for screen:" << desktopnumber; }
  QStringList plugins = settings->value(DPREFIX+"pluginlist", QStringList()).toStringList();
  if(defaultdesktop && plugins.isEmpty()){
    //plugins << "sample" << "sample" << "sample";
  }
  bool changed=false; //in case the plugin list needs to be changed
  //First make sure all the plugin names are unique
  for(int i=0; i<plugins.length(); i++){
	if(!plugins[i].contains("---") ){
	  int num=1;
	  while( plugins.contains(plugins[i]+"---"+QString::number(desktopnumber)+"."+QString::number(num)) ){
	    num++;
	  }
	  plugins[i] = plugins[i]+"---"+QString::number(desktopnumber)+"."+QString::number(num);
	  changed=true;
	}
  }
  if(changed){
    //save the modified plugin list to file (so per-plugin settings are preserved)
    issyncing=true; //don't let the change cause a refresh
    settings->setValue(DPREFIX+"pluginlist", plugins);
    settings->sync();
    QTimer::singleShot(200, this, SLOT(UnlockSettings()) );
  }
  //If generating desktop file launchers, add those in
  if(settings->value(DPREFIX+"generateDesktopIcons",false).toBool()){
    QFileInfoList files = LSession::handle()->DesktopFiles();
    for(int i=0; i<files.length(); i++){
      plugins << "applauncher::"+files[i].absoluteFilePath()+"---"+DPREFIX;
    }
  }
  //Go through the plugins and remove any existing ones that do not show up on the current list

  for(int i=0; i<PLUGINS.length(); i++){
    if(!plugins.contains(PLUGINS[i]->ID())){
      //Remove this plugin (with settings) - is not currently listed
      DesktopPluginRemoved(PLUGINS[i]->ID(),true); //flag this as an internal removal
      i--;
    }
  }
  //Now get an accounting of all the available/used space
  QRegion avail;//Note that this is child-geometry space
  if(!bgDesktop->isVisible()){ avail = QRegion( QRect(QPoint(0,0),desktop->screenGeometry(desktopnumber).size()) ); }
  else{ avail = QRegion(QRect(QPoint(0,0),bgDesktop->size()) ); qDebug() << "Desktop size:" << bgDesktop->size(); } 
  //qDebug() << "Available Screen Geom:" << avail.boundingRect();
  //avail = avail.subtracted( bgDesktop->childrenRegion() ); //
  /*QList<QMdiSubWindow*> wins = bgDesktop->subWindowList();
  for(int i=0; i<wins.length(); i++){
    QRect geom = wins[i]->geometry()
    if(avail.contains(wins[i]->geometry())){ avail = avail.subtracted( QRegion(wins[i]->geometry()) ); }
  }*/
  //qDebug() << " - after removals:" << avail.boundingRect();
  //Now add/update plugins
  for(int i=0; i<plugins.length(); i++){
    //See if this plugin is already there
    LDPlugin *plug = 0;
    for(int p=0; p<PLUGINS.length(); p++){
      //qDebug() << " -- Existing Plugin:" << PLUGINS[p]->ID() << p << PLUGINS.length();
      if(PLUGINS[p]->ID()==plugins[i]){
	//qDebug() << "  -- Found Plugin";
	plug = PLUGINS[p];
	break;
      }
    }
    if(plug==0){
      //New Plugin
      if(DEBUG){qDebug() << " -- New Plugin:" << plugins[i];}
      plug = NewDP::createPlugin(plugins[i], bgDesktop);
      if(plug != 0){
	connect(plug, SIGNAL(OpenDesktopMenu()), this, SLOT(ShowMenu()) );
	if(DEBUG){ qDebug() << " --- Show Plugin"; }
	PLUGINS << plug;
	QApplication::processEvents(); //need a moment between plugin/container creation
	QRegion tmpavail = avail - bgDesktop->childrenRegion(); //currently available space right now
	LDPluginContainer *cont = CreateDesktopPluginContainer(plug);
	cont->show();
	QApplication::processEvents();
	if(!cont->hasFixedPosition()){
	  //Need to arrange the location of the plugin (leave size alone)
	  if(DEBUG){ qDebug() << " ---  Floating Plugin - find a spot for it"; }
	  QPoint pt = findNewPluginLocation(tmpavail, cont->size());
	  if(pt.x()>=0 && pt.y()>=0){
	    cont->saveNewPosition(pt); 
	    QTimer::singleShot(500, cont, SLOT(loadInitialPosition()) ); //re-load geometry in a moment
	    if(DEBUG){ qDebug() << " --- Moving to point:" << pt; }
	  }
	}
	//Done with this plugin - removed it's area from the available space
	if(DEBUG){ qDebug() << " ---  Done Creating Plugin Container" << cont->geometry(); }
	//avail = avail.subtracted( QRegion(cont->geometry()) );
	
      }
    }
    QApplication::processEvents(); //need to process events between loading of plugins
  }
}

void LDesktop::ToggleDesktopLock(){
  desktoplocked = !desktoplocked; //flip to other value
  //Remove all the current containers
  QList<QMdiSubWindow*> wins = bgDesktop->subWindowList();
  for(int i=0; i<wins.length(); i++){
    wins[i]->setWhatsThis(""); //clear this so it knows it is being temporarily removed
    bgDesktop->removeSubWindow(wins[i]->widget()); //unhook plugin from container
    bgDesktop->removeSubWindow(wins[i]); //remove container from screen
    delete wins[i]; //delete old container
  }
  //Now recreate all the containers on the screen
  for(int i=0; i<PLUGINS.length(); i++){
    CreateDesktopPluginContainer(PLUGINS[i]);
  }
  bgDesktop->update(); //refresh visuals
  UpdateMenu(false);
}

void LDesktop::AlignDesktopPlugins(){
  QList<QMdiSubWindow*> wins = bgDesktop->subWindowList();
  QSize fit = bgDesktop->size();
  //Auto-determine the best grid sizing
    // It will try to exactly fit the desktop plugin area, with at least 10-20 grid points
  int xgrid, ygrid;
	xgrid = ygrid = 32;
	//while(fit.width()%xgrid != 0){ xgrid = xgrid-1; }
	//while(fit.height()%ygrid != 0){ ygrid = ygrid-1; }
  //qDebug() << "Grid:" << xgrid << ygrid << fit.width() << fit.height();
  //Make sure there are at least 10 points. It will not fit the area exactly, but should be very close
  //while(xgrid < 10){ xgrid = xgrid*2; }
  //while(ygrid < 10){ ygrid = ygrid*2; }
  //qDebug() << "Grid (adjusted):" << xgrid << ygrid;
 // xgrid = int(fit.width()/xgrid); //now get the exact pixel size of the grid
  //ygrid = int(fit.height()/ygrid); //now get the exact pixel size of the grid
  //qDebug() << "Grid (pixel):" << xgrid << ygrid;
  //qDebug() << "  X-Grid:" << xgrid << "("+QString::number(fit.width()/xgrid)+" points)";
  //qDebug() << "  Y-Grid:" << ygrid << "("+QString::number(fit.height()/ygrid)+" points)";
  for(int i=0; i<wins.length(); i++){
    //align the plugin on a grid point (that is not right/bottom edge)
    QRect geom = wins[i]->geometry();
        int x, y;
        if(geom.x()<0){ x=0; }
	else{ x = qRound(geom.x()/float(xgrid)) * xgrid; }
	if(x>= fit.width()){ x = fit.width()-xgrid; geom.setWidth(xgrid); }
	if(geom.y()<0){ y=0; }
	else{ y = qRound(geom.y()/float(ygrid)) * ygrid; }
	if(y>= fit.height()){ y = fit.height()-ygrid; geom.setHeight(ygrid); }
	geom.moveTo(x,y);
    //Now adjust the size to also be the appropriate grid multiple
	geom.setWidth( qRound(geom.width()/float(xgrid))*xgrid );
	geom.setHeight( qRound(geom.height()/float(ygrid))*ygrid );

    //Now check for edge spillover and adjust accordingly
	int diff = (geom.x()+geom.width()) - bgDesktop->size().width();
	if( diff > 0 ){ geom.moveTo( geom.x() - diff, geom.y() ); }
	else if( diff > -11 ){ geom.setWidth( geom.width()-diff ); }
	diff = (geom.y()+geom.height()) - bgDesktop->size().height();
	if( diff > 0 ){ geom.moveTo( geom.x(), geom.y() - diff ); }
	else if( diff > -11 ){ geom.setHeight( geom.height()-diff ); }
    //Now move the plugin
	wins[i]->setGeometry(geom);
  }
}

void LDesktop::DesktopPluginRemoved(QString ID, bool internal){
  //Close down that plugin instance (NOTE: the container might have already closed by the user)
  if(DEBUG){ qDebug() << "Desktop Plugin Removed:" << ID; }
  //First look for the container (just in case)
  QList<QMdiSubWindow*> wins = bgDesktop->subWindowList();
  for(int i=0; i<wins.length(); i++){
    if(wins[i]->whatsThis() == ID){
      if(DEBUG){ qDebug() << " - Removing Plugin Container"; }
      //wins[i]->setWhatsThis(""); //clear this so it knows it is being temporarily removed
      bgDesktop->removeSubWindow(wins[i]->widget()); //unhook plugin from container
      bgDesktop->removeSubWindow(wins[i]); //remove container from screen
      if(internal){ delete wins[i]; }//delete old container
      break;
    }
  }
	
  //qDebug() << "PLUGINS:" << PLUGINS.length() << ID;
  for(int i=0; i<PLUGINS.length(); i++){
    if(PLUGINS[i]->ID() == ID){
      //qDebug() << "- found ID";
      if(DEBUG){ qDebug() << " - Deleting Desktop Plugin:" << ID; }
      PLUGINS[i]->removeSettings(); //Remove any settings associated with this plugin
      delete PLUGINS.takeAt(i);
      break;
    }
  }
  
  //Now remove that plugin from the internal list (then let the plugin system remove the actual plugin)
  QStringList plugins = settings->value(DPREFIX+"pluginlist",QStringList()).toStringList();
  if(DEBUG){ qDebug() << " - Also removing plugin from future list"; }
  if(plugins.removeAll(ID) > 0){
    issyncing = true;
    if(DEBUG){ qDebug() << " - Save modified plugins list"; }
    settings->setValue(DPREFIX+"pluginlist", plugins);
    if(DEBUG){ qDebug() << " - Unlock settings file in 200 ms"; }
    QTimer::singleShot(200, this, SLOT(UnlockSettings()) );
  }
    /*if(QFile::exists(QDir::homePath()+"/.lumina/desktop-plugins/"+ID+".conf")){
      if(DEBUG){ qDebug() << " - Removing settings file"; }
      QFile::remove(QDir::homePath()+"/.lumina/desktop-plugins/"+ID+".conf");
    }*/
  if(DEBUG){ qDebug() << " - Done removing plugin"; }
}

void LDesktop::UpdatePanels(){
  if(DEBUG){ qDebug() << " - Update Panels For Screen:" << desktopnumber; }
  int panels = settings->value(DPREFIX+"panels", -1).toInt();
  //if(panels==-1 && defaultdesktop){ panels=1; } //need at least 1 panel on the primary desktop
  //Remove all extra panels
  for(int i=0; i<PANELS.length(); i++){
    if(panels <= PANELS[i]->number()){
      if(DEBUG){ qDebug() << " -- Remove Panel:" << PANELS[i]->number(); }
      PANELS[i]->prepareToClose();
      delete PANELS.takeAt(i);
      i--;
    }
  }
  for(int i=0; i<panels; i++){
    //Check for a panel with this number
    bool found = false;
    for(int p=0; p<PANELS.length() && !found; p++){
      if(PANELS[p]->number() == i){
        found = true;
	if(DEBUG){ qDebug() << " -- Update panel "<< i; }
        //panel already exists - just update it
        QTimer::singleShot(0, PANELS[p], SLOT(UpdatePanel()) );
      }
    }
    if(!found){
      if(DEBUG){ qDebug() << " -- Create panel "<< i; }
      //New panel
      LPanel *pan = new LPanel(settings, desktopnumber, i, bgWindow);
      PANELS << pan;
      pan->show();
    }
  }
  //Give it a 1/2 second before ensuring that the visible desktop area is correct
  QTimer::singleShot(500, this, SLOT(UpdateDesktopPluginArea()) );
}

void LDesktop::UpdateDesktopPluginArea(){
  QRegion visReg( bgWindow->geometry() ); //visible region (not hidden behind a panel)
  QRect rawRect = visReg.boundingRect(); //initial value (screen size)
  for(int i=0; i<PANELS.length(); i++){
    QRegion shifted = visReg;
    QString loc = settings->value(PANELS[i]->prefix()+"location","top").toString().toLower();
    int vis = PANELS[i]->visibleWidth();
    if(loc=="top"){ 
      if(!shifted.contains(QRect(rawRect.x(), rawRect.y(), rawRect.width(), vis))){ continue; }
      shifted.translate(0, (rawRect.top()+vis)-shifted.boundingRect().top() ); 
    }else if(loc=="bottom"){
      if(!shifted.contains(QRect(rawRect.x(), rawRect.bottom()-vis, rawRect.width(), vis))){ continue; }	    
      shifted.translate(0, (rawRect.bottom()-vis)-shifted.boundingRect().bottom()); 
    }else if(loc=="left"){ 
      if( !shifted.contains(QRect(rawRect.x(), rawRect.y(), vis,rawRect.height())) ){ continue; }
      shifted.translate((rawRect.left()+vis)-shifted.boundingRect().left() ,0); 
    }else{  //right
      if(!shifted.contains(QRect(rawRect.right()-vis, rawRect.y(), vis,rawRect.height())) ){ continue; }
      shifted.translate((rawRect.right()-vis)-shifted.boundingRect().right(),0); 
    }
    visReg = visReg.intersected( shifted );
  }
  //Now make sure the desktop plugin area is only the visible area
  QRect rec = visReg.boundingRect();
  //LSession::handle()->XCB->SetScreenWorkArea((unsigned int) desktopnumber, rec);
  //Now remove the X offset to place it on the current screen (needs widget-coords, not global)
  globalWorkRect = rec; //save this for later
  rec.moveTopLeft( QPoint( rec.x()-desktop->screenGeometry(desktopnumber).x() , rec.y() ) );
  //qDebug() << "DPlug Area:" << rec.x() << rec.y() << rec.width() << rec.height();
  if(rec == bgDesktop->geometry()){return; } //nothing changed
  bgDesktop->setGeometry( rec );
  bgDesktop->setBackground( QBrush(Qt::NoBrush) );
  bgDesktop->update();
  //Re-paint the panels (just in case a plugin was underneath it and the panel is transparent)
  for(int i=0; i<PANELS.length(); i++){ PANELS[i]->update(); }
  //Also need to re-arrange any desktop plugins to ensure that nothing is out of the screen area
  AlignDesktopPlugins();
}

void LDesktop::UpdateBackground(){
  //Get the current Background
  if(bgupdating || bgWindow==0){ return; } //prevent multiple calls to this at the same time
  bgupdating = true;
  if(DEBUG){ qDebug() << " - Update Desktop Background for screen:" << desktopnumber; }
  //Get the list of background(s) to show
  QStringList bgL = settings->value(DPREFIX+"background/filelist", QStringList()).toStringList();
  //qDebug() << " - List:" << bgL << CBG;
    //Remove any invalid files
    for(int i=0; i<bgL.length(); i++){
      if( (!QFile::exists(bgL[i]) && bgL[i]!="default" && !bgL[i].startsWith("rgb(") ) || bgL[i].isEmpty()){ bgL.removeAt(i); i--; }
    }
    if(bgL.isEmpty()){ bgL << "default"; } //always fall back on the default
  //Determine if the background needs to be changed
  //qDebug() << "BG List:" << bgL << oldBGL << CBG << bgtimer->isActive();
  if(bgL==oldBGL && !CBG.isEmpty() && bgtimer->isActive()){
    //No background change scheduled - just update the widget
    bgWindow->update();
    bgupdating=false;
    return;
  }
  oldBGL = bgL; //save this for later
  //Determine which background to use next
  int index;
  if(CBG.isEmpty()){ index = ( qrand() % bgL.length() ); } //random first wallpaper
  else{ 
    //Go to the next in the list
    index = bgL.indexOf(CBG); 
    if(index < 0 || index >= bgL.length()-1){ index = 0; } //if invalid or last item in the list - go to first
    else{ index++; } //go to next
  }
  QString bgFile = bgL[index];
  //Save this file as the current background
  CBG = bgFile;
  //qDebug() << " - Set Background to:" << CBG << index << bgL;
  if( (bgFile.toLower()=="default")){ bgFile = LOS::LuminaShare()+"desktop-background.jpg"; }
  //Now set this file as the current background
  QString style;
  if(bgFile.startsWith("rgb(")){
    //qDebug() << "Set background color:" << bgFile;
    style = "QWidget#bgWindow{ border-image: none; background-color: %1;}";
  }else{
    style = "QWidget#bgWindow{ background-color: black; border-image:url(%1) stretch;}";
  }
  style = style.arg(bgFile);
  bgWindow->setStyleSheet(style);
  bgWindow->show();
  //Now reset the timer for the next change (if appropriate)
  if(bgtimer->isActive()){ bgtimer->stop(); }
  if(bgL.length() > 1){
    //get the length of the timer (in minutes)
    int min = settings->value(DPREFIX+"background/minutesToChange",5).toInt();
    //restart the internal timer
    if(min > 0){
      bgtimer->start(min*60000); //convert from minutes to milliseconds
    }
  }
  //Now update the panel backgrounds
  for(int i=0; i<PANELS.length(); i++){
    PANELS[i]->update();
    PANELS[i]->show();
  }
  bgupdating=false;
}
