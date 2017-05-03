//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012-2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LSession.h"
#include "global-objects.h"

#include "src-desktop/ContextMenu.h"

#include "BootSplash.h"
#ifndef DEBUG
#define DEBUG 1
#endif

//Initialize all the global objects to null pointers
EventFilter* Lumina::EFILTER = 0;
LScreenSaver* Lumina::SS = 0;
DesktopSettings* Lumina::SETTINGS = 0;
//Lumina::WM = 0;
QThread* Lumina::EVThread = 0;
RootWindow* Lumina::ROOTWIN = 0;
XDGDesktopList* Lumina::APPLIST = 0;
LShortcutEvents* Lumina::SHORTCUTS = 0;

LSession::LSession(int &argc, char ** argv) : LSingleApplication(argc, argv, "lumina-desktop-unified"){
  //Initialize the global objects to null pointers
  mediaObj = 0; //private object used for playing login/logout chimes
 if(this->isPrimaryProcess()){
  //Setup the global registrations
  qsrand(QDateTime::currentMSecsSinceEpoch());
  this->setApplicationName("Lumina Desktop Environment");
  this->setApplicationVersion( LDesktopUtils::LuminaDesktopVersion() );
  this->setOrganizationName("LuminaDesktopEnvironment");
  this->setQuitOnLastWindowClosed(false); //since the LDesktop's are not necessarily "window"s
  //Enable a few of the simple effects by default
  this->setEffectEnabled( Qt::UI_AnimateMenu, true);
  this->setEffectEnabled( Qt::UI_AnimateCombo, true);
  this->setEffectEnabled( Qt::UI_AnimateTooltip, true);
  //this->setAttribute(Qt::AA_UseDesktopOpenGL);
  this->setAttribute(Qt::AA_UseHighDpiPixmaps); //allow pixmaps to be scaled up as well as down

  //Now initialize the global objects (but do not start them yet)
  Lumina::EFILTER = new EventFilter(); //Need the XCB Event filter first
  Lumina::SETTINGS = new DesktopSettings();
  Lumina::SS = new LScreenSaver();
  //Lumina::WM = new LWindowManager();
  //Now put the Event Filter into it's own thread to keep things snappy
  Lumina::EVThread = new QThread();
    Lumina::EFILTER->moveToThread(Lumina::EVThread);
  Lumina::EVThread->start();
  Lumina::ROOTWIN = new RootWindow();
  Lumina::APPLIST = new XDGDesktopList(0, true); //keep this list up to date
  Lumina::SHORTCUTS = new LShortcutEvents(); //this can be moved to it's own thread eventually as well

  //Setup the basic connections between the shortcuts class and the session itself
  connect(Lumina::SHORTCUTS, SIGNAL(StartLogout()), this, SLOT(StartLogout()) );
  connect(Lumina::SHORTCUTS, SIGNAL(StartReboot()), this, SLOT(StartReboot()) );
  connect(Lumina::SHORTCUTS, SIGNAL(StartShutdown()), this, SLOT(StartShutdown()) );
  //Setup the various connections between the global classes
  // NOTE: Most of these connections will only become "active" as the global objects get started during the setupSession routine
  connect(Lumina::ROOTWIN, SIGNAL(RegisterVirtualRoot(WId)), Lumina::EFILTER, SLOT(RegisterVirtualRoot(WId)) );
  connect(Lumina::EFILTER, SIGNAL(WindowCreated(NativeWindow*)), Lumina::ROOTWIN, SLOT(NewWindow(NativeWindow*)) );
 } //end check for primary process 
}

LSession::~LSession(){
   //Clean up the global objects as needed
  if(Lumina::EFILTER!=0){ Lumina::EFILTER->deleteLater(); }
  if(Lumina::SS!=0){ Lumina::SS->deleteLater(); }
  if(Lumina::EVThread!=0){
    if(Lumina::EVThread->isRunning()){ Lumina::EVThread->quit(); }
    Lumina::EVThread->deleteLater();
  }
  if(Lumina::SETTINGS!=0){ Lumina::SETTINGS->deleteLater(); }
  if(Lumina::ROOTWIN!=0){ Lumina::ROOTWIN->deleteLater(); }
  if(Lumina::APPLIST!=0){ Lumina::APPLIST->deleteLater(); }
}

void LSession::setupSession(){
  BootSplash splash;
    splash.showScreen("init");
  qDebug() << "Initializing Session:" << QDateTime::currentDateTime().toString( Qt::SystemLocaleShortDate);;
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
  Lumina::SETTINGS->start();
  /*sessionsettings = new QSettings("lumina-desktop", "sessionsettings");
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
  }*/
  if(DEBUG){ qDebug() << " - Load Localization Files:" << timer->elapsed();}  
  currTranslator = LUtils::LoadTranslation(this, "lumina-desktop"); 
  if(DEBUG){ qDebug() << " - Start Event Filter:" << timer->elapsed(); }
  Lumina::EFILTER->start();
//use the system settings
  //Setup the user's lumina settings directory as necessary
    splash.showScreen("user");
  if(DEBUG){ qDebug() << " - Init User Files:" << timer->elapsed();}  
  //checkUserFiles(); //adds these files to the watcher as well

  //Initialize the internal variables
  //DESKTOPS.clear();
      
  //Start the background system tray
    splash.showScreen("systray");
	
  //Initialize the global menus
  qDebug() << " - Initialize system menus";
    splash.showScreen("apps");
  if(DEBUG){ qDebug() << " - Populate App List:" << timer->elapsed();}
  Lumina::APPLIST->updateList();
  //appmenu = new AppMenu();
  
    splash.showScreen("menus");
  //if(DEBUG){ qDebug() << " - Init SettingsMenu:" << timer->elapsed();}
  //settingsmenu = new SettingsMenu();
  //if(DEBUG){ qDebug() << " - Init SystemWindow:" << timer->elapsed();}
  //sysWindow = new SystemWindow();
  
  //Initialize the desktops
    splash.showScreen("desktop");
  if(DEBUG){ qDebug() << " - Init Desktops:" << timer->elapsed(); }
  QList<QScreen*> scrns= QApplication::screens();
  for(int i=0; i<scrns.length(); i++){
    qDebug() << "   --- Load Wallpaper for Screen:" << scrns[i]->name();
    Lumina::ROOTWIN->ChangeWallpaper(scrns[i]->name(), RootWindow::Stretch, LOS::LuminaShare()+"desktop-background.jpg");
  }
  Lumina::ROOTWIN->start();
  DesktopContextMenu *cmenu = new DesktopContextMenu(Lumina::ROOTWIN);
  connect(cmenu, SIGNAL(showLeaveDialog()), this, SLOT(StartLogout()) );
  cmenu->start();

  //desktopFiles = QDir(QDir::homePath()+"/Desktop").entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs, QDir::Name | QDir::IgnoreCase | QDir::DirsFirst);
  //updateDesktops();
  //for(int i=0; i<6; i++){ LSession::processEvents(); } //Run through this a few times so the interface systems get up and running

  //Now setup the system watcher for changes
    splash.showScreen("final");
  //if(DEBUG){ qDebug() << " - Init QFileSystemWatcher:" << timer->elapsed();}
  /*watcher = new QFileSystemWatcher(this);
    QString confdir = sessionsettings->fileName().section("/",0,-2);
    watcherChange(sessionsettings->fileName() );
    watcherChange( confdir+"/desktopsettings.conf" );
    watcherChange( confdir+"/fluxbox-init" );
    watcherChange( confdir+"/fluxbox-keys" );
    watcherChange( confdir+"/favorites.list" );
    //Try to watch the localized desktop folder too
    if(QFile::exists(QDir::homePath()+"/"+tr("Desktop"))){ watcherChange( QDir::homePath()+"/"+tr("Desktop") ); }
    watcherChange( QDir::homePath()+"/Desktop" );*/

  //connect internal signals/slots
  //connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(watcherChange(QString)) );
  //connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(watcherChange(QString)) );
  //connect(this, SIGNAL(aboutToQuit()), this, SLOT(SessionEnding()) );
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
  LOS::setAudioVolume( LOS::audioVolume() ); //make sure the audio volume is saved in the backend for the next login
  bool playaudio = Lumina::SETTINGS->value(DesktopSettings::Session,"PlayLogoutAudio",true).toBool();
  if( playaudio ){ playAudioFile(LOS::LuminaShare()+"Logout.ogg"); }
  //Now perform any other cleanup
  Lumina::EFILTER->stop();
  //Now wait a moment for things to close down before quitting
  if(playaudio){
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
    if(Lumina::SETTINGS->value(DesktopSettings::System,"EnableNumlock",false).toBool()){
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
  //ExternalProcess::launch("nice lumina-open -autostart-apps");
  
  //Re-load the screen brightness and volume settings from the previous session
  // Wait until after the XDG-autostart functions, since the audio system might be started that way
  qDebug() << " - Loading previous settings";
  tmp = LOS::audioVolume();
  LOS::setAudioVolume(tmp);
  qDebug() << " - - Audio Volume:" << QString::number(tmp)+"%";
  
  //Now play the login music since we are finished
  if(Lumina::SETTINGS->value(DesktopSettings::System,"PlayStartupAudio",true).toBool()){
    //Make sure to re-set the system volume to the last-used value at outset
    int vol = LOS::audioVolume();
    if(vol>=0){ LOS::setAudioVolume(vol); }
    LSession::playAudioFile(LOS::LuminaShare()+"Login.ogg");
  }
  qDebug() << " - Finished with startup routines";
}

void LSession::checkUserFiles(){
  //internal version conversion examples: 
  //  [1.0.0 -> 1000000], [1.2.3 -> 1002003], [0.6.1 -> 6001]
  QString OVS = Lumina::SETTINGS->value(DesktopSettings::System,"DesktopVersion","0").toString(); //Old Version String
  bool changed = LDesktopUtils::checkUserFiles(OVS);
  if(changed){
    //Save the current version of the session to the settings file (for next time)
    Lumina::SETTINGS->setValue(DesktopSettings::System,"DesktopVersion", this->applicationVersion());
  }
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
