//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012-2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LSession.h"
#include "global-objects.h"

#include "BootSplash.h"

#ifndef DEBUG
#define DEBUG 0
#endif

LSession::LSession(int &argc, char ** argv) : LSingleApplication(argc, argv, "lumina-desktop"){
  //Initialize the global objects to null pointers
  mediaObj = 0; //private object used for playing login/logout chimes
  Lumina::SYSTEM = 0;
  Lumina::EFILTER = 0;
  Lumina::SS = 0;
  Lumina::WM = 0;
  Lumina::EVThread = 0;

 if(this->isPrimaryProcess()){
  //Setup the global registrations
  this->setApplicationName("Lumina Desktop Environment");
  this->setApplicationVersion( LDesktopUtils::LuminaDesktopVersion() );
  this->setOrganizationName("LuminaDesktopEnvironment");
  this->setQuitOnLastWindowClosed(false); //since the LDesktop's are not necessarily "window"s
  //Enable a few of the simple effects by default
  this->setEffectEnabled( Qt::UI_AnimateMenu, true);
  this->setEffectEnabled( Qt::UI_AnimateCombo, true);
  this->setEffectEnabled( Qt::UI_AnimateTooltip, true);
  //this->setAttribute(Qt::AA_UseDesktopOpenGL);
  //this->setAttribute(Qt::AA_UseHighDpiPixmaps); //allow pixmaps to be scaled up as well as down

  //Now initialize the global objects (but do not start them yet)
  Lumina::SYSTEM = new LXCB(); //need access to XCB data/functions right away
  Lumina::EFILTER = new EventFilter(); //Need the XCB Event filter 
  Lumina::SS = new LScreenSaver();
  Lumina::WM = new LWindowManager();
  //Now put the Event Filter into it's own thread to keep things snappy
  Lumina::EVThread = new QThread();
    Lumina::EFILTER->moveToThread(Lumina::EVThread);


 } //end check for primary process 
}

LSession::~LSession(){
   //Clean up the global objects as needed
   if(Lumina::SYSTEM!=0){ Lumina::SYSTEM->deleteLater(); }

}

void LSession::setupSession(){
  BootSplash splash;
    splash.showScreen("init");
  qDebug() << "Initializing Session";
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
  currTranslator = LUtils::LoadTranslation(this, "lumina-desktop"); 
//use the system settings
  //Setup the user's lumina settings directory as necessary
    splash.showScreen("user");
  if(DEBUG){ qDebug() << " - Init User Files:" << timer->elapsed();}  
  checkUserFiles(); //adds these files to the watcher as well

  //Initialize the internal variables
  //DESKTOPS.clear();
      
  //Start the background system tray
    splash.showScreen("systray");
  if(DEBUG){ qDebug() << " - Init System Tray:" << timer->elapsed();}
  //startSystemTray();
	
  //Initialize the global menus
  qDebug() << " - Initialize system menus";
    splash.showScreen("apps");
  if(DEBUG){ qDebug() << " - Init AppMenu:" << timer->elapsed();}
  //appmenu = new AppMenu();
  
    splash.showScreen("menus");
  if(DEBUG){ qDebug() << " - Init SettingsMenu:" << timer->elapsed();}
  //settingsmenu = new SettingsMenu();
  if(DEBUG){ qDebug() << " - Init SystemWindow:" << timer->elapsed();}
  //sysWindow = new SystemWindow();
  
  //Initialize the desktops
    splash.showScreen("desktop");
  if(DEBUG){ qDebug() << " - Init Desktops:" << timer->elapsed();}
  //desktopFiles = QDir(QDir::homePath()+"/Desktop").entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs, QDir::Name | QDir::IgnoreCase | QDir::DirsFirst);
  //updateDesktops();
  //for(int i=0; i<6; i++){ LSession::processEvents(); } //Run through this a few times so the interface systems get up and running

  //Now setup the system watcher for changes
    splash.showScreen("final");
  qDebug() << " - Initialize file system watcher";
  if(DEBUG){ qDebug() << " - Init QFileSystemWatcher:" << timer->elapsed();}
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
  if(DEBUG){ qDebug() << " - Init Finished:" << timer->elapsed(); delete timer;}
  //for(int i=0; i<4; i++){ LSession::processEvents(); } //Again, just a few event loops here so thing can settle before we close the splash screen
  //launchStartupApps();
  QTimer::singleShot(500, this, SLOT(launchStartupApps()) );
  splash.hide();
  LSession::processEvents();
  splash.close(); 
  LSession::processEvents();
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
  bool playaudio = sessionsettings->value("PlayLogoutAudio",true).toBool();
  if( playaudio ){ playAudioFile(LOS::LuminaShare()+"Logout.ogg"); }
  //Stop the background system tray (detaching/closing apps as necessary)
  //stopSystemTray(!cleansession);
  //Now perform any other cleanup
  if(cleansession){
    //Close any open windows
    //qDebug() << " - Closing any open windows";
    /*QList<WId> WL = XCB->WindowList(true);
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
    }*/
  }
  //evFilter->StopEventHandling();
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
    /*if(list[i]=="--check-geoms"){
      screensChanged();
    }else if(list[i]=="--show-start"){ 
      emit StartButtonActivated();
    }*/
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

void LSession::checkUserFiles(){
  //internal version conversion examples: 
  //  [1.0.0 -> 1000000], [1.2.3 -> 1002003], [0.6.1 -> 6001]
  QString OVS = sessionsettings->value("DesktopVersion","0").toString(); //Old Version String
  bool changed = LDesktopUtils::checkUserFiles(OVS);
  if(changed){
    //Save the current version of the session to the settings file (for next time)
    sessionsettings->setValue("DesktopVersion", this->applicationVersion());
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
