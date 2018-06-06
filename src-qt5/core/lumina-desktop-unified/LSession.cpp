//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012-2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LSession.h"
#include "global-objects.h"

#include "BootSplash.h"

#ifndef DEBUG
#define DEBUG 0
#endif

//Initialize all the global objects to null pointers
NativeWindowSystem* Lumina::NWS = 0;
NativeEventFilter* Lumina::NEF = 0;
LScreenSaver* Lumina::SS = 0;
QThread* Lumina::EVThread = 0;
RootWindow* Lumina::ROOTWIN = 0;
XDGDesktopList* Lumina::APPLIST = 0;
LShortcutEvents* Lumina::SHORTCUTS = 0;
DesktopManager* Lumina::DESKMAN = 0;

LSession::LSession(int &argc, char ** argv) : LSingleApplication(argc, argv, "lumina-desktop-unified"){
  //Initialize the global objects to null pointers
  qRegisterMetaType< Qt::Key >("Qt::Key");
  qRegisterMetaType< NativeWindowObject::Property >("NativeWindowObject::Property");
  qRegisterMetaType< QList< NativeWindowObject::Property > >("QList<NativeWindowObject::Property>");
  qRegisterMetaType< NativeWindowSystem::MouseButton >("NativeWindowSystem::MouseButton");

  mediaObj = 0; //private object used for playing login/logout chimes
 if(this->isPrimaryProcess()){
  //Setup the global registrations
  if(DEBUG){ qDebug() << "Starting session init..."; }
  qsrand(QDateTime::currentMSecsSinceEpoch());
  this->setApplicationName("Lumina Desktop Environment");
  this->setApplicationVersion( LDesktopUtils::LuminaDesktopVersion() );
  this->setOrganizationName("LuminaDesktopEnvironment");
  this->setQuitOnLastWindowClosed(false); //since the LDesktop's are not necessarily "window"s
  //Enable a few of the simple effects by default
  if(DEBUG){ qDebug() << " - Setting attributes and effects"; }
  this->setEffectEnabled( Qt::UI_AnimateMenu, true);
  this->setEffectEnabled( Qt::UI_AnimateCombo, true);
  this->setEffectEnabled( Qt::UI_AnimateTooltip, true);
  this->setAttribute(Qt::AA_UseDesktopOpenGL);
  this->setAttribute(Qt::AA_UseHighDpiPixmaps); //allow pixmaps to be scaled up as well as down

  //Now initialize the global objects (but do not start them yet)
  if(DEBUG){ qDebug() << " - Create native event objects"; }
  Lumina::NEF = new NativeEventFilter();
  Lumina::NWS = new NativeWindowSystem();
  Lumina::SS = new LScreenSaver();
  Lumina::DESKMAN = new DesktopManager();
  //Now put the Native Window System into it's own thread to keep things snappy
  if(DEBUG){ qDebug() << " - Create extra threads"; }
  Lumina::EVThread = new QThread();
    Lumina::DESKMAN->moveToThread(Lumina::EVThread);
  Lumina::EVThread->start();
  Lumina::APPLIST = XDGDesktopList::instance();
  if(DEBUG){ qDebug() << " - Create root window"; }
  Lumina::ROOTWIN = new RootWindow();
  Lumina::SHORTCUTS = new LShortcutEvents(); //this can be moved to it's own thread eventually as well
  if(DEBUG){ qDebug() << " - Setup Connections"; }
  setupGlobalConnections();
 } //end check for primary process
 if(DEBUG){ qDebug() << " [finished] Session init"; }
}

LSession::~LSession(){
   //Clean up the global objects as needed
  if(Lumina::NEF!=0){ Lumina::NEF->deleteLater(); }
  if(Lumina::NWS!=0){ Lumina::NWS->deleteLater(); }
  if(Lumina::SS!=0){ Lumina::SS->deleteLater(); }
  if(Lumina::EVThread!=0){
    if(Lumina::EVThread->isRunning()){ Lumina::EVThread->quit(); }
    Lumina::EVThread->deleteLater();
  }
  if(DesktopSettings::instance()!=0){ DesktopSettings::instance()->deleteLater(); }
  if(Lumina::ROOTWIN!=0){ Lumina::ROOTWIN->deleteLater(); }
  if(Lumina::APPLIST!=0){ Lumina::APPLIST->deleteLater(); }
  if(Lumina::DESKMAN!=0){ Lumina::DESKMAN->deleteLater(); }
  if(OSInterface::instance()->isRunning()){ OSInterface::instance()->stop(); }
  OSInterface::instance()->deleteLater();
}

void LSession::setupSession(){
  BootSplash splash;
    splash.showScreen("init");
  qDebug() << "Setting up session:" << QDateTime::currentDateTime().toString( Qt::SystemLocaleShortDate);;
  if(QFile::exists("/tmp/.luminastopping")){ QFile::remove("/tmp/.luminastopping"); }
  QTime* timer = 0;
  if(DEBUG){ timer = new QTime(); timer->start(); qDebug() << " - Init srand:" << timer->elapsed();}
  //Seed random number generator
  qsrand( QTime::currentTime().msec() );
  //Connect internal signal/slots
  connect(this, SIGNAL(InputsAvailable(QStringList)), this, SLOT(NewCommunication(QStringList)) );
  //Setup the QSettings default paths
    splash.showScreen("settings");
  if(DEBUG){ qDebug() << " - Init QSettings:" << timer->elapsed();}
  DesktopSettings::instance(); //don't do anything other than init/start the static object here
  if(DEBUG){ qDebug() << " - Load Localization Files:" << timer->elapsed();}
  currTranslator = LUtils::LoadTranslation(this, "lumina-desktop");
  if(DEBUG){ qDebug() << " - Start Event Filter:" << timer->elapsed(); }
  Lumina::NEF->start();
  if( !Lumina::NWS->start() ){
    qWarning() << "Could not start the Lumina desktop. Is another desktop or window manager running?";
    this->exit(1);
    return;
  }
//use the system settings
  //Setup the user's lumina settings directory as necessary
    splash.showScreen("user");
  if(DEBUG){ qDebug() << " - Init User Files:" << timer->elapsed();}
  //checkUserFiles(); //adds these files to the watcher as well
  Lumina::NWS->setRoot_numberOfWorkspaces(QStringList() << "one" << "two");
  Lumina::NWS->setRoot_currentWorkspace(0);
  OSInterface::instance()->start();

  Lumina::DESKMAN->start();
  Lumina::ROOTWIN->start();
  //Initialize the internal variables


  //Start the background system tray
    splash.showScreen("systray");
  //Initialize the global menus
  qDebug() << " - Initialize system menus";
    splash.showScreen("apps");
  if(DEBUG){ qDebug() << " - Populate App List:" << timer->elapsed();}
  Lumina::APPLIST->updateList();

    splash.showScreen("menus");


  //Initialize the desktops
    splash.showScreen("desktop");

  //for(int i=0; i<6; i++){ LSession::processEvents(); } //Run through this a few times so the interface systems get up and running

  //Now setup the system watcher for changes
    splash.showScreen("final");

  if(DEBUG){ qDebug() << " - Start Screen Saver:" << timer->elapsed();}
  Lumina::SS->start();

  if(DEBUG){ qDebug() << " - Init Finished:" << timer->elapsed(); delete timer;}
  Lumina::SHORTCUTS->start(); //Startup the shortcut handler now

  //for(int i=0; i<4; i++){ LSession::processEvents(); } //Again, just a few event loops here so thing can settle before we close the splash screen
  //launchStartupApps();
  QTimer::singleShot(500, this, SLOT(launchStartupApps()) );
  splash.hide();
  LSession::processEvents();
  splash.close();
  LSession::processEvents();
  //DEBUG: Wait a bit then close down the session
  //QTimer::singleShot(15000, this, SLOT(StartLogout()) );
}

//================
//    PRIVATE
//================
void LSession::CleanupSession(){
  //Close any running applications and tray utilities (Make sure to keep the UI interactive)
  LSession::processEvents();
  QDateTime time = QDateTime::currentDateTime();
  qDebug() << "Start closing down the session: " << time.toString( Qt::SystemLocaleShortDate);
  //Create a temporary flag to prevent crash dialogs from opening during cleanup
  LUtils::writeFile("/tmp/.luminastopping",QStringList() << "yes", true);
  //Start the logout chimes (if necessary)
  //LOS::setAudioVolume( LOS::audioVolume() ); //make sure the audio volume is saved in the backend for the next login
  bool playaudio = DesktopSettings::instance()->value(DesktopSettings::Session,"PlayLogoutAudio",true).toBool();
  if( playaudio ){ playAudioFile(LOS::LuminaShare()+"Logout.ogg"); }
  //Now perform any other cleanup
  //Lumina::NEF->stop();
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

//=================

void LSession::setupGlobalConnections(){
  //Setup the various connections between the global classes
  // NOTE: Most of these connections will only become "active" as the global objects get started during the setupSession routine

  //Setup the basic connections between the shortcuts class and the session itself
  connect(Lumina::SHORTCUTS, SIGNAL(StartLogout()), this, SLOT(StartLogout()) );
  connect(Lumina::SHORTCUTS, SIGNAL(StartReboot()), this, SLOT(StartReboot()) );
  connect(Lumina::SHORTCUTS, SIGNAL(StartShutdown()), this, SLOT(StartShutdown()) );
  connect(Lumina::SHORTCUTS, SIGNAL(LaunchApplication(QString)), this, SLOT(LaunchApplication(QString)) );
  connect(Lumina::SHORTCUTS, SIGNAL(LaunchStandardApplication(QString)), this, SLOT(LaunchStandardApplication(QString)) );
  connect(Lumina::SHORTCUTS, SIGNAL(LockSession()), Lumina::SS, SLOT(LockScreenNow()) );

  //Root window connections
  connect(Lumina::ROOTWIN, SIGNAL(RegisterVirtualRoot(WId)), Lumina::NWS, SLOT(RegisterVirtualRoot(WId)) );
  connect(Lumina::ROOTWIN, SIGNAL(RootResized(QRect)), Lumina::NWS, SLOT(setRoot_desktopGeometry(QRect)) );
  connect(RootDesktopObject::instance(), SIGNAL(mouseMoved()), Lumina::SS, SLOT(newInputEvent()) );
  connect(RootDesktopObject::instance(), SIGNAL(startLogout()), this, SLOT(StartLogout()) );
  connect(RootDesktopObject::instance(), SIGNAL(lockScreen()), Lumina::SS, SLOT(LockScreenNow()) );
  connect(RootDesktopObject::instance(), SIGNAL(launchApplication(QString)), this, SLOT(LaunchStandardApplication(QString)) );

  //Native Window Class connections
  connect(Lumina::NEF, SIGNAL(WindowCreated(WId)), Lumina::NWS, SLOT(NewWindowDetected(WId)));
  connect(Lumina::NEF, SIGNAL(WindowDestroyed(WId)), Lumina::NWS, SLOT(WindowCloseDetected(WId)));
  connect(Lumina::NEF, SIGNAL(WindowPropertyChanged(WId, NativeWindowObject::Property)), Lumina::NWS, SLOT(WindowPropertyChanged(WId, NativeWindowObject::Property)));
  connect(Lumina::NEF, SIGNAL(WindowPropertiesChanged(WId, QList<NativeWindowObject::Property>)), Lumina::NWS, SLOT(WindowPropertiesChanged(WId, QList<NativeWindowObject::Property>)) );
  connect(Lumina::NEF, SIGNAL(WindowPropertyChanged(WId, NativeWindowObject::Property, QVariant)), Lumina::NWS, SLOT(WindowPropertyChanged(WId, NativeWindowObject::Property, QVariant)));
  connect(Lumina::NEF, SIGNAL(WindowPropertiesChanged(WId, QList<NativeWindowObject::Property>, QList<QVariant>)), Lumina::NWS, SLOT(WindowPropertiesChanged(WId, QList<NativeWindowObject::Property>, QList<QVariant>)) );
  connect(Lumina::NEF, SIGNAL(RequestWindowPropertyChange(WId, NativeWindowObject::Property, QVariant)), Lumina::NWS, SLOT(RequestPropertyChange(WId, NativeWindowObject::Property, QVariant)));
  connect(Lumina::NEF, SIGNAL(RequestWindowPropertiesChange(WId, QList<NativeWindowObject::Property>, QList<QVariant>)), Lumina::NWS, SLOT(RequestPropertiesChange(WId, QList<NativeWindowObject::Property>, QList<QVariant>)));
  connect(Lumina::NEF, SIGNAL(TrayWindowCreated(WId)), Lumina::NWS, SLOT(NewTrayWindowDetected(WId)));
  connect(Lumina::NEF, SIGNAL(TrayWindowDestroyed(WId)), Lumina::NWS, SLOT(WindowCloseDetected(WId)));
  connect(Lumina::NEF, SIGNAL(PossibleDamageEvent(WId)), Lumina::NWS, SLOT(CheckDamageID(WId)));
  connect(Lumina::NEF, SIGNAL(KeyPressed(int, WId)), Lumina::NWS, SLOT(NewKeyPress(int, WId)));
  connect(Lumina::NEF, SIGNAL(KeyReleased(int, WId)), Lumina::NWS, SLOT(NewKeyRelease(int, WId)));
  connect(Lumina::NEF, SIGNAL(MousePressed(int, WId)), Lumina::NWS, SLOT(NewMousePress(int, WId)));
  connect(Lumina::NEF, SIGNAL(MouseReleased(int, WId)), Lumina::NWS, SLOT(NewMouseRelease(int, WId)));
  //connect(Lumina::NEF, SIGNAL(MouseMovement(WId)), Lumina::NWS, SLOT());
  //connect(Lumina::NEF, SIGNAL(MouseEnterWindow(WId)), Lumina::NWS, SLOT());
  //connect(Lumina::NEF, SIGNAL(MouseLeaveWindow(WId)), Lumina::NWS, SLOT());

  //Input Events for ScreenSaver
  connect(Lumina::NEF, SIGNAL(KeyPressed(int, WId)), Lumina::SS, SLOT(newInputEvent()));
  connect(Lumina::NEF, SIGNAL(KeyReleased(int, WId)), Lumina::SS, SLOT(newInputEvent()));
  connect(Lumina::NEF, SIGNAL(MousePressed(int, WId)), Lumina::SS, SLOT(newInputEvent()));
  connect(Lumina::NEF, SIGNAL(MouseReleased(int, WId)), Lumina::SS, SLOT(newInputEvent()));
  connect(Lumina::NEF, SIGNAL(MouseMovement()), Lumina::SS, SLOT(newInputEvent()));

  connect(Lumina::SS, SIGNAL(LockStatusChanged(bool)), Lumina::NWS, SLOT(ScreenLockChanged(bool)) );

  //Mouse/Keyboard Shortcut Events (Make sure to connect to the NWS - the raw events need to be ignored sometimes)
  connect(Lumina::NWS, SIGNAL(KeyPressDetected(WId, Qt::Key)), Lumina::SHORTCUTS, SLOT(KeyPress(WId, Qt::Key)) );
  connect(Lumina::NWS, SIGNAL(KeyReleaseDetected(WId, Qt::Key)), Lumina::SHORTCUTS, SLOT(KeyRelease(WId, Qt::Key)) );
  connect(Lumina::NWS, SIGNAL(MousePressDetected(WId, NativeWindowSystem::MouseButton)), Lumina::SHORTCUTS, SLOT(MousePress(WId, NativeWindowSystem::MouseButton)) );
  connect(Lumina::NWS, SIGNAL(MouseReleaseDetected(WId, NativeWindowSystem::MouseButton)), Lumina::SHORTCUTS, SLOT(MouseRelease(WId, NativeWindowSystem::MouseButton)) );

  //NWS Events to the window system
  connect(Lumina::NWS, SIGNAL(NewWindowAvailable(NativeWindowObject*)), Lumina::DESKMAN, SLOT(NewWindowAvailable(NativeWindowObject*)) );
  connect(Lumina::NWS, SIGNAL(WindowClosed()), Lumina::DESKMAN, SLOT(syncWindowList()) );
  connect(Lumina::NWS, SIGNAL(NewTrayWindowAvailable(NativeWindowObject*)), Lumina::DESKMAN, SLOT(NewTrayWindowAvailable(NativeWindowObject*)) );
  connect(Lumina::NWS, SIGNAL(TrayWindowClosed()), Lumina::DESKMAN, SLOT(syncTrayWindowList()) );

}

//=================

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

//=================

//Play System Audio
void LSession::playAudioFile(QString filepath){
  if( !QFile::exists(filepath) ){ return; }
  //Setup the audio output systems for the desktop
  if(DEBUG){ qDebug() << "Play Audio File"; }
  if(mediaObj==0){   qDebug() << " - Initialize media player"; mediaObj = new QMediaPlayer(0,QMediaPlayer::LowLatency); }
  if(mediaObj !=0 ){
    if(DEBUG){ qDebug() << " - starting playback:" << filepath; }
    mediaObj->setVolume(100);
    mediaObj->setMedia(QUrl::fromLocalFile(filepath));
    mediaObj->play();
    LSession::processEvents();
  }
  if(DEBUG){ qDebug() << " - Done with Audio File"; }
}

//================
//  PRIVATE SLOTS
//================
void LSession::NewCommunication(QStringList list){
  if(DEBUG){ qDebug() << "New Communications:" << list; }
  for(int i=0; i<list.length(); i++){
    if(list[i]=="--logout"){
      QTimer::singleShot(0, this, SLOT(StartLogout()) );
    }else if(list[i]=="--lock-session"){
      Lumina::SS->LockScreenNow();
    }
  }
}

void LSession::launchStartupApps(){
  //First start any system-defined startups, then do user defined
  qDebug() << "Launching startup applications";

  //Enable Numlock
  if(LUtils::isValidBinary("numlockx")){ //make sure numlockx is installed
    if(DesktopSettings::instance()->value(DesktopSettings::System,"EnableNumlock",false).toBool()){
      QProcess::startDetached("numlockx on");
    }else{
      QProcess::startDetached("numlockx off");
    }
  }
  /*int tmp = LOS::ScreenBrightness();
  if(tmp>0){
    LOS::setScreenBrightness( tmp );
    qDebug() << " - - Screen Brightness:" << QString::number(tmp)+"%";
  }
  //ExternalProcess::launch("nice lumina-open -autostart-apps");

  //Re-load the screen brightness and volume settings from the previous session
  // Wait until after the XDG-autostart functions, since the audio system might be started that way
  qDebug() << " - Loading previous settings";
  tmp = LOS::audioVolume();
  LOS::setAudioVolume(tmp);
  qDebug() << " - - Audio Volume:" << QString::number(tmp)+"%";
  */
  //Now play the login music since we are finished
  if(DesktopSettings::instance()->value(DesktopSettings::System,"PlayStartupAudio",true).toBool()){
    //Make sure to re-set the system volume to the last-used value at outset
    /*int vol = LOS::audioVolume();
    if(vol>=0){ LOS::setAudioVolume(vol); }*/
    LSession::playAudioFile(LOS::LuminaShare()+"Login.ogg");
  }
  qDebug() << " - Finished with startup routines";
}

void LSession::checkUserFiles(){
  //internal version conversion examples:
  //  [1.0.0 -> 1000000], [1.2.3 -> 1002003], [0.6.1 -> 6001]
  /*QString OVS = DesktopSettings::instance()->value(DesktopSettings::System,"DesktopVersion","0").toString(); //Old Version String
  bool changed = LDesktopUtils::checkUserFiles(OVS);
  if(changed){
    //Save the current version of the session to the settings file (for next time)
    DesktopSettings::instance()->setValue(DesktopSettings::System,"DesktopVersion", this->applicationVersion());
  }*/
}


//==================
//  PUBLIC SLOTS
//==================
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

void LSession::LaunchApplication(QString exec){
  qDebug() << "Launch Application:" << exec;
  ExternalProcess::launch(exec);
}

void LSession::LaunchDesktopApplication(QString app, QString action){
  qDebug() << "Launch Desktop Application:" << app << action;
  XDGDesktop *xdg = Lumina::APPLIST->findAppFile(app);
  bool cleanup = false;
  if(xdg==0){
    xdg = new XDGDesktop(app);
    cleanup = true;
  }
  if(xdg->isValid()){
    QString exec = xdg->generateExec(QStringList(), action);
    ExternalProcess::launch(exec, QStringList(), xdg->startupNotify);
  }

  if(cleanup && xdg!=0){ xdg->deleteLater(); }
}

void LSession::LaunchStandardApplication(QString app, QStringList args){
  qDebug() << "Launch Standard Application:" << app << args;
  //Find/replace standardized apps with thier mimetypes
  if(app.startsWith("--")){ app = "application/"+app.section("--",-1).simplified(); }
  //First see if this is a mimetype with a default application
  if(app.count("/")==1 && !app.startsWith("/")){
    QString mimeapp = XDGMime::findDefaultAppForMime(app);
    if(!mimeapp.isEmpty()){ app = mimeapp; }
  }
  if(!app.endsWith(".desktop")){
    //actual command/binary - just launch it
    ExternalProcess::launch(app, args, false); // do not use startup notify cursor
  }else{
    //Get the XDGDesktop structure
    XDGDesktop *desk = 0; bool cleanup = false;
    if(app.startsWith("/") && QFile::exists(app)){ desk = new XDGDesktop(app); cleanup = true; }
    if(desk==0 || !desk->isValid()){
      //Need to find the app within the current list
      if(cleanup){ desk->deleteLater(); desk = 0; cleanup = false; }
      app = app.section("/",-1); //make sure this is a relative path
      desk = Lumina::APPLIST->findAppFile(app);
    }
    if(desk!=0 && desk->isValid()){
      //Got the application - go ahead and assemble the startup command
      QString exec = desk->generateExec(args); //args are embedded into the exec command as needed
      ExternalProcess::launch(exec, QStringList(), desk->startupNotify);
    }
    if(cleanup){ desk->deleteLater(); }
  }

}

void LSession::reloadIconTheme(){
  //Wait a moment for things to settle before sending out the signal to the interfaces
  QApplication::processEvents();
  QApplication::processEvents();
  emit IconThemeChanged();
}

//Temporarily change the session locale (nothing saved between sessions)
void LSession::switchLocale(QString localeCode){
  currTranslator = LUtils::LoadTranslation(this, "lumina-desktop", localeCode, currTranslator);
  if(currTranslator!=0 || localeCode=="en_US"){
    LUtils::setLocaleEnv(localeCode); //will set everything to this locale (no custom settings)
  }
  emit LocaleChanged();
}
