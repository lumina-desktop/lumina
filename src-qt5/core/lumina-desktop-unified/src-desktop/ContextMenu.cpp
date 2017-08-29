//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2012-2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "ContextMenu.h"
#include <global-objects.h>

void DesktopContextMenu::SettingsChanged(DesktopSettings::File file){
  if(file == DesktopSettings::ContextMenu){ UpdateMenu(false); }
}

void DesktopContextMenu::UpdateMenu(bool fast){
  //Put a label at the top
  unsigned int num = Lumina::NWS->currentWorkspace();
  workspaceLabel->setText( "<b>"+QString(tr("Workspace %1")).arg(QString::number(num+1))+"</b>");
  if(fast && usewinmenu){ updateWinMenu(); }
  if(fast){ return; } //already done
  this->clear(); //clear it for refresh
  this->addAction(wkspaceact);
  this->addSeparator();
  //Now load the user's menu setup and fill the menu
  QStringList items = DesktopSettings::instance()->value(DesktopSettings::ContextMenu, "itemlist", QStringList()<< "terminal" << "filemanager" << "line" << "applications" << "windowlist" << "settings" << "lockdesktop").toStringList();
  usewinmenu=false;
  for(int i=0; i<items.length(); i++){
    if(items[i]=="terminal"){ this->addAction(LXDG::findIcon("utilities-terminal",""), tr("Terminal"))->setWhatsThis("lumina-open -terminal"); }
    else if(items[i]=="lockdesktop"){ this->addAction(LXDG::findIcon("system-lock-screen",""), tr("Lock Session"), this, SIGNAL(LockSession()) ); }
    else if(items[i]=="filemanager"){ this->addAction( LXDG::findIcon("user-home",""), tr("Browse Files"))->setWhatsThis("lumina-open \""+QDir::homePath()+"\""); }
    else if(items[i]=="applications"){ this->addMenu( appMenu ); }
    else if(items[i]=="line"){ this->addSeparator(); }
    //else if(items[i]=="settings"){ this->addMenu( LSession::handle()->settingsMenu() ); }
    else if(items[i]=="windowlist"){ updateWinMenu(); this->addMenu( winMenu); usewinmenu=true; }
    else if(items[i].startsWith("app::::") && items[i].endsWith(".desktop")){
      //Custom *.desktop application
      QString file = items[i].section("::::",1,1).simplified();
      //Try to use the pre-loaded app entry for this
      XDGDesktop *xdg = XDGDesktopList::instance()->findAppFile(file);
      if(xdg!=0){ xdg->addToMenu(this); }
      else{
        XDGDesktop xdgf(file);// = LXDG::loadDesktopFile(file, ok);
        if(xdgf.type!=XDGDesktop::BAD){ xdgf.addToMenu(this); }
	  }
    }/*else if(items[i].startsWith("jsonmenu::::")){
      //Custom JSON menu system (populated on demand via external scripts/tools
      QStringList info = items[i].split("::::"); //FORMAT:[ "jsonmenu",exec,name, icon(optional)]
      if(info.length()>=3){
        qDebug() << "Custom JSON Menu Loaded:" << info;
        JsonMenu *tmp = new JsonMenu(info[1], deskMenu);
        tmp->setTitle(info[2]);
        connect(tmp, SIGNAL(triggered(QAction*)), this, SLOT(SystemApplication(QAction*)) );
        if(info.length()>=4){ tmp->setIcon( LXDG::findIcon(info[3],"") ); }
        this->addMenu(tmp);
      }
    }*/
  }
  //Now add the system quit options
  this->addSeparator();
  this->addAction(LXDG::findIcon("system-log-out",""), tr("Leave"), this, SIGNAL(showLeaveDialog()) );
}

// === PUBLIC ===
DesktopContextMenu::DesktopContextMenu(QWidget *parent) : QMenu(parent){
  if(parent!=0){
    parent->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(parent, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showMenu(const QPoint&)) );
  }
  appMenu = 0;
  winMenu = 0;
  workspaceLabel = new QLabel(0);
  wkspaceact = new QWidgetAction(0);
    wkspaceact->setDefaultWidget(workspaceLabel);
  connect(this, SIGNAL(triggered(QAction*)), this, SLOT(LaunchAction(QAction*)) );
  //Connect to a couple global objects
  connect(XDGDesktopList::instance(), SIGNAL(appsUpdated()), this, SLOT(updateAppMenu()) );
  connect(this, SIGNAL(aboutToShow()), this, SLOT(UpdateMenu()) ); //this will do a "fast" update
}

DesktopContextMenu::~DesktopContextMenu(){
  //nothing special
  //workspaceLabel->deleteLater(); //The QWidgetAction takes ownership of the label when inserted - do not manually delete
  wkspaceact->deleteLater();
}

void DesktopContextMenu::start(){
  connect(DesktopSettings::instance(), SIGNAL(FileModified(DesktopSettings::File)), this, SLOT(SettingsChanged(DesktopSettings::File)) );
  connect(this, SIGNAL(LockSession()), Lumina::SS, SLOT(LockScreenNow()) );
  UpdateMenu(false);
  //Still need to connect to some "workspaceChanged(int)" signal
}

// === PRIVATE SLOTS ===
void DesktopContextMenu::LaunchAction(QAction *act){
  //qDebug() << "Launch Action Triggered:" << act->whatsThis();
  if(act->whatsThis().isEmpty() || act->parent()!=this ){ return; }
  qDebug() << "Launch Menu Action:" << act->whatsThis();
  QString cmd = act->whatsThis();
  ExternalProcess::launch(cmd);
}

void DesktopContextMenu::showMenu(const QPoint &pt){
  this->popup(pt);
}

void DesktopContextMenu::updateAppMenu(){
  if(appMenu==0){
    appMenu = new QMenu(this);
    appMenu->setTitle( tr("Applications"));
    appMenu->setIcon( LXDG::findIcon("system-run","") );
  }
  XDGDesktopList::instance()->populateMenu(appMenu);
}

void DesktopContextMenu::updateWinMenu(){
  if(winMenu==0){
    winMenu = new QMenu(this);
    winMenu->setTitle( tr("Task Manager") );
  }
}

