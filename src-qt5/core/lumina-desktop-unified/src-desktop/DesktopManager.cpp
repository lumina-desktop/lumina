//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017-2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "DesktopManager.h"

#include "global-objects.h"

// === PUBLIC ===
DesktopManager::DesktopManager(){

}

DesktopManager::~DesktopManager(){

}

void DesktopManager::start(){
  connect(DesktopSettings::instance(), SIGNAL(FileModified(DesktopSettings::File)), this, SLOT(settingsChanged(DesktopSettings::File)) );
  //Perform the initial load of the settings files
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
  //Next look for a list that matches this exact screen
  if(wpaperList.isEmpty()){ wpaperList= DesktopSettings::instance()->value(DesktopSettings::Desktop, "wallpapers/"+screen_id, QStringList()).toStringList(); }
  //Now look for a list that matches any screen/workspace
  if(wpaperList.isEmpty()){ wpaperList= DesktopSettings::instance()->value(DesktopSettings::Desktop, "wallpapers/default", QStringList()).toStringList(); }
  //Now use the failover wallpaper directory
  if(wpaperList.isEmpty()){ wpaperList << LOS::LuminaShare()+"../wallpapers/lumina-nature"; }
  //Wallpaper selection/randomization
  if(wpaperList.count()==1 && wpaperList.first()==current){ return; } //nothing to do - just the same image
  QString wpaper;
  QStringList bgL = wpaperList; //need a copy at the moment - could change the entire list in a second (opening a dir for instance)
  while(wpaper.isEmpty() || QFileInfo(wpaper).isDir()){
    QString prefix;
    if(!wpaper.isEmpty()){
      //Got a directory - update the list of files and re-randomize the selection
      QStringList imgs = LUtils::imageExtensions(true);
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
    if(bgL.isEmpty()){ wpaper="default"; break; }
    int index = ( qrand() % bgL.length() );
    if(index== bgL.indexOf(current)){ //if the current wallpaper was selected by the randomization again
      //Go to the next in the list
      if(index < 0 || index >= bgL.length()-1){ index = 0; } //if invalid or last item in the list - go to first
      else{ index++; } //go to next
    }
    wpaper = prefix+bgL[index];
  }
  //Now go ahead and set the wallpaper in the screen object
  if(wpaper.isEmpty() || wpaper=="default"){ wpaper = LOS::LuminaShare()+"desktop-background.jpg"; } //failover image
  qDebug() << "Updating Wallpaper:" << screen_id << wpaper;
  RootDesktopObject::instance()->ChangeWallpaper(screen_id,QUrl::fromLocalFile(wpaper).toString() );
}

void DesktopManager::updatePanels(QString panel_id){

}

void DesktopManager::updatePlugins(QString plugin_id){

}

// === PUBLIC SLOTS ===
void DesktopManager::workspaceChanged(int wknum){
  qDebug() << "Got Workspace Changed:" << wknum;

}

void DesktopManager::settingsChanged(DesktopSettings::File type){
  switch(type){
	case DesktopSettings::Desktop:
	  QTimer::singleShot(0, this, SLOT(updateDesktopSettings()) );
	case DesktopSettings::Panels:
	  QTimer::singleShot(0, this, SLOT(updatePanelSettings()) );
	case DesktopSettings::Plugins:
	  QTimer::singleShot(0, this, SLOT(updatePluginSettings()) );
	case DesktopSettings::ContextMenu:
	  QTimer::singleShot(0, this, SLOT(updateMenuSettings()) );
	case DesktopSettings::Animation:
	  QTimer::singleShot(0, this, SLOT(updateAnimationSettings()) );
	default:
	  break;
	  //Do nothing - not a settings change we care about here
  }
}

// === PRIVATE SLOTS ===
void DesktopManager::updateDesktopSettings(){
  qDebug() << "Update Desktop Settings...";
  QList<QScreen*> scrns= QApplication::screens();
  int wkspace = Lumina::NWS->currentWorkspace();
  for(int i=0; i<scrns.length(); i++){ updateWallpaper(scrns[i]->name(), wkspace); }

}

void DesktopManager::updatePanelSettings(){

}

void DesktopManager::updatePluginSettings(){

}

void DesktopManager::updateMenuSettings(){

}

void DesktopManager::updateAnimationSettings(){

}

