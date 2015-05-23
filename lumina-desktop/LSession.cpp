//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LSession.h"
#include <LuminaOS.h>

#include <QTime>
#include "LXcbEventFilter.h"
#include "BootSplash.h"

//LibLumina X11 class
#include <LuminaX11.h>
#include <LuminaUtils.h>

#include <unistd.h> //for usleep() usage

//X includes (these need to be last due to Qt compile issues)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xdamage.h>

#ifndef DEBUG
#define DEBUG 0
#endif

XCBEventFilter *evFilter = 0;

LSession::LSession(int &argc, char ** argv) : QApplication(argc, argv){
  this->setApplicationName("Lumina Desktop Environment");
  this->setApplicationVersion( LUtils::LuminaDesktopVersion() );
  this->setOrganizationName("LuminaDesktopEnvironment");
  this->setQuitOnLastWindowClosed(false); //since the LDesktop's are not necessarily "window"s
  //Enabled a few of the simple effects by default
  this->setEffectEnabled( Qt::UI_AnimateMenu, true);
  this->setEffectEnabled( Qt::UI_AnimateCombo, true);
  this->setEffectEnabled( Qt::UI_AnimateTooltip, true);
  //this->setAttribute(Qt::AA_UseHighDpiPixmaps); //allow pixmaps to be scaled up as well as down
  //this->setStyle( new MenuProxyStyle); //QMenu icon size override
  SystemTrayID = 0; VisualTrayID = 0;
  sysWindow = 0;
  TrayDmgEvent = 0;
  TrayDmgError = 0;
  cleansession = true;
  TrayStopping = false;
  for(int i=1; i<argc; i++){
    if( QString::fromLocal8Bit(argv[i]) == "--noclean" ){ cleansession = false; break; }
  }
  XCB = new LXCB(); //need access to XCB data/functions right away
  //initialize the empty internal pointers to 0
  appmenu = 0;
  settingsmenu = 0;
  currTranslator=0;
  mediaObj=0;
  //audioThread=0;
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
}

void LSession::setupSession(){
  BootSplash splash;
    splash.showScreen("init");
  qDebug() << "Initializing Session";
  if(QFile::exists("/tmp/.luminastopping")){ QFile::remove("/tmp/.luminastopping"); }
  QTime* timer = 0;
  if(DEBUG){ timer = new QTime(); timer->start(); qDebug() << " - Init srand:" << timer->elapsed();}
  //Seed random number generator (if needed)
  qsrand( QTime::currentTime().msec() );
  //Setup the QSettings default paths
    splash.showScreen("settings");
  if(DEBUG){ qDebug() << " - Init QSettings:" << timer->elapsed();}
  QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, QDir::homePath()+"/.lumina");
  sessionsettings = new QSettings("LuminaDE", "sessionsettings");
  //Setup the user's lumina settings directory as necessary
    splash.showScreen("user");
  if(DEBUG){ qDebug() << " - Init User Files:" << timer->elapsed();}  
  checkUserFiles(); //adds these files to the watcher as well

  //Initialize the internal variables
  DESKTOPS.clear();
	
  //Start the background system tray
    splash.showScreen("systray");
  if(DEBUG){ qDebug() << " - Init System Tray:" << timer->elapsed();}
  startSystemTray();
	
  //Launch Fluxbox
    splash.showScreen("wm");
  if(DEBUG){ qDebug() << " - Init WM:" << timer->elapsed();}
  WM = new WMProcess();
    WM->startWM();
	
  //Initialize the desktops
    splash.showScreen("desktop");
  if(DEBUG){ qDebug() << " - Init Desktops:" << timer->elapsed();}
  updateDesktops();

  //Initialize the global menus
  qDebug() << " - Initialize system menus";
    splash.showScreen("apps");
  if(DEBUG){ qDebug() << " - Init AppMenu:" << timer->elapsed();}
  appmenu = new AppMenu();
  if(DEBUG){ qDebug() << " - Init SettingsMenu:" << timer->elapsed();}
  splash.showScreen("menus");
  settingsmenu = new SettingsMenu();
  if(DEBUG){ qDebug() << " - Init SystemWindow:" << timer->elapsed();}
  sysWindow = new SystemWindow();
  
  //Now setup the system watcher for changes
    splash.showScreen("final");
  qDebug() << " - Initialize file system watcher";
  if(DEBUG){ qDebug() << " - Init QFileSystemWatcher:" << timer->elapsed();}
  watcher = new QFileSystemWatcher(this);
    //watcher->addPath( QDir::homePath()+"/.lumina/stylesheet.qss" );
    watcher->addPath( QDir::homePath()+"/.lumina/LuminaDE/sessionsettings.conf" );
    watcher->addPath( QDir::homePath()+"/.lumina/LuminaDE/desktopsettings.conf" );
    watcher->addPath( QDir::homePath()+"/.lumina/fluxbox-init" );
    watcher->addPath( QDir::homePath()+"/.lumina/fluxbox-keys" );

  //connect internal signals/slots
  connect(this->desktop(), SIGNAL(screenCountChanged(int)), this, SLOT(updateDesktops()) );
  connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(watcherChange(QString)) );
  connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(watcherChange(QString)) );
  connect(this, SIGNAL(aboutToQuit()), this, SLOT(SessionEnding()) );
  if(DEBUG){ qDebug() << " - Init Finished:" << timer->elapsed(); delete timer;}
  QTimer::singleShot(3000, this, SLOT(launchStartupApps()) ); //startup these processes in 3 seconds
  splash.close();
}

void LSession::CleanupSession(){
  //Close any running applications and tray utilities (Make sure to keep the UI interactive)
  LSession::processEvents();
  QDateTime time = QDateTime::currentDateTime();
  qDebug() << "Start closing down the session: " << time.toString( Qt::SystemLocaleShortDate);
  //Create a temporary flag to prevent crash dialogs from opening during cleanup
  LUtils::writeFile("/tmp/.luminastopping",QStringList() << "yes", true);
  //Start the logout chimes (if necessary)
  bool playaudio = sessionsettings->value("PlayLogoutAudio",true).toBool();
  if( playaudio ){ playAudioFile(LOS::LuminaShare()+"Logout.ogg"); }
  //Stop the background system tray (detaching/closing apps as necessary)
  stopSystemTray(!cleansession);
  //Now perform any other cleanup
  if(cleansession){
    //Close any open windows
    //qDebug() << " - Closing any open windows";
    QList<WId> WL = XCB->WindowList(true);
    for(int i=0; i<WL.length(); i++){
      qDebug() << " - Closing window:" << XCB->WindowClass(WL[i]) << WL[i];
      XCB->CloseWindow(WL[i]);
      LSession::processEvents();
    }
    //Now wait a moment for things to close down before quitting
    for(int i=0; i<20; i++){ LSession::processEvents(); usleep(25); } //1/2 second pause
    //Kill any remaining windows
    WL = XCB->WindowList(true); //all workspaces
    for(int i=0; i<WL.length(); i++){
      qDebug() << " - Window did not close, killing application:" << XCB->WindowClass(WL[i]) << WL[i];
      XCB->KillClient(WL[i]);
      LSession::processEvents();
    }
  }
  evFilter->StopEventHandling();
  //Stop the window manager
  qDebug() << " - Stopping the window manager";
  WM->stopWM();
  //Now close down the desktop
  qDebug() << " - Closing down the desktop elements";
  for(int i=0; i<DESKTOPS.length(); i++){
    DESKTOPS[i]->prepareToClose(); 
    //don't actually close them yet - that will happen when the session exits
    // this will leave the wallpapers up for a few moments (preventing black screens)
  }
  //Now wait a moment for things to close down before quitting
  if(playaudio){
    //wait a max of 3 seconds for audio to finish
    bool waitmore = true;
    for(int i=0; i<60 && waitmore; i++){
      usleep(50000); //50ms = 50000 us
      waitmore = (mediaObj->state()==QMediaPlayer::PlayingState);
      //waitmore = !audioThread->wait(500);
      LSession::processEvents();
    }
    if(waitmore){ mediaObj->stop(); } //timed out
  }else{
    for(int i=0; i<20; i++){ LSession::processEvents(); usleep(25000); } //1/2 second pause
  }
  //Clean up the temporary flag
  if(QFile::exists("/tmp/.luminastopping")){ QFile::remove("/tmp/.luminastopping"); }
  //if(audioThread!=0){ audioThread->exit(0); }
}

int LSession::VersionStringToNumber(QString version){
  version = version.section("-",0,0); //trim any extra labels off the end
  int maj, mid, min; //major/middle/minor version numbers (<Major>.<Middle>.<Minor>)
  maj = mid = min = 0; 
  bool ok = true;
  maj = version.section(".",0,0).toInt(&ok);
  if(ok){ mid = version.section(".",1,1).toInt(&ok); }else{ maj = 0; }
  if(ok){ min = version.section(".",2,2).toInt(&ok); }else{ mid = 0; }
  if(!ok){ min = 0; }
  //Now assemble the number
  //NOTE: This format allows numbers to be anywhere from 0->999 without conflict
  return (maj*1000000 + mid*1000 + min);
}

void LSession::launchStartupApps(){
  //First start any system-defined startups, then do user defined
  qDebug() << "Launching startup applications";

  //Enable Numlock
  if(LUtils::isValidBinary("numlockx")){ //make sure numlockx is installed
    if(sessionsettings->value("EnableNumlock",false).toBool()){
      QProcess::startDetached("numlockx on");
    }
    else{
      QProcess::startDetached("numlockx off");
    }
  }
    
  //First create the list of all possible locations in order of precedence
  // NOTE: Lumina/system defaults should be launched earlier due to possible system admin utilities
  /*QStringList filelist; 
    filelist << LOS::LuminaShare()+"startapps"; //anything special for the Lumina installation
    filelist << "/etc/luminaStartapps" << LOS::SysPrefix()+"luminaStartapps" << LOS::AppPrefix()+"luminaStartapps"; //System defaults
    filelist << QDir::homePath()+"/.lumina/startapps"; //User defaults
    filelist.removeDuplicates(); //just in case sysPrefix/appPrefix/etc are the same
  //Now load all the available files
  QStringList entries; //where to save good entries
  for(int i=0; i<filelist.length(); i++){
    if( !QFile::exists(filelist[i]) ){ continue; } //file does not exist
    QFile file(filelist[i]);
    if( file.open(QIODevice::ReadOnly | QIODevice::Text) ){
      QTextStream in(&file);
      while(!in.atEnd()){
        QString entry = in.readLine();
        if(entry.startsWith("#") || entry.isEmpty()){ continue; }
        entries << entry;
      }
      file.close();
    }
  }
  //Now start all the listed apps
  entries.removeDuplicates(); //Just in case something is duplicated between system/user defaults
  for(int i=0; i<entries.length(); i++){
    qDebug() << " - Starting Application:" << entries[i];
    LSession::LaunchApplication(entries[i]);
    LSession::processEvents();
  }*/
  
  //Now get any XDG startup applications and launch them
  QList<XDGDesktop> xdgapps = LXDG::findAutoStartFiles();
  for(int i=0; i<xdgapps.length(); i++){
    qDebug() << " - Auto-Starting File:" << xdgapps[i].filePath;
    if(xdgapps[i].startupNotify){
      LSession::LaunchApplication("lumina-open \""+xdgapps[i].filePath+"\"");
    }else{
      //Don't update the mouse cursor
      QProcess::startDetached("lumina-open \""+xdgapps[i].filePath+"\"");
    }
    //Put a tiny bit of space between app starts (don't overload the system)
    for(int i=0; i<5; i++){
      usleep(50000); //50ms = 50000 us --> 250ms total wait
      LSession::processEvents();
    }
    
  }
  
  //Re-load the screen brightness and volume settings from the previous session
  // Wait until after the XDG-autostart functions, since the audio system might be started that way
  qDebug() << " - Loading previous settings";
  int tmp = LOS::audioVolume();
  LOS::setAudioVolume(tmp);
  qDebug() << " - - Audio Volume:" << QString::number(tmp)+"%";
  tmp = LOS::ScreenBrightness();
  LOS::setScreenBrightness( tmp );
  qDebug() << " - - Screen Brightness:" << QString::number(tmp)+"%";
  
  //Now play the login music since we are finished
  if(sessionsettings->value("PlayStartupAudio",true).toBool()){
    //Make sure to re-set the system volume to the last-used value at outset
    int vol = LOS::audioVolume();
    if(vol>=0){ LOS::setAudioVolume(vol); }
    LSession::playAudioFile(LOS::LuminaShare()+"Login.ogg");
  }
  qDebug() << " - Finished with startup routines";
}

void LSession::StartLogout(){
  CleanupSession();
  QCoreApplication::exit(0);
}

void LSession::StartShutdown(){
  CleanupSession();
  LOS::systemShutdown();
  QCoreApplication::exit(0);		
}

void LSession::StartReboot(){
  CleanupSession();
  LOS::systemRestart();
  QCoreApplication::exit(0);	
}

void LSession::watcherChange(QString changed){
  if(DEBUG){ qDebug() << "Session Watcher Change:" << changed; }
  if(changed.endsWith("fluxbox-init") || changed.endsWith("fluxbox-keys")){ refreshWindowManager(); }
  else if(changed.endsWith("sessionsettings.conf") ){ sessionsettings->sync(); emit SessionConfigChanged(); }
  else if(changed.endsWith("desktopsettings.conf") ){ emit DesktopConfigChanged(); }

  //Now double-check all the watches files to ensure that none of them got removed

  QStringList files = watcher->files();
  if(files.length() < 4){
    qDebug() << " - Resetting Watched Files...";
    watcher->removePaths(files); //clear the current files before re-setting them
    watcher->addPath( QDir::homePath()+"/.lumina/LuminaDE/sessionsettings.conf" );
    watcher->addPath( QDir::homePath()+"/.lumina/LuminaDE/desktopsettings.conf" );
    watcher->addPath( QDir::homePath()+"/.lumina/fluxbox-init" );
    watcher->addPath( QDir::homePath()+"/.lumina/fluxbox-keys" );
  }
}

void LSession::checkUserFiles(){
  //internal version conversion examples: 
  //  [1.0.0 -> 1000000], [1.2.3 -> 1002003], [0.6.1 -> 6001]
  QString OVS = sessionsettings->value("DesktopVersion","0").toString(); //Old Version String
  int oldversion = VersionStringToNumber(OVS);
  bool newversion =  ( oldversion < VersionStringToNumber(this->applicationVersion()) ); //increasing version number
  bool newrelease = ( OVS.contains("-devel", Qt::CaseInsensitive) && this->applicationVersion().contains("-release", Qt::CaseInsensitive) ); //Moving from devel to release
  
  //Check for the desktop settings file
  QString dset = QDir::homePath()+"/.lumina/LuminaDE/desktopsettings.conf";
  bool firstrun = false;
  if(!QFile::exists(dset) || oldversion < 5000){
    if( oldversion < 5000 ){ QFile::remove(dset); qDebug() << "Current desktop settings obsolete: Re-implementing defaults"; }
    else{ firstrun = true; }
    LUtils::LoadSystemDefaults();
  }
  //Convert the favorites framework as necessary (change occured with 0.8.4)
  if(newversion || newrelease){
    LUtils::upgradeFavorites(oldversion);	  
  }
  //Convert to the XDG autostart spec as necessary (Change occured with 0.8.5)
  if(QFile::exists(QDir::homePath()+"/.lumina/startapps") ){
    QStringList cmds = LUtils::readFile(QDir::homePath()+"/.lumina/startapps");
    for(int i=0; i<cmds.length(); i++){
      cmds[i] = cmds[i].remove("lumina-open").simplified(); //remove the file opener
      if(cmds[i].startsWith("#") || cmds[i].isEmpty()){ continue; } //invalid line
      
      LXDG::setAutoStarted(true, cmds[i]);
    }
    QFile::remove(QDir::homePath()+"/.lumina/startapps"); //delete the old file
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
  else if(newversion || newrelease){
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
	if(firstrun && DW->screenGeometry(i).x()==0){
	  DESKTOPS << new LDesktop(i,true); //set this one as the default	
	}else{
          DESKTOPS << new LDesktop(i);
	}
      }
    }
    //qDebug() << " - Done Starting Desktops";

    if(!firstrun){//Done right here on first run
    //Now go through and make sure to delete any desktops for detached screens
      for(int i=0; i<DESKTOPS.length(); i++){
        if(DESKTOPS[i]->Screen() >= DW->screenCount()){
	  qDebug() << " - Close desktop on screen:" << DESKTOPS[i]->Screen();
          DESKTOPS[i]->prepareToClose();
	  delete DESKTOPS.takeAt(i);
	  i--;
        }else{
	  qDebug() << " - Show desktop on screen:" << DESKTOPS[i]->Screen();
          DESKTOPS[i]->show();
	  //QTimer::singleShot(0,DESKTOPS[i], SLOT(checkResolution()));
        }
      }
      WM->updateWM(); //Make sure fluxbox also gets prompted to re-load screen config
    }
    //Make sure all the background windows are registered on the system as virtual roots
    QTimer::singleShot(100,this, SLOT(registerDesktopWindows()));
    //qDebug() << " - Done Checking Desktops";
}

void LSession::registerDesktopWindows(){
  QList<WId> wins;
  for(int i=0; i<DESKTOPS.length(); i++){
    wins << DESKTOPS[i]->backgroundID();
  }
  XCB->RegisterVirtualRoots(wins);
}

void LSession::adjustWindowGeom(WId win, bool maximize){
  //Quick hack for making sure that new windows are not located underneath any panels
  // Get the window location
  QRect geom = XCB->WindowGeometry(win, true); //always include the frame if possible
  if(DEBUG){ qDebug() << "Check Window Geometry:" << XCB->WindowClass(win) << !geom.isNull(); }
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
  if(maximize){
    if(DEBUG){ qDebug() << " - Maximizing New Window:" << desk.width() << desk.height(); }
    geom = desk; //Use the full screen
    XCB->MoveResizeWindow(win, geom);
    XCB->MaximizeWindow(win, true); //directly set the appropriate "maximized" flags (bypassing WM)
	  
  }else if(!desk.contains(geom) ){
    if(DEBUG){
      qDebug() << " - Desk:" << desk.x() << desk.y() << desk.width() << desk.height();
      qDebug() << " - Geom:" << geom.x() << geom.y() << geom.width() << geom.height();
    }
    //Adjust origin point for left/top margins
    if(geom.y() < desk.y()){ geom.moveTop(desk.y()); } //move down to the edge (top panel)
    if(geom.x() < desk.x()){ geom.moveLeft(desk.x()); } //move right to the edge (left panel)
    //Adjust size for bottom margins (within reason, since window titles are on top normally)
   // if(geom.right() > desk.right() && (geom.width() > 100)){ geom.setRight(desk.right()); }
    if(geom.bottom() > desk.bottom() && geom.height() > 100){ geom.setBottom(desk.bottom()); }
    //Now move/resize the window
    if(DEBUG){ qDebug() << " - New Geom:" << geom.x() << geom.y() << geom.width() << geom.height(); }
    XCB->MoveResizeWindow(win, geom);
  }
  
}

void LSession::SessionEnding(){
  stopSystemTray(); //just in case it was not stopped properly earlier
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
  if(sysWindow==0){ sysWindow = new SystemWindow(); }
  else{ sysWindow->updateWindow(); }
  sysWindow->show();
  /*SystemWindow win;
  win.exec();*/
  LSession::processEvents();
}

//Play System Audio
void LSession::playAudioFile(QString filepath){
  //Setup the audio output systems for the desktop
  if(DEBUG){ qDebug() << "Play Audio File"; }
  //if(audioThread==0){   qDebug() << " - Initialize audio systems"; audioThread = new QThread(); init = true; }
  if(mediaObj==0){   qDebug() << " - Initialize media player"; mediaObj = new QMediaPlayer(); }
  /*if(mediaObj && init){  //in case it errors for some reason
    qDebug() << " -- Move audio objects to separate thread";
    mediaObj->moveToThread(audioThread);
    audioThread->start();
  }*/
  if(mediaObj !=0 ){
    if(DEBUG){ qDebug() << " - starting playback:" << filepath; }
    mediaObj->setVolume(100);
    mediaObj->setMedia(QUrl::fromLocalFile(filepath));
    mediaObj->play();
    //if(!audioThread->isRunning()){ audioThread->start(); }
    LSession::processEvents();
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
    for(int i=0; i<newapps.length() && !TrayStopping; i++){
      if(!RunningApps.contains(newapps[i])){ adjustWindowGeom(newapps[i]); }
    }
  }
  RunningApps = newapps;
  emit WindowListEvent();
}

void LSession::WindowPropertyEvent(WId win){
  //Emit the single-app signal if the window in question is one used by the task manager
  if(RunningApps.contains(win)){
    if(DEBUG){ qDebug() << "Single-window property event"; }
    emit WindowListEvent();
  }
}

void LSession::SysTrayDockRequest(WId win){
  if(TrayStopping){ return; }
  attachTrayWindow(win); //Check to see if the window is already registered
}

void LSession::WindowClosedEvent(WId win){
  if(TrayStopping){ return; }
  removeTrayWindow(win); //Check to see if the window is a tray app
}

void LSession::WindowConfigureEvent(WId win){
  if(TrayStopping){ return; } 
  for(int i=0; i<RunningTrayApps.length(); i++){
    if(win==RunningTrayApps[i]){
      if(DEBUG){ qDebug() << "SysTray: Configure Event"; }
      emit TrayIconChanged(RunningTrayApps[i]); //trigger a repaint event
      break;
    }
  }
}

void LSession::WindowDamageEvent(WId win){
  if(TrayStopping){ return; }
  for(int i=0; i<RunningTrayApps.length(); i++){
    if(win==RunningTrayApps[i]){
      if(DEBUG){ qDebug() << "SysTray: Damage Event"; }
      emit TrayIconChanged(RunningTrayApps[i]); //trigger a repaint event
      break;
    }
  }	
}

void LSession::WindowSelectionClearEvent(WId win){
  if(win==SystemTrayID && !TrayStopping){
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
  if(TrayStopping){ return false; }
  else if(VisualTrayID==0){ VisualTrayID = visualTray; return true; }
  else if(VisualTrayID==visualTray){ return true; }
  else{ return false; }
}

void LSession::unregisterVisualTray(WId visualTray){
  if(VisualTrayID==visualTray){ 
    qDebug() << "Unregistered Visual Tray";
    VisualTrayID=0; 
    if(!TrayStopping){ emit VisualTrayAvailable(); }
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
  if(TrayStopping){ return; } //already run
  qDebug() << "Stopping system tray...";
  TrayStopping = true; //make sure the internal list does not modified during this
  //Close all the running Tray Apps
  QList<WId> tmpApps = RunningTrayApps;
  RunningTrayApps.clear(); //clear this ahead of time so tray's do not attempt to re-access the apps
  if(!detachall){
    for(int i=0; i<tmpApps.length(); i++){
      qDebug() << " - Stopping tray app:" << XCB->WindowClass(tmpApps[i]);
      //XCB->CloseWindow(RunningTrayApps[i]);
      //Tray apps are special and closing the window does not close the app
      XCB->KillClient(tmpApps[i]);
      LSession::processEvents();
    }  
  }
  //Now close down the tray backend
  LX11::closeSystemTray(SystemTrayID);
  SystemTrayID = 0;
  TrayDmgEvent = 0; 
  TrayDmgError = 0;
  evFilter->setTrayDamageFlag(0); //turn off tray event handling
  emit TrayListChanged();
  LSession::processEvents();
}

void LSession::attachTrayWindow(WId win){
  //static int appnum = 0;
  if(TrayStopping){ return; }
  if(RunningTrayApps.contains(win)){ return; } //already managed
  RunningTrayApps << win;
  LSession::restoreOverrideCursor();
  if(DEBUG){ qDebug() << "Tray List Changed"; }
  emit TrayListChanged();
}

void LSession::removeTrayWindow(WId win){
  if(SystemTrayID==0){ return; }
  for(int i=0; i<RunningTrayApps.length(); i++){
    if(win==RunningTrayApps[i]){ 
      RunningTrayApps.removeAt(i); 
      emit TrayListChanged();
      break;	    
    }
  }	
}
