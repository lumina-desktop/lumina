//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LAppMenuPlugin.h"
#include "../../LSession.h"

#include <LuminaXDG.h>

LAppMenuPlugin::LAppMenuPlugin(QWidget *parent, QString id, bool horizontal) : LPPlugin(parent, id, horizontal){
  button = new QToolButton(this);
    button->setAutoRaise(true);
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  mainmenu = new QMenu(this);
    button->setMenu( mainmenu );
    button->setPopupMode(QToolButton::InstantPopup);
    this->layout()->setContentsMargins(0,0,0,0);
    this->layout()->addWidget(button);

  connect(mainmenu, SIGNAL(aboutToHide()), this, SIGNAL(MenuClosed()));
  connect(mainmenu, SIGNAL(triggered(QAction*)), this, SLOT(LaunchItem(QAction*)) );
  connect(LSession::handle()->applicationMenu(), SIGNAL(AppMenuUpdated()), this, SLOT(UpdateMenu()));
  QTimer::singleShot(0,this, SLOT(OrientationChange())); //Update icons/sizes
  QTimer::singleShot(0,this, SLOT(UpdateMenu()) );
  //Setup the global shortcut handling for opening the start menu
  connect(QApplication::instance(), SIGNAL(StartButtonActivated()), this, SLOT(shortcutActivated()) );
  LSession::handle()->registerStartButton(this->type());
}

LAppMenuPlugin::~LAppMenuPlugin(){

}

void LAppMenuPlugin::updateButtonVisuals(){
    button->setToolTip( tr("Quickly launch applications or open files"));
    button->setText( tr("Applications") );
    //Use the TrueOS icon by default (or the Lumina icon for non-TrueOS systems)
    button->setIcon( LXDG::findIcon("start-here-lumina","Lumina-DE") );
}

// ========================
//    PRIVATE FUNCTIONS
// ========================
void LAppMenuPlugin::shortcutActivated(){
  if(LSession::handle()->registerStartButton(this->type())){
    if(button->menu()->isVisible()){ button->menu()->hide(); }
    else{ button->showMenu(); }
  }
}

void LAppMenuPlugin::LaunchItem(QAction* item){
  QString appFile = item->whatsThis();
  if(appFile.startsWith("internal::")){
    appFile = appFile.section("::",1,50); //cut off the "internal" flag
    if(appFile=="logout"){ LSession::handle()->systemWindow(); }
  }else if(!appFile.isEmpty()){
    LSession::LaunchApplication("lumina-open "+appFile);
  }	
}

void LAppMenuPlugin::UpdateMenu(){
  mainmenu->clear();
  QHash<QString, QList<XDGDesktop*> > *HASH = LSession::handle()->applicationMenu()->currentAppHash();
    //Now Re-create the menu (orignally copied from the AppMenu class)
    //Add link to the file manager
    QAction *tmpact = mainmenu->addAction( LXDG::findIcon("user-home", ""), tr("Browse Files") );
      tmpact->setWhatsThis("\""+QDir::homePath()+"\"");
    //--Look for the app store
    XDGDesktop store(LOS::AppStoreShortcut());
    if(store.isValid()){
      tmpact = mainmenu->addAction( LXDG::findIcon(store.icon, ""), tr("Install Applications") ); 
      tmpact->setWhatsThis("\""+store.filePath+"\"");
    }
    //--Look for the control panel
    XDGDesktop controlp(LOS::ControlPanelShortcut());
    if(controlp.isValid()){
      tmpact = mainmenu->addAction( LXDG::findIcon(controlp.icon, ""), tr("Control Panel") ); 
      tmpact->setWhatsThis("\""+controlp.filePath+"\"");
    }
    mainmenu->addSeparator();
    //--Now create the sub-menus
    QStringList cats = HASH->keys();
    cats.sort(); //make sure they are alphabetical
    for(int i=0; i<cats.length(); i++){
      //Make sure they are translated and have the right icons
      QString name, icon;
      if(cats[i]=="All"){continue; } //skip this listing for the menu
      else if(cats[i] == "Multimedia"){ name = tr("Multimedia"); icon = "applications-multimedia"; }
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
      else if(cats[i] == "Wine"){ name = tr("Wine"); icon = "wine"; }
      else{ name = tr("Unsorted"); icon = "applications-other"; }

      QMenu *menu = new QMenu(name, this);
      menu->setIcon(LXDG::findIcon(icon,""));
      QList<XDGDesktop*> appL = HASH->value(cats[i]);
      for( int a=0; a<appL.length(); a++){
	if(appL[a]->actions.isEmpty()){
	  //Just a single entry point - no extra actions
          QAction *act = new QAction(LXDG::findIcon(appL[a]->icon, ""), appL[a]->name, menu);
          act->setToolTip(appL[a]->comment);
          act->setWhatsThis("\""+appL[a]->filePath+"\"");
          menu->addAction(act);
	}else{
	  //This app has additional actions - make this a sub menu
	  // - first the main menu/action
	  QMenu *submenu = new QMenu(appL[a]->name, menu);
	    submenu->setIcon( LXDG::findIcon(appL[a]->icon,"") );
	      //This is the normal behavior - not a special sub-action (although it needs to be at the top of the new menu)
	      QAction *act = new QAction(LXDG::findIcon(appL[a]->icon, ""), appL[a]->name, submenu);
              act->setToolTip(appL[a]->comment);
              act->setWhatsThis(appL[a]->filePath);
	    submenu->addAction(act);
	    //Now add entries for every sub-action listed
	    for(int sa=0; sa<appL[a]->actions.length(); sa++){
              QAction *sact = new QAction(LXDG::findIcon(appL[a]->actions[sa].icon, appL[a]->icon), appL[a]->actions[sa].name, this);
              sact->setToolTip(appL[a]->comment);
              sact->setWhatsThis("-action \""+appL[a]->actions[sa].ID+"\" \""+appL[a]->filePath+"\"");
              submenu->addAction(sact);		    
	    }
	  menu->addMenu(submenu);
	}
      }//end loop over apps within this category
      mainmenu->addMenu(menu);
    } //end loop over categories
  //Now add any logout options
  mainmenu->addSeparator();
  //QMenu *tmpmenu = mainmenu->addMenu(LXDG::findIcon("system-log-out",""), tr("Leave"));
    tmpact =mainmenu->addAction(LXDG::findIcon("system-log-out"),tr("Leave"));
      tmpact->setWhatsThis("internal::logout");

}
