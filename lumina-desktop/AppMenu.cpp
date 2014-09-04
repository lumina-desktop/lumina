//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "AppMenu.h"
#include "LSession.h"

AppMenu::AppMenu(QWidget* parent) : QMenu(parent){
  appstorelink = "/usr/local/share/applications/softmanager.desktop"; //Default application "store" to display (AppCafe in PC-BSD)
  controlpanellink = "/usr/local/share/applications/pccontrol.desktop"; //Default control panel
  APPS.clear();
  watcher = new QFileSystemWatcher(this);
    connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(watcherUpdate()) );
  QTimer::singleShot(200, this, SLOT(start()) ); //Now start filling the menu
  this->setTitle(tr("Applications"));
  this->setIcon( LXDG::findIcon("system-run","") );
}

AppMenu::~AppMenu(){

}

QHash<QString, QList<XDGDesktop> >* AppMenu::currentAppHash(){
  return &APPS;
}

//===========
//  PRIVATE
//===========
void AppMenu::updateAppList(){
  this->clear();
  APPS.clear();
  APPS = LXDG::sortDesktopCats( LXDG::systemDesktopFiles() );
  //Now fill the menu
  bool ok; //for checking inputs
    //Add link to the file manager
    this->addAction( LXDG::findIcon("user-home", ""), tr("Open Home"), this, SLOT(launchFileManager()) );
    //--Look for the app store
    XDGDesktop store = LXDG::loadDesktopFile(appstorelink, ok);
    if(ok){ 
      this->addAction( LXDG::findIcon(store.icon, ""), tr("Install Applications"), this, SLOT(launchStore()) );
    }
    //--Look for the control panel
    store = LXDG::loadDesktopFile(controlpanellink, ok);
    if(ok){ 
      this->addAction( LXDG::findIcon(store.icon, ""), tr("Control Panel"), this, SLOT(launchControlPanel()) );
    }
    this->addSeparator();
    //--Now create the sub-menus
    QStringList cats = APPS.keys();
    cats.sort(); //make sure they are alphabetical
    for(int i=0; i<cats.length(); i++){
      //Make sure they are translated and have the right icons
      QString name, icon;
      if(cats[i] == "Multimedia"){ name = tr("Multimedia"); icon = "applications-multimedia"; }
      else if(cats[i] == "Development"){ name = tr("Development"); icon = "applications-development"; }
      else if(cats[i] == "Education"){ name = tr("Education"); icon = "applications-education"; }
      else if(cats[i] == "Game"){ name = tr("Games"); icon = "applications-games"; }
      else if(cats[i] == "Graphics"){ name = tr("Graphics"); icon = "applications-graphics"; }
      else if(cats[i] == "Network"){ name = tr("Network"); icon = "applications-internet"; }
      else if(cats[i] == "Office"){ name = tr("Office"); icon = "applications-office"; }
      else if(cats[i] == "Science"){ name = tr("Science"); icon = "applications-science"; }
      else if(cats[i] == "Settings"){ name = tr("Settings"); icon = "preferences-system"; }
      else if(cats[i] == "System"){ name = tr("System"); icon = "applications-system"; }
      else if(cats[i] == "Utility"){ name = tr("Utility"); icon = "applications-utilities"; }
      else{ name = tr("Unsorted"); icon = "applications-other"; }
      
      QMenu *menu = new QMenu(name, this);
      menu->setIcon(LXDG::findIcon(icon,""));
      connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(launchApp(QAction*)) );
      QList<XDGDesktop> appL = APPS.value(cats[i]);
      for( int a=0; a<appL.length(); a++){
        QAction *act = new QAction(LXDG::findIcon(appL[a].icon, ""), appL[a].name, this);
        act->setToolTip(appL[a].comment);
        act->setWhatsThis(appL[a].filePath);
        menu->addAction(act);
      }
      this->addMenu(menu);
    }
}

//=================
//  PRIVATE SLOTS
//=================
void AppMenu::start(){
  //Setup the watcher
  watcher->addPaths(LXDG::systemApplicationDirs());
  //Now fill the menu the first time
  updateAppList();
}

void AppMenu::watcherUpdate(){
  updateAppList(); //Update the menu listings
}

void AppMenu::launchStore(){
  LSession::LaunchApplication("lumina-open \""+appstorelink+"\"");
  //QProcess::startDetached("lumina-open \""+appstorelink+"\"");
}

void AppMenu::launchControlPanel(){
  LSession::LaunchApplication("lumina-open \""+controlpanellink+"\"");
  //QProcess::startDetached("lumina-open \""+controlpanellink+"\"");
}

void AppMenu::launchFileManager(){
  LSession::LaunchApplication("lumina-fm");
  //QProcess::startDetached("lumina-fm");
}

void AppMenu::launchApp(QAction *act){
  QString appFile = act->whatsThis();
  LSession::LaunchApplication("lumina-open \""+appFile+"\"");
  //QProcess::startDetached("lumina-open \""+appFile+"\"");
}
