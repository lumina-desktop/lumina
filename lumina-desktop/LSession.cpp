//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LSession.h"
#include <LuminaOS.h>

#include <QTime>
#include <QScreen>
#include <QtConcurrent>
#include "LXcbEventFilter.h"
#include "BootSplash.h"

//LibLumina X11 class
#include <LuminaX11.h>
#include <LuminaUtils.h>

#include <unistd.h> //for usleep() usage

#ifndef DEBUG
#define DEBUG 0
#endif

XCBEventFilter *evFilter = 0;

LSession::LSession(int &argc, char ** argv) : LSingleApplication(argc, argv, "lumina-desktop"){
 if(this->isPrimaryProcess()){
  connect(this, SIGNAL(InputsAvailable(QStringList)), this, SLOT(NewCommunication(QStringList)) );
  this->setApplicationName("Lumina Desktop Environment");
  this->setApplicationVersion( LUtils::LuminaDesktopVersion() );
  this->setOrganizationName("LuminaDesktopEnvironment");
  this->setQuitOnLastWindowClosed(false); //since the LDesktop's are not necessarily "window"s
  //Enabled a few of the simple effects by default
  this->setEffectEnabled( Qt::UI_AnimateMenu, true);
  this->setEffectEnabled( Qt::UI_AnimateCombo, true);
  this->setEffectEnabled( Qt::UI_AnimateTooltip, true);
  //this->setAttribute(Qt::AA_UseDesktopOpenGL);
  //this->setAttribute(Qt::AA_UseHighDpiPixmaps); //allow pixmaps to be scaled up as well as down
  //this->setStyle( new MenuProxyStyle); //QMenu icon size override
  SystemTrayID = 0; VisualTrayID = 0;
  sysWindow = 0;
  TrayDmgEvent = 0;
  TrayDmgError = 0;
  lastActiveWin = 0; 
  cleansession = true;
  TrayStopping = false;
  screenTimer = new QTimer(this);
    screenTimer->setSingleShot(true);
    screenTimer->setInterval(2000); //2 seconds - This needs to be long(ish) to prevent being called while
				    // X is still setting up any screens
    connect(screenTimer, SIGNAL(timeout()), this, SLOT(updateDesktops()) );
  for(int i=1; i<argc; i++){
    if( QString::fromLocal8Bit(argv[i]) == "--noclean" ){ cleansession = false; break; }
  }
  XCB = new LXCB(); //need access to XCB data/functions right away
  //initialize the empty internal pointers to 0
  appmenu = 0;
  settingsmenu = 0;
  currTranslator=0;
  mediaObj=0;
  sessionsettings=0;
  //Setup the event filter for Qt5
  evFilter =  new XCBEventFilter(this);
  this->installNativeEventFilter( evFilter );
 } //end check for primary process 
}

LSession::~LSession(){
 if(this->isPrimaryProcess()){
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
  DPlugSettings = new QSettings("pluginsettings","desktopsettings");
  //Load the proper translation files
  if(sessionsettings->value("ForceInitialLocale",false).toBool()){
    //Some system locale override it in place - change the env first
    LUtils::setLocaleEnv( sessionsettings->value("InitLocale/LANG","").toString(), \
				sessionsettings->value("InitLocale/LC_MESSAGES","").toString(), \
				sessionsettings->value("InitLocale/LC_TIME","").toString(), \
				sessionsettings->value("InitLocale/LC_NUMERIC","").toString(), \
				sessionsettings->value("InitLocale/LC_MONETARY","").toString(), \
				sessionsettings->value("InitLocale/LC_COLLATE","").toString(), \
				sessionsettings->value("InitLocale/LC_CTYPE","").toString() );
  }
  currTranslator = LUtils::LoadTranslation(this, "lumina-desktop"); 
//use the system settings
  //Setup the user's lumina settings directory as necessary
    splash.showScreen("user");
  if(DEBUG){ qDebug() << " - Init User Files:" << timer->elapsed();}  
  checkUserFiles(); //adds these files to the watcher as well

  //Initialize the internal variables
  DESKTOPS.clear();
  //savedScreens.clear();
  //for(int i=0; i<this->desktop()->screenCount(); i++){ savedScreens << this->desktop()->screenGeometry(i); }
      
  //Start the background system tray
    splash.showScreen("systray");
  if(DEBUG){ qDebug() << " - Init System Tray:" << timer->elapsed();}
  startSystemTray();
	
  //Launch Fluxbox
    splash.showScreen("wm");
  if(DEBUG){ qDebug() << " - Init WM:" << timer->elapsed();}
  WM = new WMProcess();
    WM->startWM();
	
  //Initialize the global menus
  qDebug() << " - Initialize system menus";
    splash.showScreen("apps");
  if(DEBUG){ qDebug() << " - Init AppMenu:" << timer->elapsed();}
  appmenu = new AppMenu();
  
    splash.showScreen("menus");
  if(DEBUG){ qDebug() << " - Init SettingsMenu:" << timer->elapsed();}
  settingsmenu = new SettingsMenu();
  if(DEBUG){ qDebug() << " - Init SystemWindow:" << timer->elapsed();}
  sysWindow = new SystemWindow();
  
  //Initialize the desktops
    splash.showScreen("desktop");
  if(DEBUG){ qDebug() << " - Init Desktops:" << timer->elapsed();}
  desktopFiles = QDir(QDir::homePath()+"/Desktop").entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs, QDir::Name | QDir::IgnoreCase | QDir::DirsFirst);
  updateDesktops();
  
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
    watcher->addPath( QDir::homePath()+"/Desktop" );

  //connect internal signals/slots
  connect(this->desktop(), SIGNAL(screenCountChanged(int)), this, SLOT(screensChanged()) );
  connect(this->desktop(), SIGNAL(resized(int)), this, SLOT(screenResized(int)) );
  connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(watcherChange(QString)) );
  connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(watcherChange(QString)) );
  connect(this, SIGNAL(aboutToQuit()), this, SLOT(SessionEnding()) );
  if(DEBUG){ qDebug() << " - Init Finished:" << timer->elapsed(); delete timer;}
  QApplication::processEvents();
  launchStartupApps();
  //QTimer::singleShot(500, this, SLOT(launchStartupApps()) );
  //QApplication::processEvents();
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

void LSession::NewCommunication(QStringList list){
  if(DEBUG){ qDebug() << "New Communications:" << list; }
  for(int i=0; i<list.length(); i++){
    if(list[i]=="--check-geoms"){
      screensChanged();
    }
  }	  
}

void LSession::launchStartupApps(){
  //First start any system-defined startups, then do user defined
  qDebug() << "Launching startup applications";

  //Enable Numlock
  if(LUtils::isValidBinary("numlockx")){ //make sure numlockx is installed
    if(sessionsettings->value("EnableNumlock",false).toBool()){
      QProcess::startDetached("numlockx on");
    }else{
      QProcess::startDetached("numlockx off");
    }
  }
  int tmp = LOS::ScreenBrightness();
  if(tmp>0){ 
    LOS::setScreenBrightness( tmp );
    qDebug() << " - - Screen Brightness:" << QString::number(tmp)+"%";
  }
  QProcess::startDetached("nice lumina-open -autostart-apps");
  
  //Re-load the screen brightness and volume settings from the previous session
  // Wait until after the XDG-autostart functions, since the audio system might be started that way
  qDebug() << " - Loading previous settings";
  tmp = LOS::audioVolume();
  LOS::setAudioVolume(tmp);
  qDebug() << " - - Audio Volume:" << QString::number(tmp)+"%";
  
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

void LSession::reloadIconTheme(){
  //Wait a moment for things to settle before sending out the signal to the interfaces
  QApplication::processEvents();
  QApplication::processEvents();
  emit IconThemeChanged();	
}

void LSession::watcherChange(QString changed){
  if(DEBUG){ qDebug() << "Session Watcher Change:" << changed; }
  if(changed.endsWith("fluxbox-init") || changed.endsWith("fluxbox-keys")){ refreshWindowManager(); }
  else if(changed.endsWith("sessionsettings.conf") ){ sessionsettings->sync(); emit SessionConfigChanged(); }
  else if(changed.endsWith("desktopsettings.conf") ){ emit DesktopConfigChanged(); }
  else if(changed == QDir::homePath()+"/Desktop"){ 
    desktopFiles = QDir(QDir::homePath()+"/Desktop").entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs ,QDir::Name | QDir::IgnoreCase | QDir::DirsFirst);
    if(DEBUG){ qDebug() << "New Desktop Files:" << desktopFiles.length(); }
    emit DesktopFilesChanged(); 
  }
  //Now ensure this file was not removed from the watcher
  if(!watcher->files().contains(changed) && !watcher->directories().contains(changed)){
    watcher->addPath(changed);
  }
  /*QStringList files = watcher->files();
  if(files.length() < 5){
    qDebug() << " - Resetting Watched Files...";
    watcher->removePaths(files); //clear the current files before re-setting them
    watcher->addPath( QDir::homePath()+"/.lumina/LuminaDE/sessionsettings.conf" );
    watcher->addPath( QDir::homePath()+"/.lumina/LuminaDE/desktopsettings.conf" );
    watcher->addPath( QDir::homePath()+"/.lumina/fluxbox-init" );
    watcher->addPath( QDir::homePath()+"/.lumina/fluxbox-keys" );
    watcher->addPath( QDir::homePath()+"/Desktop");
  }*/
}

void LSession::screensChanged(){
  qDebug() << "Screen Number Changed";
  if(screenTimer->isActive()){ screenTimer->stop(); }
  screenTimer->start();
  //updateDesktops();
}

void LSession::screenResized(int scrn){
  qDebug() << "Screen Resized:" << scrn; // << this->desktop()->screenGeometry(scrn);
  /*for(int i=0; i<DESKTOPS.length(); i++){
    if(DESKTOPS[i]->Screen() == scrn){ DESKTOPS[i]->UpdateGeometry(); return; }
  }*/	  
  if(screenTimer->isActive()){ screenTimer->stop(); }  
  screenTimer->start();	
  //updateDesktops();
}

void LSession::checkWindowGeoms(){
  //Only do one window per run (this will be called once per new window - with time delays between)
  if(checkWin.isEmpty()){ return; }
  WId win = checkWin.takeFirst();
  if(RunningApps.contains(win) ){ //just to make sure it did not close during the delay
    adjustWindowGeom( win );
  }
}

void LSession::checkUserFiles(){
  //internal version conversion examples: 
  //  [1.0.0 -> 1000000], [1.2.3 -> 1002003], [0.6.1 -> 6001]
  QString OVS = sessionsettings->value("DesktopVersion","0").toString(); //Old Version String
  int oldversion = VersionStringToNumber(OVS);
  int nversion = VersionStringToNumber(this->applicationVersion());
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
  //Convert any "userbutton" and "appmenu" panel plugins to the new "systemstart" plugin (0.8.7)
  if(oldversion <= 8007 && (newversion || newrelease) && nversion < 8008){
    QSettings dset(QSettings::UserScope, "LuminaDE","desktopsettings", this);
    QStringList plugKeys = dset.allKeys().filter("panel").filter("/pluginlist");
    for(int i=0; i<plugKeys.length(); i++){
      QStringList plugs = dset.value(plugKeys[i],QStringList()).toStringList();
      //Do the appmenu/userbutton -> systemstart conversion
      plugs = plugs.join(";;;;").replace("userbutton","systemstart").replace("appmenu","systemstart").split(";;;;");
      //Remove any system dashboard plugins
      plugs.removeAll("systemdashboard");
      //Now save that back to the file
      dset.setValue(plugKeys[i], plugs);
    }
    //Also remove any "desktopview" desktop plugin and enable the automatic desktop icons instead
    plugKeys = dset.allKeys().filter("desktop-").filter("/pluginlist");
    for(int i=0; i<plugKeys.length(); i++){
      QStringList plugs = dset.value(plugKeys[i], QStringList()).toStringList();
      QStringList old = plugs.filter("desktopview");
      bool found = !old.isEmpty();
      for(int j=0; j<old.length(); j++){ plugs.removeAll(old[j]); }
      if(found){
        dset.setValue(plugKeys[i],plugs); //save the modified plugin list
	//Also set the auto-generate flag on this desktop
	dset.setValue(plugKeys[i].section("/",0,0)+"/generateDesktopIcons", true);
      }
    }
    dset.sync();
    //Due to the grid size change for desktop plugins, need to remove any old plugin geometries
    if(QFile::exists(QDir::homePath()+"/.lumina/pluginsettings/desktopsettings.conf")){
      QFile::remove(QDir::homePath()+"/.lumina/pluginsettings/desktopsettings.conf");
    }
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
    /*if(QFile::exists(LOS::LuminaShare()+"defaultapps.conf")){
      if( QFile::copy(LOS::LuminaShare()+"defaultapps.conf", dset) ){
        QFile::setPermissions(dset, QFile::ReadUser | QFile::WriteUser | QFile::ReadOwner | QFile::WriteOwner);
      }
    }*/

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
  if(newversion || newrelease){
    sessionsettings->setValue("DesktopVersion", this->applicationVersion());
  }
}

void LSession::refreshWindowManager(){
  WM->updateWM();
}

void LSession::updateDesktops(){
  qDebug() << " - Update Desktops";
  QDesktopWidget *DW = this->desktop();
  int sC = DW->screenCount();
  qDebug() << "  Screen Count:" << sC;
  qDebug() << "  DESKTOPS Length:" << DESKTOPS.length();
  if(sC<1){ return; } //stop here - no screens available temporarily (displayport/4K issue)

  for(int i=0; i<sC; i++){ qDebug() << " -- Screen["+QString::number(i)+"]:" << DW->screenGeometry(i); }

  bool firstrun = (DESKTOPS.length()==0);
  bool numchange = DESKTOPS.length()!=sC;

  // If the screen count is changing on us
  if ( sC != DW->screenCount() ) {
    qDebug() << "Screen Count changed while running";
    return;
  }

  //qDebug() << "  -- Desktop Flags:" << firstrun << numchange << DW->isVirtualDesktop();
  for(int i=0; i<sC; i++){
      bool found = false;
      for (int j=0; j<DESKTOPS.length() && !found; j++) {
	if( DESKTOPS[j]->Screen() == i ) { found = true; break; }
      }
      if(!found) {
	//Start the desktop on the new screen
        qDebug() << " - Start desktop on screen:" << i << DW->screenGeometry(i) << "Virtual:" << DW->isVirtualDesktop();
        DESKTOPS << new LDesktop(i);
      }
  }

  // If we only have one desktop, let's end here
  if (DESKTOPS.length() <= 1) {
    QTimer::singleShot(100,this, SLOT(registerDesktopWindows()));
    return;
  }

  // If this isn't the initial setup
  if (!firstrun) {

    //Now go through and make sure to delete any desktops for detached screens
    for (int i=1; i<DESKTOPS.length(); i++){
      if (DESKTOPS[i]->Screen() >= sC) {
        qDebug() << " - Close desktop:" << i;
        qDebug() << " - Close desktop on screen:" << DESKTOPS[i]->Screen();
        DESKTOPS[i]->prepareToClose();
        //delete DESKTOPS.takeAt(i);
        DESKTOPS.removeAt(i);
        i--;
      } else {
        qDebug() << " - Show desktop:" << i;
        qDebug() << " - Show desktop on screen:" << DESKTOPS[i]->Screen();
        DESKTOPS[i]->UpdateGeometry();
        DESKTOPS[i]->show();
        //QTimer::singleShot(0,DESKTOPS[i], SLOT(checkResolution()));
      }
    }

    //Make sure fluxbox also gets prompted to re-load screen config if the number of screens changes
    if (numchange) {
      qDebug() << "Update WM";
      //QTimer::singleShot(1000,WM, SLOT(restartWM()));  //Note: This causes crashes in X if a full-screen app
      WM->updateWM();
    }
  } // End of !firstrun

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
  //return; //temporary disable
  if(DEBUG){ qDebug() << "AdjustWindowGeometry():" << win << maximize << XCB->WindowClass(win); }
  if(XCB->WindowIsFullscreen(win) >=0 ){ return; } //don't touch it
  //Quick hack for making sure that new windows are not located underneath any panels
  // Get the window location
  QRect geom = XCB->WindowGeometry(win, false);
  //Get the frame size
  QList<int> frame = XCB->WindowFrameGeometry(win); //[top,bottom,left,right] sizes of the frame
  //Calculate the full geometry (window + frame)
  QRect fgeom = QRect(geom.x()-frame[2], geom.y()-frame[0], geom.width()+frame[2]+frame[3], geom.height()+frame[0]+frame[1]);
  if(DEBUG){ 
    qDebug() << "Check Window Geometry:" << XCB->WindowClass(win) << !geom.isNull() << geom << fgeom;
  }
  if(geom.isNull()){ return; } //Could not get geometry for some reason
  //Get the available geometry for the screen the window is on
  QRect desk;
  for(int i=0; i<DESKTOPS.length(); i++){
    if( this->desktop()->screenGeometry(DESKTOPS[i]->Screen()).contains(geom.center()) ){
      //Window is on this screen
      if(DEBUG){ qDebug() << " - On Screen:" << DESKTOPS[i]->Screen(); }
      desk = DESKTOPS[i]->availableScreenGeom();
      if(DEBUG){ qDebug() << " - Screen Geom:" << desk; }
      break;
    }
  }
  if(desk.isNull()){ return; } //Unable to determine screen
  //Adjust the window location if necessary
  if(maximize){
    if(DEBUG){ qDebug() << " - Maximizing New Window:" << desk.width() << desk.height(); }
    geom = desk; //Use the full screen
    XCB->MoveResizeWindow(win, geom);
    XCB->MaximizeWindow(win, true); //directly set the appropriate "maximized" flags (bypassing WM)
	  
  }else if(!desk.contains(fgeom) ){
    //Adjust origin point for left/top margins
    if(fgeom.y() < desk.y()){ geom.moveTop(desk.y()+frame[0]); fgeom.moveTop(desk.y()); } //move down to the edge (top panel)
    if(fgeom.x() < desk.x()){ geom.moveLeft(desk.x()+frame[2]); fgeom.moveLeft(desk.x()); } //move right to the edge (left panel)
    //Adjust size for bottom margins (within reason, since window titles are on top normally)
   // if(geom.right() > desk.right() && (geom.width() > 100)){ geom.setRight(desk.right()); }
    if(fgeom.bottom() > desk.bottom() && geom.height() > 10){ 
      if(DEBUG){ qDebug() << "Adjust Y:" << fgeom << geom << desk; }
      int diff = fgeom.bottom()-desk.bottom(); //amount of overlap
      if(DEBUG){ qDebug() << "Y-Diff:" << diff; }
      if(diff < 0){ diff = -diff; } //need a positive value
      if( (fgeom.height()+ diff)< desk.height()){
        //just move the window - there is room for it above
	geom.setBottom(desk.bottom()-frame[1]); 
	fgeom.setBottom(desk.bottom());
      }else if(geom.height() > diff){ //window bigger than the difference
	//Need to resize the window - keeping the origin point the same
	geom.setHeight( geom.height()-diff-1 ); //shrink it by the difference (need an extra pixel somewhere)
	fgeom.setHeight( fgeom.height()-diff );
      }
    }
    //Now move/resize the window
    if(DEBUG){ 
      qDebug() << " - New Geom:" << geom << fgeom; 
    }
    XCB->WM_Request_MoveResize_Window(win, geom);
    /*
      //Need to use the frame origin point with the window size (for some reason - strange Fluxbox issue)
      XCB->MoveResizeWindow(win, QRect(fgeom.topLeft(), geom.size()) );

    //For the random windows which are *still* off the top of the screen 
    QRect nfgeom = XCB->WindowGeometry(win, true); //re-fetch the current geometry (including frame)
    if(nfgeom!=fgeom){
      if(DEBUG){ qDebug() << " -- Adjust again:" << fgeom; }
      XCB->MoveResizeWindow(win, geom);
    }*/
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

QFileInfoList LSession::DesktopFiles(){
  return desktopFiles;	
}

QRect LSession::screenGeom(int num){
  if(num < 0 || num >= this->desktop()->screenCount() ){ return QRect(); }
  QRect geom = this->desktop()->screenGeometry(num);
  QScreen* scrn = this->screens().at(num);
  //if(DEBUG){ qDebug() << "Screen Geometry:" << num << geom << scrn->geometry() << scrn->virtualGeometry(); }
  if(geom.isNull() ){
    if( !scrn->geometry().isNull() ){ geom = scrn->geometry(); }
    else if( !scrn->virtualGeometry().isNull() ){ geom = scrn->virtualGeometry(); }
    //else if(num < savedScreens.length() ){
    //Qt is backfiring (Xinarama w/ Fluxbox?) - return the saved geometry
    //geom = savedScreens[num];	
    //}
  }
  return geom;
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

QSettings* LSession::DesktopPluginSettings(){
  return DPlugSettings;
}

WId LSession::activeWindow(){
  //Check the last active window pointer first
  WId active = XCB->ActiveWindow();
  //qDebug() << "Check Active Window:" << active << lastActiveWin;
  if(RunningApps.contains(active)){ lastActiveWin = active; }
  else if(RunningApps.contains(lastActiveWin) && XCB->WindowState(lastActiveWin) >= LXCB::VISIBLE){} //no change needed
  else if(RunningApps.contains(lastActiveWin) && RunningApps.length()>1){
    int start = RunningApps.indexOf(lastActiveWin);
    if(start<1){ lastActiveWin = RunningApps.length()-1; } //wrap around to the last item
    else{ lastActiveWin = RunningApps[start-1]; }
  }else{
    //Need to change the last active window - find the first one which is visible
    lastActiveWin = 0; //fallback value - nothing active
    for(int i=0; i<RunningApps.length(); i++){
      if(XCB->WindowState(RunningApps[i]) >= LXCB::VISIBLE){
        lastActiveWin = RunningApps[i];
	break;
      }
    }
    //qDebug() << " -- New Last Active Window:" << lastActiveWin;
  }
  return lastActiveWin;
}

//Temporarily change the session locale (nothing saved between sessions)
void LSession::switchLocale(QString localeCode){
  currTranslator = LUtils::LoadTranslation(this, "lumina-desktop", localeCode, currTranslator); 
  if(currTranslator!=0 || localeCode=="en_US"){ 
    LUtils::setLocaleEnv(localeCode); //will set everything to this locale (no custom settings)
  }
  emit LocaleChanged();
}

void LSession::systemWindow(){
  if(sysWindow==0){ sysWindow = new SystemWindow(); }
  else{ sysWindow->updateWindow(); }
  sysWindow->show();
  //LSession::processEvents();
}

//Play System Audio
void LSession::playAudioFile(QString filepath){
  //Setup the audio output systems for the desktop
  if(DEBUG){ qDebug() << "Play Audio File"; }
  if(mediaObj==0){   qDebug() << " - Initialize media player"; mediaObj = new QMediaPlayer(); }
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
    //qDebug() << "New window found";
    LSession::restoreOverrideCursor(); //restore the mouse cursor back to normal (new window opened?)
    //Perform sanity checks on any new window geometries
    for(int i=0; i<newapps.length() && !TrayStopping; i++){
      if(!RunningApps.contains(newapps[i])){ 
        checkWin << newapps[i]; 
	XCB->SelectInput(newapps[i]); //make sure we get property/focus events for this window
	if(DEBUG){ qDebug() << "New Window - check geom in a moment:" << XCB->WindowClass(newapps[i]); }
	QTimer::singleShot(50, this, SLOT(checkWindowGeoms()) );
      }
    }
  }
  
  //Now save the list and send out the event
  RunningApps = newapps;
  emit WindowListEvent();
}

void LSession::WindowPropertyEvent(WId win){
  //Emit the single-app signal if the window in question is one used by the task manager
  if(RunningApps.contains(win)){
    if(DEBUG){ qDebug() << "Single-window property event"; }
    //emit WindowListEvent();
    WindowPropertyEvent(); //Run through the entire routine for window checks
  }else if(RunningTrayApps.contains(win)){
    emit TrayIconChanged(win);
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
    if(RunningTrayApps.contains(win)){
      if(DEBUG){ qDebug() << "SysTray: Configure Event"; }
      emit TrayIconChanged(win); //trigger a repaint event
    }else if(RunningApps.contains(win)){
      WindowPropertyEvent();
    }
}

void LSession::WindowDamageEvent(WId win){
  if(TrayStopping){ return; }
    if(RunningTrayApps.contains(win)){
      if(DEBUG){ qDebug() << "SysTray: Damage Event"; }
      emit TrayIconChanged(win); //trigger a repaint event
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
    //Check the validity of all the current tray apps (make sure nothing closed erratically)
    for(int i=0; i<RunningTrayApps.length(); i++){
      if(XCB->WindowClass(RunningTrayApps[i]).isEmpty()){ RunningTrayApps.removeAt(i); i--; }
    }
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
  SystemTrayID = XCB->startSystemTray(0);
  TrayStopping = false;
  if(SystemTrayID!=0){
    XCB->SelectInput(SystemTrayID); //make sure TrayID events get forwarded here
    TrayDmgEvent = XCB->GenerateDamageID(SystemTrayID);
    //XDamageQueryExtension( QX11Info::display(), &TrayDmgEvent, &TrayDmgError);
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
 XCB->closeSystemTray(SystemTrayID);
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
  qDebug() << "Session Tray: Window Added";
  RunningTrayApps << win;
  LSession::restoreOverrideCursor();
  if(DEBUG){ qDebug() << "Tray List Changed"; }
  emit TrayListChanged();
}

void LSession::removeTrayWindow(WId win){
  if(SystemTrayID==0){ return; }
  for(int i=0; i<RunningTrayApps.length(); i++){
    if(win==RunningTrayApps[i]){ 
      qDebug() << "Session Tray: Window Removed";
      RunningTrayApps.removeAt(i); 
      emit TrayListChanged();
      break;	    
    }
  }	
}
