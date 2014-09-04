//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012-2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LSession.h"

#include <Phonon/MediaObject>
#include <Phonon/AudioOutput>
#include <QThread>

//X includes (these need to be last due to Qt compile issues)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>

//Private/global variables (for static function access)
//static WId LuminaSessionTrayID;
static AppMenu *appmenu;
static SettingsMenu *settingsmenu;
static QTranslator *currTranslator;
static Phonon::MediaObject *mediaObj;
static Phonon::AudioOutput *audioOut;
static QThread *audioThread;
static QSettings *sessionsettings;

LSession::LSession(int &argc, char ** argv) : QApplication(argc, argv){
  this->setApplicationName("Lumina Desktop Environment");
  this->setApplicationVersion("0.6.2");
  this->setOrganizationName("LuminaDesktopEnvironment");
  this->setQuitOnLastWindowClosed(false); //since the LDesktop's are not necessarily "window"s
  //Enabled a few of the simple effects by default
  this->setEffectEnabled( Qt::UI_AnimateMenu, true);
  this->setEffectEnabled( Qt::UI_AnimateCombo, true);
  this->setEffectEnabled( Qt::UI_AnimateTooltip, true);
  this->setStyle( new MenuProxyStyle); //QMenu icon size override
  //LuminaSessionTrayID = 0;
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
  delete mediaObj;
  delete audioOut;
}

void LSession::setupSession(){
  qDebug() << "Initializing Session";
  //Load the stylesheet
  loadStyleSheet();
  //Setup the QSettings default paths
  QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, QDir::homePath()+"/.lumina");
  sessionsettings = new QSettings("LuminaDE", "sessionsettings");
  //Setup the user's lumina settings directory as necessary
  checkUserFiles(); //adds these files to the watcher as well

  //Initialize the internal variables
  DESKTOPS.clear();
	
  //Launch Fluxbox
  qDebug() << " - Launching Fluxbox";
  WM = new WMProcess();
    WM->startWM();
	
  //Initialize the desktops
  updateDesktops();
	
  //Initialize the global menus
  qDebug() << " - Initialize system menus";
  appmenu = new AppMenu();
  settingsmenu = new SettingsMenu();

  /*//Setup the audio output systems for the desktop
  qDebug() << " - Initialize audio systems";
  mediaObj = new Phonon::MediaObject(0);
  audioOut = new Phonon::AudioOutput(Phonon::MusicCategory,0);
  qDebug() << " -- Initialize new audio thread";
  audioThread = new QThread(this);
  if(mediaObj && audioOut){  //in case Phonon errors for some reason
    qDebug() << " -- Create path between audio objects";
    Phonon::createPath(mediaObj, audioOut);
    qDebug() << " -- Move audio objects to separate thread";
    mediaObj->moveToThread(audioThread);
    audioOut->moveToThread(audioThread);
  }*/
    
  //Now setup the system watcher for changes
  qDebug() << " - Initialize file system watcher";
  watcher = new QFileSystemWatcher(this);
    watcher->addPath( QDir::homePath()+"/.lumina/stylesheet.qss" );
    //watcher->addPath( QDir::homePath()+"/.lumina/LuminaDE/desktopsettings.conf" );
    watcher->addPath( QDir::homePath()+"/.lumina/fluxbox-init" );
    watcher->addPath( QDir::homePath()+"/.lumina/fluxbox-keys" );
    
  //connect internal signals/slots
  connect(this->desktop(), SIGNAL(screenCountChanged(int)), this, SLOT(updateDesktops()) );
  connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(watcherChange(QString)) );
  connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(watcherChange(QString)) );
  connect(this, SIGNAL(aboutToQuit()), this, SLOT(SessionEnding()) );
}

bool LSession::LoadLocale(QString langCode){
    QTranslator translator;
    if ( ! QFile::exists(SYSTEM::installDir()+"i18n/lumina-desktop_" + langCode + ".qm" ) )  langCode.truncate(langCode.indexOf("_"));
    bool ok = translator.load( QString("lumina-desktop_") + langCode, SYSTEM::installDir()+"i18n/" );
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
}

void LSession::launchStartupApps(){
  //First start any system-defined startups, then do user defined
  qDebug() << "Launching startup applications";
  for(int i=0; i<2; i++){
    QString startfile;
    if(i==0){startfile = "/usr/local/share/Lumina-DE/startapps"; }
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
    LSession::playAudioFile("/usr/local/share/Lumina-DE/Login.ogg");
  }
  if(sessionsettings->value("EnableNumlock",true).toBool()){
    QProcess::startDetached("numlockx on");
  }
}

void LSession::watcherChange(QString changed){
  qDebug() << "Session Watcher Change:" << changed;
  if(changed.endsWith("stylesheet.qss")){ loadStyleSheet(); }
  else if(changed.endsWith("fluxbox-init") || changed.endsWith("fluxbox-keys")){ refreshWindowManager(); }
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
    if(QFile::exists(SYSTEM::installDir()+"desktopsettings.conf")){
      if( QFile::copy(SYSTEM::installDir()+"desktopsettings.conf", dset) ){
        QFile::setPermissions(dset, QFile::ReadUser | QFile::WriteUser | QFile::ReadOwner | QFile::WriteOwner);
      }
    }
  }
  //Check for the default applications file for lumina-open
  dset = QDir::homePath()+"/.lumina/LuminaDE/lumina-open.conf";
  if(!QFile::exists(dset)){
    firstrun = true;
    if(QFile::exists(SYSTEM::installDir()+"defaultapps.conf")){
      if( QFile::copy(SYSTEM::installDir()+"defaultapps.conf", dset) ){
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
    if(QFile::exists(dset+"fluxbox-init")){ QFile::remove(dset+"fluxbox-init"); }
    if(QFile::exists(dset+"fluxbox-keys")){ QFile::remove(dset+"fluxbox-keys"); }
    QFile::copy(":/fluxboxconf/fluxbox-init-rc", dset+"fluxbox-init");
    QFile::copy(":/fluxboxconf/fluxbox-keys", dset+"fluxbox-keys");
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

void LSession::loadStyleSheet(){
  QString ss = QDir::homePath()+"/.lumina/stylesheet.qss";
  if(!QFile::exists(ss)){ ss = SYSTEM::installDir()+"stylesheet.qss"; }
  if(!QFile::exists(ss)){ return; } //no default stylesheet on the system
  //Now read/apply the custom stylesheet
  QFile file(ss);
  if( file.open(QIODevice::ReadOnly | QIODevice::Text) ){
    QTextStream in(&file);
    QString sheet = in.readAll();
    file.close();
    //Now fix/apply the sheet
      sheet.replace("\n"," "); //make sure there are no newlines
    this->setStyleSheet(sheet);
  }  
}

void LSession::refreshWindowManager(){
  WM->updateWM();
}

void LSession::updateDesktops(){
  qDebug() << " - Update Desktops";
  QDesktopWidget *DW = this->desktop();
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
    //Now go through and make sure to delete any desktops for detached screens
    for(int i=0; i<DESKTOPS.length(); i++){
      if(DESKTOPS[i]->Screen() >= DW->screenCount()){
	qDebug() << " - Hide desktop on screen:" << DESKTOPS[i]->Screen();
        DESKTOPS[i]->hide();
      }else{
        DESKTOPS[i]->show();
      }
    }
}


void LSession::SessionEnding(){
  audioThread->wait(3000); //wait a max of 3 seconds for the audio thread to finish
}

bool LSession::x11EventFilter(XEvent *event){
  //Detect X Event types and send the appropriate signal(s)
  emit TrayEvent(event); //Make sure the tray also can check this event
   switch(event->type){
  // -------------------------
    case PropertyNotify:
	//qDebug() << "Property Event:";
 	  if(event->xproperty.atom == XInternAtom(QX11Info::display(),"_NET_CLIENT_LIST",false) \
	  || event->xproperty.atom == XInternAtom(QX11Info::display(),"_NET_ACTIVE_WINDOW",false) \
	  || event->xproperty.atom == XInternAtom(QX11Info::display(),"_NET_WM_NAME",false) \
	  || event->xproperty.atom == XInternAtom(QX11Info::display(),"_NET_WM_VISIBLE_NAME",false) \
	  || event->xproperty.atom == XInternAtom(QX11Info::display(),"_NET_WM_ICON_NAME",false) \
	  || event->xproperty.atom == XInternAtom(QX11Info::display(),"_NET_WM_VISIBLE_ICON_NAME",false) ){
		LSession::restoreOverrideCursor(); //restore the mouse cursor back to normal (new window opened?)
		emit WindowListEvent();
	  }  
	break;
  }
  // -----------------------
  //Now continue on with the event handling (don't change it)
  return false;
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
  bool init = false;
  if(audioThread==0){   qDebug() << " - Initialize audio systems"; audioThread = new QThread(); init = true; }
  if(mediaObj==0){   qDebug() << " - Initialize Phonon media Object"; mediaObj = new Phonon::MediaObject(); init = true;}
  //if(audioOut==0){   qDebug() << " - Initialize Phonon audio output"; audioOut = new Phonon::AudioOutput(); init=true;}
  if(mediaObj && audioOut && init){  //in case Phonon errors for some reason
    qDebug() << " -- Create path between audio objects";
    Phonon::createPath(mediaObj, audioOut);
    qDebug() << " -- Move audio objects to separate thread";
    mediaObj->moveToThread(audioThread);
    audioOut->moveToThread(audioThread);
  }
  if(mediaObj !=0 && audioOut!=0){
    mediaObj->setCurrentSource(QUrl(filepath));
    mediaObj->play();
    audioThread->start();
  }
}
