//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012-2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LSession.h"
#include <LuminaOS.h>

#include <QTime>
#include "LXcbEventFilter.h"

//LibLumina X11 class
#include <LuminaX11.h>

//X includes (these need to be last due to Qt compile issues)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xdamage.h>

#ifndef DEBUG
#define DEBUG 1
#endif

XCBEventFilter *evFilter = 0;

LSession::LSession(int &argc, char ** argv) : QApplication(argc, argv){
  this->setApplicationName("Lumina Desktop Environment");
  this->setApplicationVersion("0.7.2");
  this->setOrganizationName("LuminaDesktopEnvironment");
  this->setQuitOnLastWindowClosed(false); //since the LDesktop's are not necessarily "window"s
  //Enabled a few of the simple effects by default
  this->setEffectEnabled( Qt::UI_AnimateMenu, true);
  this->setEffectEnabled( Qt::UI_AnimateCombo, true);
  this->setEffectEnabled( Qt::UI_AnimateTooltip, true);
  //this->setStyle( new MenuProxyStyle); //QMenu icon size override
  SystemTrayID = 0; VisualTrayID = 0;
  TrayDmgEvent = 0;
  TrayDmgError = 0;
  XCB = 0;
  //initialize the empty internal pointers to 0
  appmenu = 0;
  settingsmenu = 0;
  currTranslator=0;
  mediaObj=0;
  //audioOut=0;
  audioThread=0;
  sessionsettings=0;
  //Setup the event filter for Qt5
  evFilter =  new XCBEventFilter(this);
  this->installNativeEventFilter( evFilter );
}

LSession::~LSession(){
  WM->stopWM();
  for(int i=0; i<DESKTOPS.length(); i++){
    delete DESKTOPS[i];
  }
  delete WM;
  delete settingsmenu;
  delete appmenu;
  delete currTranslator;
  if(mediaObj!=0){delete mediaObj;}
  //if(audioOut!=0){delete audioOut; }
}

void LSession::setupSession(){
  qDebug() << "Initializing Session";
  QTime* timer = 0;
  if(DEBUG){ timer = new QTime(); timer->start(); qDebug() << " - Init srand:" << timer->elapsed();}
  //Initialize the XCB interface backend
  XCB = new LXCB();
  //Seed random number generator (if needed)
  qsrand( QTime::currentTime().msec() );
  //Setup the QSettings default paths
  if(DEBUG){ qDebug() << " - Init QSettings:" << timer->elapsed();}
  QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, QDir::homePath()+"/.lumina");
  sessionsettings = new QSettings("LuminaDE", "sessionsettings");
  //Setup the user's lumina settings directory as necessary
  if(DEBUG){ qDebug() << " - Init User Files:" << timer->elapsed();}  
  checkUserFiles(); //adds these files to the watcher as well

  //Initialize the internal variables
  DESKTOPS.clear();
	
  //Start the background system tray
  if(DEBUG){ qDebug() << " - Init System Tray:" << timer->elapsed();}
  startSystemTray();
	
  //Launch Fluxbox
  qDebug() << " - Launching Fluxbox";
  if(DEBUG){ qDebug() << " - Init WM:" << timer->elapsed();}
  WM = new WMProcess();
    WM->startWM();
	
  //Initialize the desktops
  if(DEBUG){ qDebug() << " - Init Desktops:" << timer->elapsed();}
  updateDesktops();

  //Initialize the global menus
  qDebug() << " - Initialize system menus";
  if(DEBUG){ qDebug() << " - Init AppMenu:" << timer->elapsed();}
  appmenu = new AppMenu();
  if(DEBUG){ qDebug() << " - Init SettingsMenu:" << timer->elapsed();}
  settingsmenu = new SettingsMenu();

  //Now setup the system watcher for changes
  qDebug() << " - Initialize file system watcher";
  if(DEBUG){ qDebug() << " - Init QFileSystemWatcher:" << timer->elapsed();}
  watcher = new QFileSystemWatcher(this);
    //watcher->addPath( QDir::homePath()+"/.lumina/stylesheet.qss" );
    watcher->addPath( QDir::homePath()+"/.lumina/LuminaDE/desktopsettings.conf" );
    watcher->addPath( QDir::homePath()+"/.lumina/fluxbox-init" );
    watcher->addPath( QDir::homePath()+"/.lumina/fluxbox-keys" );

  //connect internal signals/slots
  connect(this->desktop(), SIGNAL(screenCountChanged(int)), this, SLOT(updateDesktops()) );
  connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(watcherChange(QString)) );
  connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(watcherChange(QString)) );
  connect(this, SIGNAL(aboutToQuit()), this, SLOT(SessionEnding()) );
  if(DEBUG){ qDebug() << " - Init Finished:" << timer->elapsed(); delete timer;}
}

/*bool LSession::LoadLocale(QString langCode){
    QTranslator translator;
    if ( ! QFile::exists(LOS::LuminaShare()+"i18n/lumina-desktop_" + langCode + ".qm" ) )  langCode.truncate(langCode.indexOf("_"));
    bool ok = translator.load( QString("lumina-desktop_") + langCode, LOS::LuminaShare()+"i18n/" );
    if(ok){
      //Remove any old translator
      if(currTranslator != 0){ this->removeTranslator(currTranslator); }
      //Insert the new translator
      currTranslator = &translator;
      this->installTranslator( currTranslator );
      qDebug() << "Loaded Locale:" << langCode;
    }else{
      qDebug() << "Invalid Locale:" << langCode;
    }
    emit LocaleChanged();
    return ok;
}*/

void LSession::launchStartupApps(){
  //First start any system-defined startups, then do user defined
  qDebug() << "Launching startup applications";
  for(int i=0; i<2; i++){
    QString startfile;
    if(i==0){startfile = LOS::LuminaShare()+"startapps"; }
    else{ startfile = QDir::homePath()+"/.lumina/startapps"; }
    if(!QFile::exists(startfile)){ continue; } //go to the next

    QFile file(startfile);
    if( file.open(QIODevice::ReadOnly | QIODevice::Text) ){
      QTextStream in(&file);
      while(!in.atEnd()){
        QString entry = in.readLine();
        if(entry.startsWith("#") || entry.isEmpty()){ continue; }
        //Might put other sanity checks here
	qDebug() << " - Starting Application:" << entry;
        LSession::LaunchApplication(entry);
      }
      file.close();
    }
  }
  //Now play the login music
  if(sessionsettings->value("PlayStartupAudio",true).toBool()){
    LSession::playAudioFile(LOS::LuminaShare()+"Login.ogg");
  }
  if(sessionsettings->value("EnableNumlock",true).toBool()){
    QProcess::startDetached("numlockx on");
  }
}

void LSession::watcherChange(QString changed){
  qDebug() << "Session Watcher Change:" << changed;
  if(changed.endsWith("fluxbox-init") || changed.endsWith("fluxbox-keys")){ refreshWindowManager(); }
  else{ emit DesktopConfigChanged(); }
}

void LSession::checkUserFiles(){
  //version conversion examples: [1.0.0 -> 100], [1.2.0 -> 120], [0.6.0 -> 60]
  int oldversion = sessionsettings->value("DesktopVersion",0).toString().remove(".").toInt();
  bool newversion =  ( oldversion < this->applicationVersion().remove(".").toInt() );

  //Check for the desktop settings file
  QString dset = QDir::homePath()+"/.lumina/LuminaDE/desktopsettings.conf";
  bool firstrun = false;
  if(!QFile::exists(dset) || oldversion < 50){
    if( oldversion < 50 ){ QFile::remove(dset); qDebug() << "Current desktop settings obsolete: Re-implementing defaults"; }
    else{ firstrun = true; }
    if(QFile::exists(LOS::LuminaShare()+"desktopsettings.conf")){
      if( QFile::copy(LOS::LuminaShare()+"desktopsettings.conf", dset) ){
        QFile::setPermissions(dset, QFile::ReadUser | QFile::WriteUser | QFile::ReadOwner | QFile::WriteOwner);
      }
    }
  }
  //Check for the default applications file for lumina-open
  dset = QDir::homePath()+"/.lumina/LuminaDE/lumina-open.conf";
  if(!QFile::exists(dset)){
    firstrun = true;
    if(QFile::exists(LOS::LuminaShare()+"defaultapps.conf")){
      if( QFile::copy(LOS::LuminaShare()+"defaultapps.conf", dset) ){
        QFile::setPermissions(dset, QFile::ReadUser | QFile::WriteUser | QFile::ReadOwner | QFile::WriteOwner);
      }
    }

  }
  //Check the fluxbox configuration files
  dset = QDir::homePath()+"/.lumina/";
  bool fluxcopy = false;
  if(!QFile::exists(dset+"fluxbox-init")){ fluxcopy=true; }
  else if(!QFile::exists(dset+"fluxbox-keys")){fluxcopy=true; }
  else if(oldversion < 60){ fluxcopy=true; qDebug() << "Current fluxbox settings obsolete: Re-implementing defaults"; }
  if(fluxcopy){
    qDebug() << "Copying default fluxbox configuration files";
    if(QFile::exists(dset+"fluxbox-init")){ QFile::remove(dset+"fluxbox-init"); }
    if(QFile::exists(dset+"fluxbox-keys")){ QFile::remove(dset+"fluxbox-keys"); }
    QFile::copy(LOS::LuminaShare()+"fluxbox-init-rc", dset+"fluxbox-init");
    QFile::copy(LOS::LuminaShare()+"fluxbox-keys", dset+"fluxbox-keys");
    QFile::setPermissions(dset+"fluxbox-init", QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::ReadOther | QFile::ReadGroup);
    QFile::setPermissions(dset+"fluxbox-keys", QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::ReadOther | QFile::ReadGroup);
  }

  if(firstrun){ qDebug() << "First time using Lumina!!"; }
  else if(newversion){
    qDebug() << "Updating session file to current version";
  }


  //Save the current version of the session to the settings file (for next time)
  sessionsettings->setValue("DesktopVersion", this->applicationVersion());
}

void LSession::refreshWindowManager(){
  WM->updateWM();
}

void LSession::updateDesktops(){
  //qDebug() << " - Update Desktops";
  QDesktopWidget *DW = this->desktop();
  bool firstrun = (DESKTOPS.length()==0);
    for(int i=0; i<DW->screenCount(); i++){
      bool found = false;
      for(int j=0; j<DESKTOPS.length() && !found; j++){
        if(DESKTOPS[j]->Screen()==i){ found = true; }
      }
      if(!found){
	//Start the desktop on the new screen
        qDebug() << " - Start desktop on screen:" << i;
        DESKTOPS << new LDesktop(i);
      }
    }
    //qDebug() << " - Done Starting Desktops";
    //Make sure all the background windows are registered on the system as virtual roots
    QTimer::singleShot(200,this, SLOT(registerDesktopWindows()));
    if(firstrun){ return; } //Done right here on first run
    //Now go through and make sure to delete any desktops for detached screens
    for(int i=0; i<DESKTOPS.length(); i++){
      if(DESKTOPS[i]->Screen() >= DW->screenCount()){
	qDebug() << " - Hide desktop on screen:" << DESKTOPS[i]->Screen();
        DESKTOPS[i]->hide();
      }else{
	qDebug() << " - Show desktop on screen:" << DESKTOPS[i]->Screen();
        DESKTOPS[i]->show();
      }
    }
    //qDebug() << " - Done Checking Desktops";
}

void LSession::registerDesktopWindows(){
  QList<WId> wins;
  for(int i=0; i<DESKTOPS.length(); i++){
    wins << DESKTOPS[i]->backgroundID();
  }
  XCB->RegisterVirtualRoots(wins);
}

void LSession::adjustWindowGeom(WId win){
  //Quick hack for making sure that new windows are not located underneath any panels
  // Get the window location
  QRect geom = XCB->WindowGeometry(win, true); //always include the frame if possible
  if(DEBUG){ qDebug() << "Adjust Window Geometry:" << XCB->WindowClass(win) << !geom.isNull(); }
  if(geom.isNull()){ return; } //Could not get geometry
  //Get the available geometry for the screen the window is on
  QRect desk;
  for(int i=0; i<DESKTOPS.length(); i++){
    if( this->desktop()->screenGeometry(DESKTOPS[i]->Screen()).contains(geom.topLeft()) ){
      //Window is on this screen
      if(DEBUG){ qDebug() << " - On Screen:" << DESKTOPS[i]->Screen(); }
      desk = DESKTOPS[i]->availableScreenGeom();
      break;
    }
  }
  //Adjust the window location if necessary
  if(!desk.contains(geom) ){
    if(DEBUG){
      qDebug() << "Desk:" << desk.x() << desk.y() << desk.width() << desk.height();
      qDebug() << "Geom:" << geom.x() << geom.y() << geom.width() << geom.height();
    }
    //Adjust origin point for left/top margins
    if(geom.y() < desk.y()){ geom.moveTop(desk.y()); } //move down to the edge (top panel)
    if(geom.x() < desk.x()){ geom.moveLeft(desk.x()); } //move right to the edge (left panel)
    //Adjust size for right/bottom margins (within reason, since window titles are on top normally)
    if(geom.right() > desk.right() && (geom.width() > 100)){ geom.setRight(desk.right()); }
    if(geom.bottom() > desk.bottom() && geom.height() > 100){ geom.setBottom(desk.bottom()); }
    //Now move/resize the window
    if(DEBUG){ qDebug() << "New Geom:" << geom.x() << geom.y() << geom.width() << geom.height(); }
    XCB->MoveResizeWindow(win, geom);
  }
  
}

void LSession::SessionEnding(){
  audioThread->wait(3000); //wait a max of 3 seconds for the audio thread to finish
  stopSystemTray();
}

//===============
//  SYSTEM ACCESS
//===============
void LSession::LaunchApplication(QString cmd){
  LSession::setOverrideCursor(QCursor(Qt::BusyCursor));
  QProcess::startDetached(cmd);
}

AppMenu* LSession::applicationMenu(){
  return appmenu;
}

SettingsMenu* LSession::settingsMenu(){
  return settingsmenu;
}

QSettings* LSession::sessionSettings(){
  return sessionsettings;
}

void LSession::systemWindow(){
  SystemWindow win;
  win.exec();
  LSession::processEvents();
}

//Play System Audio
void LSession::playAudioFile(QString filepath){
  //Setup the audio output systems for the desktop
  //return; //Disable this for now: too many issues with Phonon at the moment (hangs the session)
  bool init = false;
  if(DEBUG){ qDebug() << "Play Audio File"; }
  if(audioThread==0){   qDebug() << " - Initialize audio systems"; audioThread = new QThread(); init = true; }
  //if(mediaObj==0){   qDebug() << " - Initialize Phonon media Object"; mediaObj = new Phonon::MediaObject(); init = true;}
  if(mediaObj==0){   qDebug() << " - Initialize media player"; mediaObj = new QMediaPlayer(); init = true;}
  //if(audioOut==0){   qDebug() << " - Initialize Phonon audio output"; audioOut = new Phonon::AudioOutput(); init=true;}
  if(mediaObj && init){  //in case it errors for some reason
    //qDebug() << " -- Create path between audio objects";
    //Phonon::createPath(mediaObj, audioOut);
    qDebug() << " -- Move audio objects to separate thread";
    mediaObj->moveToThread(audioThread);
    //audioOut->moveToThread(audioThread);
    audioThread->start();
  }
  if(mediaObj !=0 ){//&& audioOut!=0){
    //mediaObj->setCurrentSource(QUrl(filepath));
    mediaObj->setMedia(QUrl::fromLocalFile(filepath));
    mediaObj->setVolume(100);
    mediaObj->play();
    //audioThread->start();
  }
  if(DEBUG){ qDebug() << " - Done with Audio File"; }
}
// =======================
//  XCB EVENT FILTER FUNCTIONS
// =======================
void LSession::WindowPropertyEvent(){
  if(DEBUG){ qDebug() << "Window Property Event"; }
  QList<WId> newapps = XCB->WindowList();
  if(RunningApps.length() < newapps.length()){
    //New Window found
    LSession::restoreOverrideCursor(); //restore the mouse cursor back to normal (new window opened?)
    //Perform sanity checks on any new window geometries
    for(int i=0; i<newapps.length(); i++){
      if(!RunningApps.contains(newapps[i])){ adjustWindowGeom(newapps[i]); }
    }
  }
  RunningApps = newapps;
  emit WindowListEvent();
}

void LSession::SysTrayDockRequest(WId win){
  attachTrayWindow(win); //Check to see if the window is already registered
}

void LSession::WindowClosedEvent(WId win){
  removeTrayWindow(win); //Check to see if the window is a tray app
}

void LSession::WindowConfigureEvent(WId win){
  for(int i=0; i<RunningTrayApps.length(); i++){
    if(win==RunningTrayApps[i]){
      if(DEBUG){ qDebug() << "SysTray: Configure Event"; }
      emit TrayIconChanged(RunningTrayApps[i]); //trigger a repaint event
      break;
    }
  }
}

void LSession::WindowDamageEvent(WId win){
  for(int i=0; i<RunningTrayApps.length(); i++){
    if(win==RunningTrayApps[i]){
      if(DEBUG){ qDebug() << "SysTray: Damage Event"; }
      emit TrayIconChanged(RunningTrayApps[i]); //trigger a repaint event
      break;
    }
  }	
}

void LSession::WindowSelectionClearEvent(WId win){
  if(win==SystemTrayID){
    qDebug() << "Stopping system tray";
    stopSystemTray(true); //make sure to detach all windows
  }
}


//======================
//   SYSTEM TRAY FUNCTIONS
//======================
bool LSession::registerVisualTray(WId visualTray){
  //Only one visual tray can be registered at a time
  //  (limitation of how tray apps are embedded)
  if(VisualTrayID==0){ VisualTrayID = visualTray; return true; }
  else if(VisualTrayID==visualTray){ return true; }
  else{ return false; }
}

void LSession::unregisterVisualTray(WId visualTray){
  if(VisualTrayID==visualTray){ 
    qDebug() << "Unregistered Visual Tray";
    VisualTrayID=0; 
    emit VisualTrayAvailable();
  }
}

QList<WId> LSession::currentTrayApps(WId visualTray){
  if(visualTray==VisualTrayID){
    return RunningTrayApps;
  }else if( registerVisualTray(visualTray) ){
    return RunningTrayApps;
  }else{
    return QList<WId>();
  }
}

void LSession::startSystemTray(){
  if(SystemTrayID!=0){ return; }
  RunningTrayApps.clear(); //nothing running yet
  SystemTrayID = LX11::startSystemTray(0);
  TrayStopping = false;
  if(SystemTrayID!=0){
    XSelectInput(QX11Info::display(), SystemTrayID, InputOutput); //make sure TrayID events get forwarded here
    XDamageQueryExtension( QX11Info::display(), &TrayDmgEvent, &TrayDmgError);
    evFilter->setTrayDamageFlag(TrayDmgEvent);
    qDebug() << "System Tray Started Successfully";
    if(DEBUG){ qDebug() << " - System Tray Flags:" << TrayDmgEvent << TrayDmgError; }
  }
}

void LSession::stopSystemTray(bool detachall){
  TrayStopping = true; //make sure the internal list does not modified during this
  //Close all the running Tray Apps
  for(int i=0; i<RunningTrayApps.length(); i++){
    if(!detachall){ LX11::CloseWindow(RunningTrayApps[i]); }
  }
  LX11::closeSystemTray(SystemTrayID);
  SystemTrayID = 0;
  TrayDmgEvent = 0; 
  TrayDmgError = 0;
  RunningTrayApps.clear();
  emit TrayListChanged();
}

void LSession::attachTrayWindow(WId win){
  //static int appnum = 0;
  if(TrayStopping){ return; }
  if(RunningTrayApps.contains(win)){ return; } //already managed
  RunningTrayApps << win;
  if(DEBUG){ qDebug() << "Tray List Changed"; }
  emit TrayListChanged();
  /*//Now try to embed the window into the tray 
  //(NOT USED - Breaks visuals due to X11 graphics constraints - need to do embed in a single visual tray instead)
  
  qDebug() << "Attach Tray App:" << appnum;
  WId cont = LX11::CreateWindow( SystemTrayID, QRect(appnum*64, 0, 64, 64) );
  if( LX11::EmbedWindow(win, cont) ){
    appnum++;
    //Successful embed, now set it up for damage report notifications
    XDamageCreate( QX11Info::display(), win, XDamageReportRawRectangles );
    //LX11::ResizeWindow(win, 64, 64); //Always use 64x64 if possible (can shrink, not expand later)
    LX11::RestoreWindow(win);
    //Add it to the tray list
    RunningTrayApps << win;
    TrayAppContainers << cont;
    //Emit that the list has changed
    emit TrayListChanged();
  }else{
    LX11::DestroyWindow(cont); //clean up
  }*/
}

void LSession::removeTrayWindow(WId win){
  if(TrayStopping || SystemTrayID==0){ return; }
  for(int i=0; i<RunningTrayApps.length(); i++){
    if(win==RunningTrayApps[i]){ 
      RunningTrayApps.removeAt(i); 
      emit TrayListChanged();
      break;	    
    }
  }	
}
