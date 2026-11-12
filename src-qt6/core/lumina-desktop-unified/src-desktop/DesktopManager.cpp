//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017-2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "DesktopManager.h"

#include "global-objects.h"

#ifdef USE_WIDGETS
#include "src-widgets/NativeWindow.h"
#endif

// === PUBLIC ===
DesktopManager::DesktopManager(){

}

DesktopManager::~DesktopManager(){

}

void DesktopManager::start(){
  connect(DesktopSettings::instance(), SIGNAL(FileModified(DesktopSettings::File)), this, SLOT(settingsChanged(DesktopSettings::File)) );
  //Perform the initial load of the settings files
  QTimer::singleShot(0, this, SLOT(updateSessionSettings()) );
  QTimer::singleShot(0, this, SLOT(updateDesktopSettings()) );
  QTimer::singleShot(0, this, SLOT(updatePanelSettings()) );
  QTimer::singleShot(0, this, SLOT(updatePluginSettings()) );
  QTimer::singleShot(0, this, SLOT(updateMenuSettings()) );
  QTimer::singleShot(0, this, SLOT(updateAnimationSettings()) );
}

void DesktopManager::stop(){
  disconnect(DesktopSettings::instance(), SIGNAL(FileModified(DesktopSettings::File)), this, SLOT(settingsChanged(DesktopSettings::File)) );
}

// === PRIVATE ===
void DesktopManager::updateWallpaper(QString screen_id, int wkspace){
  QString current = RootDesktopObject::instance()->CurrentWallpaper(screen_id);
  if(!current.isEmpty()){ current = QUrl(current).toLocalFile(); } //convert it back to the normal file syntax
  //First find the list of options from the settings
  //First look for a list that matches this exact screen/workspace combo
  QStringList wpaperList = DesktopSettings::instance()->value(DesktopSettings::Desktop, "wallpapers/"+screen_id+"_wk_"+QString::number(wkspace), QStringList()).toStringList();
  //Next look for a list that matches this exact workspace
  if(wpaperList.isEmpty()){ wpaperList= DesktopSettings::instance()->value(DesktopSettings::Desktop, "wallpapers/default_wk_"+QString::number(wkspace), QStringList()).toStringList(); }
  wpaperList.removeAll("");
  //Next look for a list that matches this exact screen
  if(wpaperList.isEmpty()){ wpaperList= DesktopSettings::instance()->value(DesktopSettings::Desktop, "wallpapers/"+screen_id, QStringList()).toStringList(); }
  wpaperList.removeAll("");
  //Now look for a list that matches any screen/workspace
  if(wpaperList.isEmpty()){ wpaperList= DesktopSettings::instance()->value(DesktopSettings::Desktop, "wallpapers/default", QStringList()).toStringList(); }
  wpaperList.removeAll("");
  //Now use the failover wallpaper directory
  if(wpaperList.isEmpty()){
    QString def = LOS::LuminaShare().section("/",0,-3)+"/wallpapers/lumina-nature"; //Note: LuminaShare() ends with an extra "/"
    //qDebug() << "Default Wallpaper:" << def;
    if(QFile::exists(def)){ wpaperList << def; }
  }
  //qDebug() << "Got wallpaper list:" << screen_id << wkspace << wpaperList;
  //Wallpaper selection/randomization
  if(wpaperList.count()==1 && wpaperList.first()==current){ return; } //nothing to do - just the same image
  QString wpaper;
  QStringList bgL = wpaperList; //need a copy at the moment - could change the entire list in a second (opening a dir for instance)
  while(wpaper.isEmpty() || QFileInfo(wpaper).isDir()){
    QString prefix;
    if(!wpaper.isEmpty()){
      //Got a directory - update the list of files and re-randomize the selection
      QStringList imgs = LUtils::imageExtensions(true);
	//qDebug() << " - Got Dir: " << imgs;
      QDir tdir(wpaper);
      prefix=wpaper+"/";
      bgL = tdir.entryList(imgs, QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
      //If directory no longer has any valid images - remove it from list and try again
      if(bgL.isEmpty()){
        wpaperList.removeAll(wpaper); //invalid directory - remove it from the list for the moment
        bgL = wpaperList; //reset the list back to the original list (not within a directory)
      }
    }
    //Verify that there are files in the list - otherwise use the default
    if(bgL.isEmpty()){ wpaper.clear(); break; }
    int index = ( qrand() % bgL.length() );
    if(index== bgL.indexOf(current)){ //if the current wallpaper was selected by the randomization again
      //Go to the next in the list
      if(index < 0 || index >= bgL.length()-1){ index = 0; } //if invalid or last item in the list - go to first
      else{ index++; } //go to next
    }
    wpaper = prefix+bgL[index];
  }
  //Now go ahead and set the wallpaper in the screen object
  if(wpaper.isEmpty() || wpaper=="default"){ wpaper = LOS::LuminaShare()+"/desktop-background.jpg"; } //failover image
  //qDebug() << "Updating Wallpaper:" << screen_id << wpaper;
  RootDesktopObject::instance()->ChangeWallpaper(screen_id,QUrl::fromLocalFile(wpaper).toString() );
}

void DesktopManager::updatePlugins(QString plugin_id){

}

// === PUBLIC SLOTS ===
void DesktopManager::workspaceChanged(int wknum){
  //qDebug() << "Got Workspace Changed:" << wknum;
  syncWindowList();
}

void DesktopManager::settingsChanged(DesktopSettings::File type){
  switch(type){
	case DesktopSettings::Session:
	  QTimer::singleShot(0, this, SLOT(updateSessionSettings()) );
	case DesktopSettings::Desktop:
	  QTimer::singleShot(1, this, SLOT(updateDesktopSettings()) );
	case DesktopSettings::Panels:
	  QTimer::singleShot(2, this, SLOT(updatePanelSettings()) );
	case DesktopSettings::Plugins:
	  QTimer::singleShot(3, this, SLOT(updatePluginSettings()) );
	case DesktopSettings::ContextMenu:
	  QTimer::singleShot(4, this, SLOT(updateMenuSettings()) );
	case DesktopSettings::Animation:
	  QTimer::singleShot(5, this, SLOT(updateAnimationSettings()) );
	default:
	  break;
	  //Do nothing - not a settings change we care about here
  }
}

void DesktopManager::NewWindowAvailable(NativeWindowObject* win){
  //connect(win, SIGNAL(WindowClosed(WId)), this, SLOT(syncWindowList()) );
#ifdef USE_WIDGETS
  qDebug() << "Got New Widget Window:" << win->name();
  NativeWindow *tmp = new NativeWindow(win);
  //qDebug() << " - Embed Into frame:" << tmp->relativeOrigin();
  //Lumina::NWS->RequestReparent(win->id(), win->frameId(), tmp->relativeOrigin());
  QTimer::singleShot(10, tmp, SLOT(initProperties()) );
#endif
  syncWindowList();
}

void DesktopManager::NewTrayWindowAvailable(NativeWindowObject* win){
  //connect(win, SIGNAL(WindowClosed(WId)), this, SLOT(syncTrayWindowList()) );
  syncTrayWindowList();
}

void DesktopManager::syncWindowList(){
  QList<NativeWindowObject*> allWins = Lumina::NWS->currentWindows();
  //Filter out all the windows not in the current workspace
  QList<NativeWindowObject*> current;
  QList<NativeWindowObject*> currentStacked;
  int wkspace = Lumina::NWS->currentWorkspace();
  for(int i=0; i<allWins.length(); i++){
    if(allWins[i]->isSticky() || (allWins[i]->workspace() == wkspace)){
      current << allWins[i];
    }
  }
  //qDebug() << "Synced Window List:" << current.length();
  RootDesktopObject::instance()->setWindows(current);
}

void DesktopManager::syncTrayWindowList(){
  QList<NativeWindowObject*> allWins = Lumina::NWS->currentTrayWindows();
  //qDebug() << "Synced Tray Window List:" << allWins.length();
  RootDesktopObject::instance()->setTrayWindows(allWins);
}

// === PRIVATE SLOTS ===
void DesktopManager::updateSessionSettings(){
  //qDebug() << "Update Session Settings...";

  RootDesktopObject::instance()->updateCurrentTimeFormat(DesktopSettings::instance()->value(DesktopSettings::Session, "datetime_format", "").toString());
}

void DesktopManager::updateDesktopSettings(){
  //qDebug() << "Update Desktop Settings...";
  QList<QScreen*> scrns = QGuiApplication::screens();
  int wkspace = Lumina::NWS->currentWorkspace();
  for(int i=0; i<scrns.length(); i++){ updateWallpaper(scrns[i]->name(), wkspace); }

}

void DesktopManager::updatePanelSettings(){
  QList<QScreen*> scrns = QGuiApplication::screens();
  int primary = QApplication::desktop()->primaryScreen();
  //qDebug() << "Panel  Settings Changed:" << primary << scrns.length();
  QStringList total_ids;
  for(int i=0; i<scrns.length(); i++){
    //qDebug() << " - Check Screen Name:" << scrns[i]->name();
    ScreenObject *sObj = RootDesktopObject::instance()->screen(scrns[i]->name());
    if(sObj == 0){ continue; } //screen is not managed directly - skip it
    QStringList ids = DesktopSettings::instance()->value(DesktopSettings::Panels, scrns[i]->name().replace("-","_")+"/active_ids", QStringList()).toStringList();
    if(ids.isEmpty() && (scrns.length()==1 || i==primary)){
      //qDebug() << "  --  PRIMARY";
      //Also look for the "default" panel id's for the primary/default screen
      ids = DesktopSettings::instance()->value(DesktopSettings::Panels, "default/active_ids", QStringList()).toStringList();
    }
    ids.removeAll("");
    //qDebug() << " -- settings:" << ids;
    for(int j=0; j<ids.length(); j++){
      total_ids << scrns[i]->name()+"/"+ids[j];
    }
  }
  //Now do the global-session panels
  QStringList ids = DesktopSettings::instance()->value(DesktopSettings::Panels, "session/active_ids", QStringList()).toStringList();
  ids.removeAll("");
  for(int i=0; i<ids.length(); i++){
      total_ids << "session/"+ids[i];
  }
  //qDebug() << "Panel Settings Changed:" << total_ids;
  RootDesktopObject::instance()->setPanels(total_ids); //put the new ones in place
  QTimer::singleShot(500, this, SLOT(updatePanelReservations()) );
}

void DesktopManager::updatePanelReservations(){
  /*QRect raw_session, reserved_session;
  QList<ScreenObject*> screens = RootDesktopObject::instance()->screenObjects();
  //Calculate the total session rectangle and session panel reservations
  QList<PanelObject*> tmp = RootDesktopObject::instance()->panelObjectList();
  for(int i=0; i<screens.length(); i++){
    raw_session = raw_session.united(screens[i]->geometry());
  }
  for(int i=0; i<tmp.length(); i++){
    reserved_session = 
  }
  //Go through all the panels and calculate the current screen reservations
  */


}

void DesktopManager::updatePluginSettings(){

}

void DesktopManager::updateMenuSettings(){

}

void DesktopManager::updateAnimationSettings(){

}
