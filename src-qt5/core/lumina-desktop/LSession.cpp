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
#include <QMimeData>
#include "LXcbEventFilter.h"
#include "BootSplash.h"

//LibLumina X11 class
#include <LuminaX11.h>
#include <LUtils.h>
#include <ExternalProcess.h>
#include <LIconCache.h>

#include <unistd.h> //for usleep() usage

#ifndef DEBUG
#define DEBUG 0
#endif

XCBEventFilter *evFilter = 0;
LIconCache *ICONS = 0;

LSession::LSession(int &argc, char ** argv) : LSingleApplication(argc, argv, "lumina-desktop"){
 xchange = false;
 if(this->isPrimaryProcess()){
  connect(this, SIGNAL(InputsAvailable(QStringList)), this, SLOT(NewCommunication(QStringList)) );
  this->setApplicationName("Lumina Desktop Environment");
  this->setApplicationVersion( LDesktopUtils::LuminaDesktopVersion() );
  this->setOrganizationName("LuminaDesktopEnvironment");
  this->setQuitOnLastWindowClosed(false); //since the LDesktop's are not necessarily "window"s
  //Enabled a few of the simple effects by default
  this->setEffectEnabled( Qt::UI_AnimateMenu, true);
  this->setEffectEnabled( Qt::UI_AnimateCombo, true);
  this->setEffectEnabled( Qt::UI_AnimateTooltip, true);
  //this->setAttribute(Qt::AA_UseDesktopOpenGL);
  this->setAttribute(Qt::AA_UseHighDpiPixmaps); //allow pixmaps to be scaled up as well as down
  //this->setStyle( new MenuProxyStyle); //QMenu icon size override
  SystemTrayID = 0; VisualTrayID = 0;
  sysWindow = 0;
  TrayDmgEvent = 0;
  TrayDmgError = 0;
  lastActiveWin = 0;
  cleansession = true;
  TrayStopping = false;
  xchange = false;
  ICONS = new LIconCache(this);
  screenTimer = new QTimer(this);
    screenTimer->setSingleShot(true);
    screenTimer->setInterval(50);
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
  ScreenSaver=0;
  //Setup the event filter for Qt5
  evFilter =  new XCBEventFilter(this);
  this->installNativeEventFilter( evFilter );
  connect(this, SIGNAL(screenAdded(QScreen*)), this, SLOT(screensChanged()) );
  connect(this, SIGNAL(screenRemoved(QScreen*)), this, SLOT(screensChanged()) );
  connect(this, SIGNAL(primaryScreenChanged(QScreen*)), this, SLOT(screensChanged()) );

  // Clipboard
  ignoreClipboard = false;
  qRegisterMetaType<QClipboard::Mode>("QClipboard::Mode");
  connect(QApplication::clipboard(), SIGNAL(changed(QClipboard::Mode)), this, SLOT(handleClipboard(QClipboard::Mode)));
 } //end check for primary process
}

LSession::~LSession(){
 if(this->isPrimaryProcess()){
  //WM->stopWM();
  for(int i=0; i<DESKTOPS.length(); i++){
    DESKTOPS[i]->deleteLater();
  }
  //delete WM;
  settingsmenu->deleteLater();
  appmenu->deleteLater();
  if(ScreenSaver!=0){ ScreenSaver->deleteLater(); }
  delete currTranslator;
  if(mediaObj!=0){delete mediaObj;}
 }
}

//Static function so everything can get the same icon name
QString LSession::batteryIconName(int charge, bool charging){
  int icon = -1;
  if (charge > 90) { icon = 4; }
  else if (charge > 70) { icon = 3; }
  else if (charge > 20) { icon = 2; }
  else if (charge > 5) { icon = 1; }
  else if (charge > 0 ) { icon = 0; }
  if(charging){ icon = icon+10; }
  QStringList iconList;
    switch (icon) {
      case 0:
        iconList << "battery-20" << "battery-020" << "battery-empty" << "battery-caution";
        break;
      case 1:
        iconList << "battery-40" << "battery-040" << "battery-low" << "battery-caution";
        break;
      case 2:
        iconList << "battery-60" << "battery-060" << "battery-good";
        break;
      case 3:
        iconList << "battery-80" << "battery-080" << "battery-good";
        break;
      case 4:
        iconList << "battery-100" << "battery-full";
        break;
      case 10:
        iconList << "battery-20-charging" << "battery-020-charging" << "battery-empty-charging" << "battery-caution-charging"
		<< "battery-charging-20" << "battery-charging-020" << "battery-charging-empty" << "battery-charging-caution";
        break;
      case 11:
        iconList << "battery-40-charging" << "battery-040-charging" << "battery-low-charging" << "battery-caution-charging"
		<< "battery-charging-40" << "battery-charging-040" << "battery-charging-low" << "battery-charging-caution";
        break;
      case 12:
        iconList << "battery-60-charging" << "battery-060-charging" << "battery-good-charging"
		<< "battery-charging-60" << "battery-charging-060" << "battery-charging-good";
        break;
      case 13:
        iconList << "battery-80-charging" << "battery-080-charging" << "battery-good-charging"
		<< "battery-charging-80" << "battery-charging-080" << "battery-charging-good";
        break;
      case 14:
        if(charge==100){ iconList << "battery-full-charged"; }
        iconList << "battery-100-charging" << "battery-full-charging"
		<< "battery-charging-100" << "battery-charging-full";
        break;
      default:
        iconList << "battery-unknown" << "battery-missing";
        break;
    }
    iconList << "battery"; //generic battery icon
    for(int i=0; i<iconList.length(); i++){
      if( QIcon::hasThemeIcon(iconList[i]) ){ return iconList[i]; }
    }
    return ""; //no icon found
}

void LSession::setupSession(){
  //Seed random number generator (if needed)
  qsrand( QTime::currentTime().msec() );

  currTranslator = LUtils::LoadTranslation(this, "lumina-desktop");
  BootSplash splash;
    splash.showScreen("init");
  qDebug() << "Initializing Session";
  if(QFile::exists("/tmp/.luminastopping")){ QFile::remove("/tmp/.luminastopping"); }
  QTime* timer = 0;
  if(DEBUG){ timer = new QTime(); timer->start(); qDebug() << " - Init srand:" << timer->elapsed();}

  //Setup the QSettings default paths
    splash.showScreen("settings");
  if(DEBUG){ qDebug() << " - Init QSettings:" << timer->elapsed();}
  sessionsettings = new QSettings("lumina-desktop", "sessionsettings");
  DPlugSettings = new QSettings("lumina-desktop","pluginsettings/desktopsettings");
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
//use the system settings
  //Setup the user's lumina settings directory as necessary
    //splash.showScreen("user");
  //if(DEBUG){ qDebug() << " - Init User Files:" << timer->elapsed();}
  //checkUserFiles(); //adds these files to the watcher as well

  //Initialize the internal variables
  DESKTOPS.clear();

  //Start the background system tray
    splash.showScreen("systray");
  if(DEBUG){ qDebug() << " - Init System Tray:" << timer->elapsed();}
  startSystemTray();

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
  desktopFiles = QDir(LUtils::standardDirectory(LUtils::Desktop)).entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs, QDir::Name | QDir::IgnoreCase | QDir::DirsFirst);
  updateDesktops();
  //if(DEBUG){ qDebug() << " - Process Events (6x):" << timer->elapsed();}
  //for(int i=0; i<6; i++){ LSession::processEvents(); } //Run through this a few times so the interface systems get up and running

  //Now setup the system watcher for changes
    splash.showScreen("final");
  //qDebug() << " - Initialize file system watcher";
  if(DEBUG){ qDebug() << " - Init QFileSystemWatcher:" << timer->elapsed();}
  watcher = new QFileSystemWatcher(this);
    QString confdir = sessionsettings->fileName().section("/",0,-2);
    watcherChange(sessionsettings->fileName() );
    watcherChange( confdir+"/desktopsettings.conf" );
    watcherChange( confdir+"/fluxbox-init" );
    watcherChange( confdir+"/fluxbox-keys" );
    watcherChange( confdir+"/favorites.list" );
    //Try to watch the localized desktop folder too
    watcherChange( LUtils::standardDirectory(LUtils::Desktop) );
    //And watch the /media directory, and /run/media/USERNAME directory
   if(QFile::exists("/media")){  watcherChange("/media"); }
   QString userMedia = QString("/run/media/%1").arg(QDir::homePath().split("/").takeLast());
   if (QFile::exists(userMedia)) { watcherChange(userMedia); }
   if(!QFile::exists("/tmp/.autofs_change")){ system("touch /tmp/.autofs_change"); }
   watcherChange("/tmp/.autofs_change");
  //connect internal signals/slots
  connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(watcherChange(QString)) );
  connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(watcherChange(QString)) );
  connect(this, SIGNAL(aboutToQuit()), this, SLOT(SessionEnding()) );
  if(DEBUG){ qDebug() << " - Start screensaver:" << timer->elapsed(); }
  ScreenSaver = new LScreenSaver();
  ScreenSaver->start();
  //if(DEBUG){ qDebug() << " - Process Events (4x):" << timer->elapsed();}
  //for(int i=0; i<4; i++){ LSession::processEvents(); } //Again, just a few event loops here so thing can settle before we close the splash screen
  if(DEBUG){ qDebug() << " - Launch Startup Apps:" << timer->elapsed();}
  //launchStartupApps();
  QTimer::singleShot(500, this, SLOT(launchStartupApps()) );
  //if(DEBUG){ qDebug() << " - Hide Splashscreen:" << timer->elapsed();}
  //splash.hide();
  //LSession::processEvents();
  if(DEBUG){ qDebug() << " - Close Splashscreen:" << timer->elapsed();}
  splash.close();
  //LSession::processEvents();
  if(DEBUG){ qDebug() << " - Init Finished:" << timer->elapsed(); delete timer;}
}

void LSession::CleanupSession(){
  //Close any running applications and tray utilities (Make sure to keep the UI interactive)
  LSession::processEvents();
  QDateTime time = QDateTime::currentDateTime();
  qDebug() << "Start closing down the session: " << time.toString( Qt::SystemLocaleShortDate);
  //Create a temporary flag to prevent crash dialogs from opening during cleanup
  LUtils::writeFile("/tmp/.luminastopping",QStringList() << "yes", true);
  //Start the logout chimes (if necessary)
  int vol = LOS::audioVolume();
  if(vol>=0){ sessionsettings->setValue("last_session_state/audio_volume", vol); }
  bool playaudio = sessionsettings->value("PlayLogoutAudio",true).toBool();
  if( playaudio ){
      QString sfile = sessionsettings->value("audiofiles/logout", "").toString();
       if(sfile.isEmpty() || !QFile::exists(sfile)){ sfile = LOS::LuminaShare()+"Logout.ogg"; }
      playAudioFile(sfile);
  }
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
  //qDebug() << " - Stopping the window manager";
  //WM->stopWM();
  //Now close down the desktop
  qDebug() << " - Closing down the desktop elements";
  for(int i=0; i<DESKTOPS.length(); i++){
    DESKTOPS[i]->prepareToClose();
    //don't actually close them yet - that will happen when the session exits
    // this will leave the wallpapers up for a few moments (preventing black screens)
  }
  //Now wait a moment for things to close down before quitting
  if(playaudio && mediaObj!=0){
    //wait a max of 5 seconds for audio to finish
    bool waitmore = true;
    for(int i=0; i<100 && waitmore; i++){
      usleep(50000); //50ms = 50000 us
      waitmore = (mediaObj->state()==QMediaPlayer::PlayingState);
      LSession::processEvents();
    }
    if(waitmore){ mediaObj->stop(); } //timed out
  }else{
    for(int i=0; i<20; i++){ LSession::processEvents(); usleep(25000); } //1/2 second pause
  }
  //Clean up the temporary flag
  if(QFile::exists("/tmp/.luminastopping")){ QFile::remove("/tmp/.luminastopping"); }
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
    }else if(list[i]=="--show-start"){
      emit StartButtonActivated();
    }else if(list[i]=="--logout"){ QTimer::singleShot(1000, this, SLOT(StartLogout()));}
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
  //QProcess::startDetached("nice lumina-open -autostart-apps");
  ExternalProcess::launch("lumina-open", QStringList() << "-autostart-apps", false);

  //Re-load the screen brightness and volume settings from the previous session
  // Wait until after the XDG-autostart functions, since the audio system might be started that way
  qDebug() << " - Loading previous settings";
  tmp = sessionsettings->value("last_session_state/audio_volume",50).toInt();
  if(tmp>=0){ LOS::setAudioVolume(tmp); }
  qDebug() << " - - Audio Volume:" << QString::number(tmp)+"%";

  //Now play the login music since we are finished
  if(sessionsettings->value("PlayStartupAudio",true).toBool()){
     QString sfile = sessionsettings->value("audiofiles/login", "").toString();
     if(sfile.isEmpty() || !QFile::exists(sfile)){ sfile = LOS::LuminaShare()+"Login.ogg"; }
     playAudioFile(sfile);
  }
  //qDebug() << "[DESKTOP INIT FINISHED]";
}

void LSession::StartLogout(){
  CleanupSession();
  QCoreApplication::exit(0);
}

void LSession::StartShutdown(bool skipupdates){
  CleanupSession();
  LOS::systemShutdown(skipupdates);
  QCoreApplication::exit(0);
}

void LSession::StartReboot(bool skipupdates){
  CleanupSession();
  LOS::systemRestart(skipupdates);
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
  //if(changed.endsWith("fluxbox-init") || changed.endsWith("fluxbox-keys")){ refreshWindowManager(); }
  if(changed.endsWith("sessionsettings.conf") ){
    sessionsettings->sync();
    //qDebug() << "Session Settings Changed";
    if(sessionsettings->contains("Qt5_theme_engine")){
      QString engine = sessionsettings->value("Qt5_theme_engine","").toString();
      //qDebug() << "Set Qt5 theme engine: " << engine;
      if(engine.isEmpty()){ unsetenv("QT_QPA_PLATFORMTHEME"); }
      else{ setenv("QT_QPA_PLATFORMTHEME", engine.toUtf8().data(),1); }
    }else{
      setenv("QT_QPA_PLATFORMTHEME", "lthemeengine",1); //ensure the lumina theme engine is always enabled
    }
    emit SessionConfigChanged();
  }else if(changed.endsWith("desktopsettings.conf") ){ emit DesktopConfigChanged(); }
  else if(changed == LUtils::standardDirectory(LUtils::Desktop) ){
    desktopFiles = QDir(changed).entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs ,QDir::Name | QDir::IgnoreCase | QDir::DirsFirst);
    if(DEBUG){ qDebug() << "New Desktop Files:" << desktopFiles.length(); }
    emit DesktopFilesChanged();
  }else if(changed.toLower() == "/media" || changed.toLower().startsWith("/run/media/") || changed == "/tmp/.autofs_change" ){
    emit MediaFilesChanged();
  }else if(changed.endsWith("favorites.list")){ emit FavoritesChanged(); }
  //Now ensure this file was not removed from the watcher
  if(!watcher->files().contains(changed) && !watcher->directories().contains(changed)){
    if(!QFile::exists(changed)){
      //Create the file really quick to ensure it can be watched
      //TODO
    }
    watcher->addPath(changed);
  }
}

void LSession::screensChanged(){
  qDebug() << "Screen Number Changed";
  if(screenTimer->isActive()){ screenTimer->stop(); }
  screenTimer->start();
  xchange = true;
}

void LSession::screenResized(int scrn){
  qDebug() << "Screen Resized:" << scrn;
  if(screenTimer->isActive()){ screenTimer->stop(); }
  screenTimer->start();
  xchange = true;
}

void LSession::checkWindowGeoms(){
  //Only do one window per run (this will be called once per new window - with time delays between)
  if(checkWin.isEmpty()){ return; }
  WId win = checkWin.takeFirst();
  if(RunningApps.contains(win) ){ //just to make sure it did not close during the delay
    adjustWindowGeom( win );
  }
}

bool LSession::checkUserFiles(){
  //internal version conversion examples:
  //  [1.0.0 -> 1000000], [1.2.3 -> 1002003], [0.6.1 -> 6001]
  qDebug() << "Check User Files";
    //char tmp[] = "junk\0";
    //int tmpN = 0;
  //QApplication A(tmpN, (char **)&tmp);
  QSettings sset("lumina-desktop", "sessionsettings");
  QString OVS = sset.value("DesktopVersion","0").toString(); //Old Version String
  qDebug() << " - Old Version:" << OVS;
  qDebug() << " - Current Version:" << LDesktopUtils::LuminaDesktopVersion();
  bool changed = LDesktopUtils::checkUserFiles(OVS, LDesktopUtils::LuminaDesktopVersion());
  qDebug() << " - Made Changes:" << changed;
  if(changed){
    //Save the current version of the session to the settings file (for next time)
    sset.setValue("DesktopVersion", LDesktopUtils::LuminaDesktopVersion());
  }
  qDebug() << "Finished with user files check";
  //delete A;
  return changed;
}

void LSession::refreshWindowManager(){
  LUtils::runCmd("touch \""+QString(getenv("XDG_CONFIG_HOME"))+"/lumina-desktop/fluxbox-init\"" );
}

void LSession::updateDesktops(){
  qDebug() << " - Update Desktops";
  QList<QScreen*> screens = QGuiApplication::screens();
  int sC = screens.count();
  qDebug() << "  Screen Count:" << sC;
  qDebug() << "  DESKTOPS Length:" << DESKTOPS.length();
  if(sC<1){ return; } //stop here - no screens available temporarily (displayport/4K issue)
  screenRect = QRect(); //clear it
  QList<QScreen*>::const_iterator it;
  int i = 0;
  for(it = screens.constBegin(); it != screens.constEnd(); ++it, ++i) {
    screenRect = screenRect.united((*it)->geometry());
    qDebug() << " -- Screen["+QString::number(i)+"]:" << (*it)->geometry();
  }

  bool firstrun = (DESKTOPS.length()==0);
  QSettings dset("lumina-desktop", "desktopsettings");
  if(firstrun && sC==1){
    //Sanity check - ensure the monitor ID did not change between sessions for single-monitor setups
    QString name = QApplication::screens().at(0)->name();
    if(!dset.contains("desktop-"+name+"/screen/lastHeight")){
      //Empty Screen - find the previous one and migrate the settings over
      QStringList old = dset.allKeys().filter("desktop-").filter("/screen/lastHeight");
	QStringList lastused = dset.value("last_used_screens").toStringList();
      QString oldname;
      for(int i=0; i<old.length(); i++){
        QString tmp = old[i].section("/",0,0).section("-",1,-1); //old desktop ID
        if(lastused.contains(tmp)){
          oldname = tmp; break; //use the first screen that was last used
        }
      }
      if(!oldname.isEmpty()){ LDesktopUtils::MigrateDesktopSettings(&dset, oldname, name); }
    }
  }

  // If the screen count is changing on us
  if ( sC != QGuiApplication::screens().count() ) {
    qDebug() << "Screen Count changed while running";
    return;
  }

  //First clean up any current desktops
  QList<int> dnums; //keep track of which screens are already managed
  QList<QRect> geoms;
  for(int i=0; i<DESKTOPS.length(); i++){
    if ( DESKTOPS[i]->Screen() < 0 || DESKTOPS[i]->Screen() >= sC || geoms.contains(screens.at(i)->geometry())) {
        //qDebug() << " - Close desktop:" << i;
        qDebug() << " - Close desktop on screen:" << DESKTOPS[i]->Screen();
        DESKTOPS[i]->prepareToClose();
        DESKTOPS.takeAt(i)->deleteLater();
        i--;
      } else {
        //qDebug() << " - Show desktop:" << i;
        qDebug() << " - Show desktop on screen:" << DESKTOPS[i]->Screen();
        DESKTOPS[i]->UpdateGeometry();
        DESKTOPS[i]->show();
	dnums << DESKTOPS[i]->Screen();
	geoms << screens.at(i)->geometry();
      }
  }

  //Now add any new desktops
  QStringList allNames;
  QList<QScreen*> scrns = QApplication::screens();
  for(int i=0; i<sC; i++){
    allNames << scrns.at(i)->name();
    if(!dnums.contains(i) && !geoms.contains(screens.at(i)->geometry()) ){
      //Start the desktop on this screen
      qDebug() << " - Start desktop on screen:" << i;
      DESKTOPS << new LDesktop(i);
      geoms << screens.at(i)->geometry();
    }
  }
  dset.setValue("last_used_screens", allNames);
  //Make sure fluxbox also gets prompted to re-load screen config if the number of screens changes in the middle of a session
    if(!firstrun && xchange){
      qDebug() << "Update WM";
      //QProcess::startDetached("pkill fluxbox");
      xchange = false;
    }

  //Make sure all the background windows are registered on the system as virtual roots
  QTimer::singleShot(100,this, SLOT(registerDesktopWindows()));
  //Determine if any High-DPI screens are available and enable auto-scaling as needed
  /*for(int i=0; i<scrns.length(); i++){
    qDebug() << "Check Screen DPI:" << scrns[i]->name();
    qDebug() << " -- Physical DPI:" << scrns[i]->physicalDotsPerInchX() << "x" << scrns[i]->physicalDotsPerInchY();
    qDebug() << " -- Logical DPI:" << scrns[i]->logicalDotsPerInchX() << "x" << scrns[i]->logicalDotsPerInchY();
    if(scrns[i]->logicalDotsPerInchX() > 110 || scrns[i]->logicalDotsPerInchY()>110){ //4K is ~196, 3K is ~150
      setenv("QT_AUTO_SCREEN_SCALE_FACTOR","1",true); //Enable the automatic Qt5 DPI scaling for apps
      break;
    }else if(i==(scrns.length()-1)){
      unsetenv("QT_AUTO_SCREEN_SCALE_FACTOR");
    }
  }*/
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
  QList<QScreen *> screens = QGuiApplication::screens();
  for(int i=0; i<DESKTOPS.length(); i++){
    if( screens.at(i)->geometry().contains(geom.center()) ){
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
  }

}

void LSession::SessionEnding(){
  stopSystemTray(); //just in case it was not stopped properly earlier
}

void LSession::handleClipboard(QClipboard::Mode mode){
  if ( !ignoreClipboard && mode == QClipboard::Clipboard ){ //only support Clipboard
    const QMimeData *mime = QApplication::clipboard()->mimeData(mode);
    if (mime==NULL) { return; }
    if (mime->hasText() && !QApplication::clipboard()->ownsClipboard()) {
      //preserve the entire mimeData set, not just the text
      //Note that even when we want to "save" the test, we should keep the whole thing
      //  this preserves formatting codes and more that apps might need
      QMimeData *copy = new QMimeData();
      QStringList fmts = mime->formats();
      for(int i=0; i<fmts.length(); i++){ copy->setData(fmts[i], mime->data(fmts[i])); }
      ignoreClipboard = true;
      QApplication::clipboard()->setMimeData(copy, mode);
      ignoreClipboard = false;
      //QMetaObject::invokeMethod(this, "storeClipboard", Qt::QueuedConnection, Q_ARG(QString, mime->text()), Q_ARG(QClipboard::Mode, mode));
    }
  }
}

void LSession::storeClipboard(QString text, QClipboard::Mode mode){
  ignoreClipboard = true;
  QApplication::clipboard()->setText(text, mode);
  ignoreClipboard = false;
}

//===============
//  SYSTEM ACCESS
//===============
void LSession::LaunchApplication(QString cmd){
  //LSession::setOverrideCursor(QCursor(Qt::BusyCursor));
  ExternalProcess::launch(cmd, QStringList(), true);
  //QProcess::startDetached(cmd);
}

QFileInfoList LSession::DesktopFiles(){
  return desktopFiles;
}

QRect LSession::screenGeom(int num){
  QList<QScreen *> screens = QGuiApplication::screens();
  if(num < 0 || num >= screens.count() ){ return QRect(); }
  QRect geom = screens.at(num)->geometry();
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

LScreenSaver* LSession::screenSaver(){
  return ScreenSaver;
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
  if( !QFile::exists(filepath) ){ return; }
  //Setup the audio output systems for the desktop
  if(DEBUG){ qDebug() << "Play Audio File"; }
  if(mediaObj==0){   qDebug() << " - Initialize media player"; mediaObj = new QMediaPlayer(); }
  if(mediaObj !=0 ){
    if(DEBUG){ qDebug() << " - starting playback:" << filepath; }
    mediaObj->setVolume(100);
    mediaObj->setMedia(QUrl::fromLocalFile(filepath));
    mediaObj->play();
    LSession::processEvents();
  }
  if(DEBUG){ qDebug() << " - Done with Audio File"; }
}
// =======================
//  XCB EVENT FILTER FUNCTIONS
// =======================
void LSession::RootSizeChange(){
  if(DESKTOPS.isEmpty() || screenRect.isNull()){ return; } //Initial setup not run yet

  QRect tmp;
  QList<QScreen*> screens = QGuiApplication::screens();
  QList<QScreen*>::const_iterator it;
  for(it = screens.constBegin(); it != screens.constEnd(); ++it) {
    tmp = tmp.united( (*it)->geometry() );
  }
  if(tmp == screenRect){ return; } //false event - session size did not change
  qDebug() << "Got Root Size Change";
  xchange = true;
  screenTimer->start();
}

void LSession::WindowPropertyEvent(){
  if(DEBUG){ qDebug() << "Window Property Event"; }
  QList<WId> newapps = XCB->WindowList();
  if(RunningApps.length() < newapps.length()){
    //New Window found
    //qDebug() << "New window found";
    //LSession::restoreOverrideCursor(); //restore the mouse cursor back to normal (new window opened?)
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
    /*if( XCB->WindowClass(win).contains("VirtualBox")){
      qDebug() << "Found VirtualBox Window:";
      QList<LXCB::WINDOWSTATE> states = XCB->WM_Get_Window_States(win);
      if(states.contains(LXCB::S_FULLSCREEN) && !states.contains(LXCB::S_HIDDEN)){
       qDebug() << "Adjusting VirtualBox Window (fullscreen)";
        XCB->WM_Set_Window_Type(win, QList<LXCB::WINDOWTYPE>() << LXCB::T_NORMAL << LXCB::T_UTILITY );
        XCB->RestoreWindow(win);
      }
    }*/
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
  //LSession::restoreOverrideCursor();
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
//=========================
//  START MENU FUNCTIONS
//=========================
bool LSession::registerStartButton(QString ID){
  if(StartButtonID.isEmpty()){ StartButtonID = ID; }
  return (StartButtonID==ID);
}

void LSession::unregisterStartButton(QString ID){
  if(StartButtonID == ID){
    StartButtonID.clear();
    emit StartButtonAvailable();
  }
}
