//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012-2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LDesktop.h"
#include "LSession.h"

LDesktop::LDesktop(int deskNum) : QObject(){
	
  DPREFIX = "desktop-"+QString::number(deskNum)+"/";
  desktopnumber = deskNum;
  desktop = QApplication::desktop();
    connect(desktop, SIGNAL(resized(int)), this, SLOT(UpdateGeometry(int)));
  defaultdesktop = (desktop->screenGeometry(desktopnumber).x()==0);
  desktoplocked = true;
  issyncing = false;
  qDebug() << "Desktop #"<<deskNum<<" -> "<< desktop->screenGeometry(desktopnumber).x() << desktop->screenGeometry(desktopnumber).y() << desktop->screenGeometry(desktopnumber).width() << desktop->screenGeometry(desktopnumber).height();
  deskMenu = new QMenu(0);
    connect(deskMenu, SIGNAL(triggered(QAction*)), this, SLOT(SystemApplication(QAction*)) );
  appmenu = new AppMenu(0);
  workspacelabel = new QLabel(0);
    workspacelabel->setAlignment(Qt::AlignCenter);
  wkspaceact = new QWidgetAction(0);
    wkspaceact->setDefaultWidget(workspacelabel);
  //Setup the internal variables
  settings = new QSettings(QSettings::UserScope, "LuminaDE","desktopsettings", this);
  //qDebug() << " - Desktop Settings File:" << settings->fileName();
  if(!QFile::exists(settings->fileName())){ settings->setValue(DPREFIX+"background/filelist",QStringList()<<"default"); settings->sync(); }
  bgtimer = new QTimer(this);
    bgtimer->setSingleShot(true);
    connect(bgtimer, SIGNAL(timeout()), this, SLOT(UpdateBackground()) );
  watcher = new QFileSystemWatcher(this);
    //connect(LSession::instance(), SIGNAL(DesktopConfigChanged()), this, SLOT(SettingsChanged()) );
    watcher->addPath(settings->fileName());
    connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(SettingsChanged()) );
 
  bgWindow = new QWidget(0);
	bgWindow->setObjectName("bgWindow");
	bgWindow->setContextMenuPolicy(Qt::CustomContextMenu);
	LX11::SetAsDesktop(bgWindow->winId());
	bgWindow->setGeometry(desktop->screenGeometry(desktopnumber));
	connect(bgWindow, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowMenu()) );
  bgDesktop = new QMdiArea(bgWindow);
	//Make sure the desktop area is transparent to show the background
        bgDesktop->setBackground( QBrush(Qt::NoBrush) );
  
  //Start the update processes
  QTimer::singleShot(1,this, SLOT(UpdateMenu()) );
  QTimer::singleShot(1,this, SLOT(UpdateBackground()) );
  QTimer::singleShot(1,this, SLOT(UpdateDesktop()) );
  QTimer::singleShot(10,this, SLOT(UpdatePanels()) );

}

LDesktop::~LDesktop(){
  delete deskMenu;
  delete appmenu;
  delete bgWindow;
  delete workspacelabel;
  delete wkspaceact;
}

int LDesktop::Screen(){
  return desktopnumber;	
}

void LDesktop::show(){
  bgWindow->show();
  bgDesktop->show();
  for(int i=0; i<PANELS.length(); i++){ PANELS[i]->show(); }
}

void LDesktop::hide(){
  bgWindow->hide();
  bgDesktop->hide();
  for(int i=0; i<PANELS.length(); i++){ PANELS[i]->hide(); }	
}

void LDesktop::SystemLogout(){ 
  LSession::systemWindow(); 
}

void LDesktop::SystemTerminal(){ 
  QString term = settings->value("default-terminal","xterm").toString();
  LSession::LaunchApplication(term);
}

void LDesktop::SystemFileManager(){
  LSession::LaunchApplication("lumina-fm");	
}

void LDesktop::SystemApplication(QAction* act){
  if(!act->whatsThis().isEmpty() && act->parent()==deskMenu){
    LSession::LaunchApplication("lumina-open \""+act->whatsThis()+"\"");
  }
}

void LDesktop::CreateDesktopPluginContainer(LDPlugin *plug){
  //Verify that a container does not already exist for this plugin
  QList<QMdiSubWindow*> wins = bgDesktop->subWindowList();
  for(int i=0; i<wins.length(); i++){
    if(wins[i]->whatsThis()==plug->ID()){ return; }
  }
  //Create a new plugin container
  LDPluginContainer *win = new LDPluginContainer(plug, desktoplocked);
  if(desktoplocked){ bgDesktop->addSubWindow(win, Qt::FramelessWindowHint); }
  else{ bgDesktop->addSubWindow(win, Qt::CustomizeWindowHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint); }
  win->loadInitialPosition();
  win->show();
  win->update();
  connect(win, SIGNAL(PluginRemoved(QString)), this, SLOT(DesktopPluginRemoved(QString)) );
}

// =====================
//     PRIVATE SLOTS 
// =====================
void LDesktop::SettingsChanged(){
  if(changingsettings || issyncing){ return; } //don't refresh for internal modifications to the fil
  issyncing = true;
  qDebug() << "Found Settings Change:" << desktopnumber;
  settings->sync(); //make sure to sync with external settings changes
  UpdateBackground();
  UpdateDesktop();
  UpdatePanels();
  UpdateMenu();
  QTimer::singleShot(200, this, SLOT(UnlockSettings()) ); //give it a few moments to settle before performing another sync
}

void LDesktop::UpdateMenu(bool fast){
  //qDebug() << " - Update Menu:" << desktopnumber;
  //Put a label at the top 
  int num = LX11::GetCurrentDesktop();
  //qDebug() << "Found desktop number:" << num;
  if(num < 0){ workspacelabel->setText( "<b>"+tr("Lumina Desktop")+"</b>"); }
  else{ workspacelabel->setText( "<b>"+QString(tr("Workspace %1")).arg(QString::number(num+1))+"</b>"); }
  if(fast){ return; } //already done
  deskMenu->clear(); //clear it for refresh
  deskMenu->addAction(wkspaceact);
  deskMenu->addSeparator();
  //Now load the user's menu setup and fill the menu
  QStringList items = settings->value("menu/itemlist", QStringList()<< "terminal" << "filemanager" <<"applications" << "line" << "settings" ).toStringList();
  for(int i=0; i<items.length(); i++){
    if(items[i]=="terminal"){ deskMenu->addAction(LXDG::findIcon("utilities-terminal",""), tr("Terminal"), this, SLOT(SystemTerminal()) ); }
    else if(items[i]=="filemanager"){ deskMenu->addAction( LXDG::findIcon("Insight-FileManager",""), tr("Browse System"), this, SLOT(SystemFileManager()) ); }
    else if(items[i]=="applications"){ deskMenu->addMenu( LSession::applicationMenu() ); }
    else if(items[i]=="line"){ deskMenu->addSeparator(); }
    else if(items[i]=="settings"){ deskMenu->addMenu( LSession::settingsMenu() ); }
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
  }else{ deskMenu->addAction(LXDG::findIcon("document-decrypt",""),tr("Unlock Desktop"), this, SLOT(ToggleDesktopLock()) ); }
  deskMenu->addSeparator();
  deskMenu->addAction(LXDG::findIcon("system-log-out",""), tr("Log Out"), this, SLOT(SystemLogout()) );
}

void LDesktop::UpdateDesktop(){
  qDebug() << " - Update Desktop Plugins for screen:" << desktopnumber;
  static bool loading = false;
  if(loading){ return; } //make sure to only run this once
  loading = true;
  QStringList plugins = settings->value(DPREFIX+"pluginlist", QStringList()).toStringList();
  if(defaultdesktop && plugins.isEmpty()){
    //plugins << "sample" << "sample" << "sample";
  }
  bool changed=false; //in case the plugin list needs to be changed
  //Go through the plugins and remove any existing ones that do not show up on the current list
  for(int i=0; i<PLUGINS.length(); i++){
    if(!plugins.contains(PLUGINS[i]->ID())){
      //Remove this plugin (with settings) - is not currently listed
      DesktopPluginRemoved(PLUGINS[i]->ID());
      i--;
    }
  }
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
        //Make sure the plugin ID is unique
	if(!plugins[i].contains("---") ){
	  int num=1;
	  while( plugins.contains(plugins[i]+"---"+QString::number(desktopnumber)+"."+QString::number(num)) ){
	    num++;
	  }
	  plugins[i] = plugins[i]+"---"+QString::number(desktopnumber)+"."+QString::number(num);
	  changed=true;
	}
      //Now create the plugin (will load existing settings if possible)
      qDebug() << " -- New Plugin:" << plugins[i];
      plug = NewDP::createPlugin(plugins[i], bgDesktop);
      if(plug != 0){ 
	//qDebug() << " -- Show Plugin";
	PLUGINS << plug;
	CreateDesktopPluginContainer(plug);
      }
    }
 
  }
  if(changed){
    //save the modified plugin list to file (so per-plugin settings are preserved)
    changingsettings=true; //don't let the change cause a refresh
    settings->setValue(DPREFIX+"pluginlist", plugins);
    settings->sync();
    changingsettings=false; //finished changing setting
  }
  loading = false;
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

void LDesktop::DesktopPluginRemoved(QString ID){
  //Close down that plugin instance
  //qDebug() << "PLUGINS:" << PLUGINS.length() << ID;
  for(int i=0; i<PLUGINS.length(); i++){
    if(PLUGINS[i]->ID() == ID){
      //qDebug() << "- found ID";
      //Delete the plugin container first
      QList<QMdiSubWindow*> wins = bgDesktop->subWindowList();
      for(int i=0; i<wins.length(); i++){
	if(wins[i]->whatsThis()==ID || wins[i]->whatsThis().isEmpty()){
          //wins[i]->setWhatsThis(""); //clear this so it knows it is being temporarily removed
          bgDesktop->removeSubWindow(wins[i]->widget()); //unhook plugin from container
          bgDesktop->removeSubWindow(wins[i]); //remove container from screen
          delete wins[i]; //delete old container
	}
      }
      //Now delete the plugin itself
      delete PLUGINS.takeAt(i);
      break;
    }
  }
  //Now remove that plugin from the internal list
  QStringList plugins = settings->value(DPREFIX+"pluginlist",QStringList()).toStringList();
  
  plugins.removeAll(ID);
  changingsettings=true; //don't let the change cause a refresh
    settings->setValue(DPREFIX+"pluginlist", plugins);
    settings->sync();
  changingsettings=false; //finished changing setting
}

void LDesktop::UpdatePanels(){
  qDebug() << " - Update Panels For Screen:" << desktopnumber;
  int panels = settings->value(DPREFIX+"panels", -1).toInt();
  if(panels==-1 && defaultdesktop){ panels=1; } //need at least 1 panel on the primary desktop
  //Remove all extra panels
  for(int i=0; i<PANELS.length(); i++){
    if(panels <= PANELS[i]->number()){
      delete PANELS.takeAt(i);
      i--;
    }
  }
  for(int i=0; i<panels; i++){
    //Check for a panel with this number
    bool found = false;
    for(int p=0; p<PANELS.length(); p++){
      if(PANELS[p]->number() == i){
        found = true;
	//qDebug() << " -- Update panel "<< i;
        //panel already exists - just update it
        QTimer::singleShot(0, PANELS[i], SLOT(UpdatePanel()) );
      }
    }
    if(!found){
      qDebug() << " -- Create panel "<< i;
      //New panel
      PANELS << new LPanel(settings, desktopnumber, i, bgWindow);
    }
  }
  //Give it a 1/2 second before ensuring that the visible desktop area is correct
  QTimer::singleShot(500, this, SLOT(UpdateDesktopPluginArea()) );
}

void LDesktop::UpdateDesktopPluginArea(){
  QRegion visReg( bgWindow->geometry() ); //visible region (not hidden behind a panel)
  for(int i=0; i<PANELS.length(); i++){
    visReg = visReg.subtracted( QRegion(PANELS[i]->geometry()) );
  }
  //Now make sure the desktop plugin area is only the visible area
  QRect rec = visReg.boundingRect();
  //Now remove the X offset to place it on the current screen (needs widget-coords, not global)
  rec.moveTopLeft( QPoint( rec.x()-desktop->screenGeometry(desktopnumber).x() , rec.y() ) );
  //qDebug() << "DPlug Area:" << rec.x() << rec.y() << rec.width() << rec.height();
  bgDesktop->setGeometry( rec );
  bgDesktop->setBackground( QBrush(Qt::NoBrush) );
  bgDesktop->update();
  //Re-paint the panels (just in case a plugin was underneath it and the panel is transparent)
  for(int i=0; i<PANELS.length(); i++){ PANELS[i]->update(); }
}
 
void LDesktop::UpdateBackground(){
  //Get the current Background
  static bool bgupdating = false;
  if(bgupdating){ return; } //prevent multiple calls to this at the same time
  bgupdating = true;
  qDebug() << " - Update Desktop Background for screen:" << desktopnumber;
  //Get the list of background(s) to show
  QStringList bgL = settings->value(DPREFIX+"background/filelist", QStringList()).toStringList();
  //qDebug() << " - List:" << bgL << CBG;
    //Remove any invalid files
    for(int i=0; i<bgL.length(); i++){ 
      if( (!QFile::exists(bgL[i]) && bgL[i]!="default") || bgL[i].isEmpty()){ bgL.removeAt(i); i--; } 
    }
  //Determine which background to use next
  int index = bgL.indexOf(CBG);
  if( (index < 0) || (index >= bgL.length()-1) ){ index = 0; } //use the first file
  else{ index++; } //use the next file in the list
  QString bgFile;
  if( bgL.isEmpty() && CBG.isEmpty()){ bgFile = "default"; }
  else if( bgL.isEmpty() && QFile::exists(CBG) ){ bgFile = CBG; }
  else if( bgL.isEmpty() ){ bgFile = "default"; }
  else{ bgFile = bgL[index]; }
  //Save this file as the current background
  CBG = bgFile;
  //qDebug() << " - Set Background to:" << CBG << index << bgL;
  if( (bgFile.toLower()=="default")){ bgFile = "/usr/local/share/Lumina-DE/desktop-background.jpg"; }
  //Now set this file as the current background
  QString style = "QWidget#bgWindow{ border-image:url(%1) stretch;}";
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
  }
  bgupdating=false;
}
