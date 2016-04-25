//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "StartMenu.h"
#include "ui_StartMenu.h"
//#include <QtConcurrent>

#include <LuminaOS.h>
#include "../../LSession.h"
#include <QtConcurrent>

#include "ItemWidget.h"
//#define SSAVER QString("xscreensaver-demo")

StartMenu::StartMenu(QWidget *parent) : QWidget(parent), ui(new Ui::StartMenu){
  ui->setupUi(this); //load the designer file
  this->setMouseTracking(true);
  sysapps = LSession::handle()->applicationMenu()->currentAppHash();
  connect(LSession::handle()->applicationMenu(), SIGNAL(AppMenuUpdated()), this, SLOT(UpdateApps()) );
  //Need to load the last used setting of the application list
  QString state = LSession::handle()->DesktopPluginSettings()->value("panelPlugs/systemstart/showcategories", "partial").toString();
  if(state=="partial"){ui->check_apps_showcats->setCheckState(Qt::PartiallyChecked); }
  else if(state=="true"){ ui->check_apps_showcats->setCheckState(Qt::Checked); }
  else{ ui->check_apps_showcats->setCheckState(Qt::Unchecked); }
  connect(ui->check_apps_showcats, SIGNAL(stateChanged(int)), this, SLOT(catViewChanged()) );
  UpdateAll();
  QTimer::singleShot(10, this,SLOT(UpdateApps()));
  QTimer::singleShot(10, this, SLOT(UpdateFavs()));
}

StartMenu::~StartMenu(){
	
}

void StartMenu::UpdateAll(){
  //Update all the icons/text on all the pages
	
  // Update Text
  ui->retranslateUi(this);
	
  //Update Icons
  ui->tool_goto_apps->setIcon(LXDG::findIcon("system-run",""));
  ui->tool_goto_settings->setIcon(LXDG::findIcon("preferences-system",""));
  ui->tool_launch_fm->setIcon(LXDG::findIcon("user-home",""));
  ui->tool_launch_desksettings->setIcon(LXDG::findIcon("preferences-desktop",""));
  ui->tool_lock->setIcon(LXDG::findIcon("system-lock-screen",""));
  ui->tool_goto_logout->setIcon(LXDG::findIcon("system-log-out",""));
  ui->tool_back->setIcon(LXDG::findIcon("go-previous",""));
  ui->tool_launch_deskinfo->setIcon(LXDG::findIcon("system-help",""));
	
  ui->tool_launch_mixer->setIcon( LXDG::findIcon("preferences-desktop-sound","") );
  ui->label_bright_icon->setPixmap( LXDG::findIcon("preferences-system-power-management","").pixmap(ui->tool_goto_apps->iconSize()) );
  ui->label_locale_icon->setPixmap( LXDG::findIcon("preferences-desktop-locale","").pixmap(ui->tool_goto_apps->iconSize()) );
  ui->tool_set_nextwkspace->setIcon(LXDG::findIcon("go-next-view",""));
  ui->tool_set_prevwkspace->setIcon(LXDG::findIcon("go-previous-view",""));
  ui->tool_logout->setIcon(LXDG::findIcon("system-log-out",""));
  ui->tool_restart->setIcon(LXDG::findIcon("system-reboot",""));
  ui->tool_shutdown->setIcon(LXDG::findIcon("system-shutdown",""));
  ui->tool_suspend->setIcon(LXDG::findIcon("system-suspend",""));
  
  //Update Visibility of system/session/OS options
  // -- Control Panel
  QString tmp = LOS::ControlPanelShortcut();
  if(QFile::exists(tmp)){
    ui->tool_launch_controlpanel->setWhatsThis(tmp);
    //Now read the file to see which icon to use
    bool ok = false;
    XDGDesktop desk = LXDG::loadDesktopFile(tmp, ok);
    if(ok && LXDG::checkValidity(desk)){
       ui->tool_launch_controlpanel->setIcon(LXDG::findIcon(desk.icon,"preferences-other"));
    }else{ ui->tool_launch_controlpanel->setVisible(false); }
  }else{ ui->tool_launch_controlpanel->setVisible(false); }
  // -- App Store
  tmp = LOS::AppStoreShortcut();
  if(QFile::exists(tmp)){
    ui->tool_launch_store->setWhatsThis(tmp);
    //Now read the file to see which icon to use
    bool ok = false;
    XDGDesktop desk = LXDG::loadDesktopFile(tmp, ok);
    if(ok && LXDG::checkValidity(desk)){
       ui->tool_launch_store->setIcon(LXDG::findIcon(desk.icon,"utilities-file-archiver"));
    }else{ ui->tool_launch_store->setVisible(false); }
  }else{ ui->tool_launch_store->setVisible(false); }
  //Audio Controls
  ui->frame_audio->setVisible( LOS::audioVolume() >=0 );
  //Brightness controls
  ui->frame_bright->setVisible( LOS::ScreenBrightness() >=0 );
  //Shutdown/restart
  bool ok = LOS::userHasShutdownAccess();
  ui->frame_leave_system->setWhatsThis(ok ? "allowed": "");
  ui->frame_leave_system->setVisible(ok);
  //Battery Availability
  ok = LOS::hasBattery();
  ui->label_status_battery->setWhatsThis(ok ? "allowed": "");
  ui->label_status_battery->setVisible(ok);
  //Locale availability
  QStringList locales = LUtils::knownLocales();
  ui->stackedWidget->setCurrentWidget(ui->page_main); //need to ensure the settings page is not active
  ui->combo_locale->clear();
  QString curr = LUtils::currentLocale();
  qDebug() << "Update Locales:" << locales;
  qDebug() << "Current Locale:" << curr;
  for(int i=0; i<locales.length(); i++){
    QLocale loc( (locales[i]=="pt") ? "pt_PT" : locales[i] );
    ui->combo_locale->addItem(loc.nativeLanguageName() +" ("+locales[i]+")", locales[i]); //Make the display text prettier later
    if(locales[i] == curr || locales[i] == curr.section("_",0,0) ){
      //Current Locale
      ui->combo_locale->setCurrentIndex(ui->combo_locale->count()-1); //the last item in the list right now
    }
  }
  ui->frame_locale->setVisible(locales.length() > 1);
  //User Name in status bar
  ui->label_status->setText( QString::fromLocal8Bit(getlogin()) );
  //Lumina Utilities
  ui->tool_launch_desksettings->setVisible(LUtils::isValidBinary("lumina-config"));
  ui->tool_launch_deskinfo->setVisible(LUtils::isValidBinary("lumina-info"));

}

void StartMenu::UpdateMenu(bool forceall){
  if(forceall){ UpdateAll(); }
  //Quick update routine before the menu is made visible
  UpdateFavs();
  if(ui->stackedWidget->currentWidget() != ui->page_main){
    ui->stackedWidget->setCurrentWidget(ui->page_main); //just show the main page
  }else{
    on_stackedWidget_currentChanged(0); //refresh/update the main page every time
  }
}

void StartMenu::ReLoadQuickLaunch(){
  emit UpdateQuickLaunch( LSession::handle()->sessionSettings()->value("QuicklaunchApps",QStringList()).toStringList() );  
}

void StartMenu::UpdateQuickLaunch(QString path, bool keep){
  QStringList QL = LSession::handle()->sessionSettings()->value("QuicklaunchApps",QStringList()).toStringList();
  if(keep){QL << path; }
  else{ QL.removeAll(path); }
  QL.removeDuplicates();
  LSession::handle()->sessionSettings()->setValue("QuicklaunchApps",QL);
  //LSession::handle()->sessionSettings()->sync();
  emit UpdateQuickLaunch(QL);
}

// ==========================
//        PRIVATE FUNCTIONS
// ==========================
void StartMenu::ClearScrollArea(QScrollArea *area){
  area->takeWidget()->deleteLater();
  area->setWidget( new QWidget() ); //create a new widget in the scroll area
  area->widget()->setContentsMargins(0,0,0,0);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(2);
    layout->setContentsMargins(3,1,3,1);
    layout->setDirection(QBoxLayout::TopToBottom);
    layout->setAlignment(Qt::AlignTop);
    area->widget()->setLayout(layout);
}

void StartMenu::SortScrollArea(QScrollArea *area){
  //qDebug() << "Sorting Scroll Area:";
  //Sort all the items in the scroll area alphabetically
  QLayout *lay = area->widget()->layout();
  QStringList items;
  for(int i=0; i<lay->count(); i++){
    items << lay->itemAt(i)->widget()->whatsThis();
  }
  
  items.sort();
  //qDebug() << " - Sorted Items:" << items;
  for(int i=0; i<items.length(); i++){
    if(items[i].isEmpty()){ continue; }
    //QLayouts are weird in that they can only add items to the end - need to re-insert almost every item
    for(int j=0; j<lay->count(); j++){
      //Find this item
      if(lay->itemAt(j)->widget()->whatsThis()==items[i]){
	//Found it - now move it if necessary
	//qDebug() << "Found Item:" << items[i] << i << j;
	lay->addItem( lay->takeAt(j) );
	break;
      }
    }
  }
}

// ========================
//        PRIVATE SLOTS
// ========================
void StartMenu::LaunchItem(QString path, bool fix){
  if(path.startsWith("chcat::::")){ 
    ChangeCategory(path.section("::::",1,50));
    return;
  }
  qDebug() << "Launching Item:" << path << fix;
  if(!path.isEmpty()){
    qDebug() << "Launch Application:" << path;
    if( fix && !path.startsWith("lumina-open") ){ LSession::LaunchApplication("lumina-open \""+path+"\""); }
    else{ LSession::LaunchApplication(path); }
    emit CloseMenu(); //so the menu container will close
  }
}

void StartMenu::ChangeCategory(QString cat){
  //This only happens on user interaction - make sure to run the update routine in a separate thread
  CCat = cat;
  UpdateApps();
  //QtConcurrent::run(this, &StartMenu::UpdateApps);
}

//Listing Update routines
void StartMenu::UpdateApps(){
  ClearScrollArea(ui->scroll_apps);
  //Now assemble the apps list (note: this normally happens in the background - not when it is visible/open)
  //qDebug() << "Update Apps:" << CCat << ui->check_apps_showcats->checkState();
  if(ui->check_apps_showcats->checkState() == Qt::PartiallyChecked){
    //qDebug() << " - Partially Checked";
    //Show a single page of apps, but still divided up by categories
    CCat.clear();
    QStringList cats = sysapps->keys();
    cats.sort();
    cats.removeAll("All");
    for(int c=0; c<cats.length(); c++){
      QList<XDGDesktop> apps = sysapps->value(cats[c]);
      if(apps.isEmpty()){ continue; }
      //Add the category label to the scroll
      QLabel *catlabel = new QLabel("<b>"+cats[c]+"</b>",ui->scroll_apps->widget());
        catlabel->setAlignment(Qt::AlignCenter);
      ui->scroll_apps->widget()->layout()->addWidget(catlabel);
      //Now add all the apps for this category
      for(int i=0; i<apps.length(); i++){
        ItemWidget *it = new ItemWidget(ui->scroll_apps->widget(), apps[i] );
        if(!it->gooditem){ continue; } //invalid for some reason
        ui->scroll_apps->widget()->layout()->addWidget(it);
        connect(it, SIGNAL(NewShortcut()), this, SLOT(UpdateFavs()) );
        connect(it, SIGNAL(RemovedShortcut()), this, SLOT(UpdateFavs()) );
        connect(it, SIGNAL(RunItem(QString)), this, SLOT(LaunchItem(QString)) );
        connect(it, SIGNAL(toggleQuickLaunch(QString, bool)), this, SLOT(UpdateQuickLaunch(QString, bool)) );
      }
    }
    
  }else if(ui->check_apps_showcats->checkState() == Qt::Checked){
    //qDebug() << " - Checked";
    //Only show categories to start with - and have the user click-into a cat to see apps
    if(CCat.isEmpty()){
      //No cat selected yet - show cats only
      QStringList cats = sysapps->keys();
      cats.sort();
      cats.removeAll("All"); //This is not a "real" category
      for(int c=0; c<cats.length(); c++){
	ItemWidget *it = new ItemWidget(ui->scroll_apps->widget(), cats[c], "chcat::::"+cats[c] );
        if(!it->gooditem){ continue; } //invalid for some reason
        ui->scroll_apps->widget()->layout()->addWidget(it);
        connect(it, SIGNAL(RunItem(QString)), this, SLOT(LaunchItem(QString)) );
      }
    }else{
      //qDebug() << "Show Apps For category:" << CCat;
      //Show the "go back" button
      ItemWidget *it = new ItemWidget(ui->scroll_apps->widget(), CCat, "chcat::::"+CCat, true);
        //if(!it->gooditem){ continue; } //invalid for some reason
        ui->scroll_apps->widget()->layout()->addWidget(it);
        connect(it, SIGNAL(RunItem(QString)), this, SLOT(LaunchItem(QString)) );
      //Show apps for this cat
      QList<XDGDesktop> apps = sysapps->value(CCat); 
      for(int i=0; i<apps.length(); i++){
	//qDebug() << " - App:" << apps[i].name;
        ItemWidget *it = new ItemWidget(ui->scroll_apps->widget(), apps[i] );
        if(!it->gooditem){ continue; } //invalid for some reason
        ui->scroll_apps->widget()->layout()->addWidget(it);
        connect(it, SIGNAL(NewShortcut()), this, SLOT(UpdateFavs()) );
        connect(it, SIGNAL(RemovedShortcut()), this, SLOT(UpdateFavs()) );
        connect(it, SIGNAL(RunItem(QString)), this, SLOT(LaunchItem(QString)) );
        connect(it, SIGNAL(toggleQuickLaunch(QString, bool)), this, SLOT(UpdateQuickLaunch(QString, bool)) );
      }
    }
    
  }else{
    //qDebug() << " - Not Checked";
    //No categories at all - just alphabetize all the apps
    QList<XDGDesktop> apps = sysapps->value("All"); 
    CCat.clear();
    //Now add all the apps for this category
    for(int i=0; i<apps.length(); i++){
      ItemWidget *it = new ItemWidget(ui->scroll_apps->widget(), apps[i] );
      if(!it->gooditem){ continue; } //invalid for some reason
      ui->scroll_apps->widget()->layout()->addWidget(it);
      connect(it, SIGNAL(NewShortcut()), this, SLOT(UpdateFavs()) );
      connect(it, SIGNAL(RemovedShortcut()), this, SLOT(UpdateFavs()) );
      connect(it, SIGNAL(RunItem(QString)), this, SLOT(LaunchItem(QString)) );
      connect(it, SIGNAL(toggleQuickLaunch(QString, bool)), this, SLOT(UpdateQuickLaunch(QString, bool)) );
    }
  }
  
  
}

void StartMenu::UpdateFavs(){
  //SYNTAX NOTE: (per-line) "<name>::::[dir/app/<mimetype>]::::<path>"
  QStringList newfavs = LUtils::listFavorites();
  if(favs == newfavs){ return; } //nothing to do - same as before
  favs = newfavs;
  ClearScrollArea(ui->scroll_favs);
  favs.sort();
  //Iterate over types of favorites
  QStringList rest = favs;
  QStringList tmp;
  for(int type = 0; type<3; type++){
    if(type==0){ tmp = favs.filter("::::app::::"); } //apps first
    else if(type==1){ tmp = favs.filter("::::dir::::"); } //dirs next
    else{ tmp = rest;  } //everything left over
    if(type==1){
      //Need to run a special routine for sorting the apps (already in the widget)
      // Since each app actually might have a different name listed within the file
      QLayout *lay = ui->scroll_favs->widget()->layout();
      QStringList items;
      for(int i=0; i<lay->count(); i++){
        items << lay->itemAt(i)->widget()->whatsThis().toLower();
      }
  
      items.sort();
      //qDebug() << " - Sorted Items:" << items;
      for(int i=0; i<items.length(); i++){
        if(items[i].isEmpty()){ continue; }
        //QLayouts are weird in that they can only add items to the end - need to re-insert almost every item
        for(int j=0; j<lay->count(); j++){
          //Find this item
          if(lay->itemAt(j)->widget()->whatsThis().toLower()==items[i]){
	    //Found it - now move it if necessary
	    //qDebug() << "Found Item:" << items[i] << i << j;
	    lay->addItem( lay->takeAt(j) );
	    break;
          }
        }
      }
    }//end of special app sorting routine
    tmp.sort(); //Sort alphabetically by name (dirs/files)
    for(int i=0; i<tmp.length(); i++){
      if(type<2){ rest.removeAll(tmp[i]); }
      if(!QFile::exists(tmp[i].section("::::",2,50))){ continue; } //invalid favorite - skip it
      ItemWidget *it = new ItemWidget(ui->scroll_favs->widget(), tmp[i].section("::::",2,50), tmp[i].section("::::",1,1) );
      if(!it->gooditem){ continue; } //invalid for some reason
      ui->scroll_favs->widget()->layout()->addWidget(it);
      connect(it, SIGNAL(NewShortcut()), this, SLOT(UpdateFavs()) );
      connect(it, SIGNAL(RemovedShortcut()), this, SLOT(UpdateFavs()) );
      connect(it, SIGNAL(RunItem(QString)), this, SLOT(LaunchItem(QString)) );
      connect(it, SIGNAL(toggleQuickLaunch(QString, bool)), this, SLOT(UpdateQuickLaunch(QString, bool)) );
    }
    QApplication::processEvents();
  }
}

// Page update routines
void StartMenu::on_stackedWidget_currentChanged(int val){
  QWidget *page = ui->stackedWidget->widget(val);
  ui->tool_back->setVisible(page != ui->page_main);
  ui->line_search->setVisible(false); //not implemented yet
  //Now the page specific updates
  if(page == ui->page_main){
    if(!ui->label_status_battery->whatsThis().isEmpty()){
      //Battery available - update the status button
      int charge = LOS::batteryCharge();
      QString TT, ICON;
      if(charge < 10){ ICON="-low"; }
      else if(charge<20){ ICON="-caution"; }
      else if(charge<40){ ICON="-040"; }
      else if(charge<60){ ICON="-060"; }
      else if(charge<80){ ICON="-080"; }
      else{ ICON="-100"; }
      if(LOS::batteryIsCharging()){
	if(charge>=80){ ICON.clear(); } //for charging, there is no suffix to the icon name over 80%
	ICON.prepend("battery-charging");
        TT = QString(tr("%1% (Plugged In)")).arg(QString::number(charge));
      }else{
	ICON.prepend("battery");
	int secs = LOS::batterySecondsLeft();
	if(secs>1){ TT = QString(tr("%1% (%2 Estimated)")).arg(QString::number(charge), LUtils::SecondsToDisplay(secs)); }
	else{ TT = QString(tr("%1% Remaining")).arg(QString::number(charge)); }
      }
      //qDebug() << " Battery Icon:" <<  ICON << val << TT
      ui->label_status_battery->setPixmap( LXDG::findIcon(ICON,"").pixmap(ui->tool_goto_apps->iconSize()/2) );
      ui->label_status_battery->setToolTip(TT);
    }
    //Network Status
    ui->label_status_network->clear(); //not implemented yet
  }else if(page == ui->page_apps){
    //Nothing needed for this page explicitly
  }else if( page == ui->page_settings){
    // -- Workspaces
    int tot = LSession::handle()->XCB->NumberOfWorkspaces();
    if(tot>1){
      ui->frame_wkspace->setVisible(true);
      int cur = LSession::handle()->XCB->CurrentWorkspace();
      ui->label_wkspace->setText( QString(tr("Workspace %1/%2")).arg(QString::number(cur+1), QString::number(tot)) );	
    }else{
      ui->frame_wkspace->setVisible(false);
    }
    // -- Brightness Controls
    int tmp = LOS::ScreenBrightness();
    ui->frame_bright->setVisible(tmp >= 0);
    if(tmp >= 0){ ui->slider_bright->setValue(tmp); }
    // -- Audio Controls
    tmp = LOS::audioVolume();
    ui->frame_audio->setVisible(tmp >= 0);
    if(tmp >= 0){ ui->slider_volume->setValue(tmp); }
  }else if(page == ui->page_leave){
    if( !ui->frame_leave_system->whatsThis().isEmpty() ){
      //This frame is allowed/visible - need to adjust the shutdown detection
      bool updating = LOS::systemPerformingUpdates();
      ui->tool_restart->setEnabled(!updating);
      ui->tool_shutdown->setEnabled(!updating);
      ui->label_updating->setVisible(updating); //to let the user know *why* they can't shutdown/restart right now
    }
    ui->frame_leave_suspend->setVisible( LOS::systemCanSuspend() );
  }
  
}

void StartMenu::catViewChanged(){
  QString state;
  switch(ui->check_apps_showcats->checkState()){
    case Qt::Checked:
	state = "true";
	break;
    case Qt::PartiallyChecked:
	state = "partial";
        break;
    default:
	state = "false";
  }
  LSession::handle()->DesktopPluginSettings()->setValue("panelPlugs/systemstart/showcategories", state);
  //Now kick off the reload of the apps list
  UpdateApps();
  //QtConcurrent::run(this, &StartMenu::UpdateApps); //this was a direct user change - keep it thread safe
}
//Page Change Buttons
void StartMenu::on_tool_goto_apps_clicked(){
  ui->stackedWidget->setCurrentWidget(ui->page_apps);
}

void StartMenu::on_tool_goto_settings_clicked(){
  ui->stackedWidget->setCurrentWidget(ui->page_settings);	
}

void StartMenu::on_tool_goto_logout_clicked(){
  ui->stackedWidget->setCurrentWidget(ui->page_leave);
}

void StartMenu::on_tool_back_clicked(){
  ui->stackedWidget->setCurrentWidget(ui->page_main);
}


//Launch Buttons
void StartMenu::on_tool_launch_controlpanel_clicked(){
  LaunchItem(ui->tool_launch_controlpanel->whatsThis());
}

void StartMenu::on_tool_launch_fm_clicked(){
  LaunchItem(QDir::homePath());	
}

void StartMenu::on_tool_launch_store_clicked(){
  LaunchItem(ui->tool_launch_store->whatsThis());
}

void StartMenu::on_tool_launch_desksettings_clicked(){
  LaunchItem("lumina-config", false);
}

void StartMenu::on_tool_launch_deskinfo_clicked(){
  LaunchItem("lumina-info",false);
}

//Logout Buttons
void StartMenu::on_tool_lock_clicked(){
  QProcess::startDetached("xscreensaver-command -lock");
}

void StartMenu::on_tool_logout_clicked(){
  emit CloseMenu();
  LSession::handle()->StartLogout();
}

void StartMenu::on_tool_restart_clicked(){
  emit CloseMenu();
  LSession::handle()->StartReboot();
}

void StartMenu::on_tool_shutdown_clicked(){
  emit CloseMenu();
  LSession::handle()->StartShutdown();
}

void StartMenu::on_tool_suspend_clicked(){
  //Make sure to lock the system first (otherwise anybody can access it again)
  emit CloseMenu();
  LUtils::runCmd("xscreensaver-command -lock");
  LOS::systemSuspend();
}


//Audio Volume
void StartMenu::on_slider_volume_valueChanged(int val){
  ui->label_vol->setText(QString::number(val)+"%");
  LOS::setAudioVolume(val);
  //Also adjust the icon for the volume
  if(val<1){ ui->tool_mute_audio->setIcon(LXDG::findIcon("audio-volume-muted","")); }
  else if(val<33){ ui->tool_mute_audio->setIcon(LXDG::findIcon("audio-volume-low","")); }
  else if(val<66){ ui->tool_mute_audio->setIcon(LXDG::findIcon("audio-volume-medium","")); }
  else{ ui->tool_mute_audio->setIcon(LXDG::findIcon("audio-volume-high","")); }
}

void StartMenu::on_tool_launch_mixer_clicked(){
  if(LOS::hasMixerUtility()){
    emit CloseMenu();
    LOS::startMixerUtility();
  }
}

void StartMenu::on_tool_mute_audio_clicked(){
  static int lastvol = 50;
  if(ui->slider_volume->value()==0){ ui->slider_volume->setValue(lastvol); }
  else{
    lastvol = ui->slider_volume->value();
    ui->slider_volume->setValue(0);
  }
}
	
//Screen Brightness
void StartMenu::on_slider_bright_valueChanged(int val){
  ui->label_bright->setText(QString::number(val)+"%");
  LOS::setScreenBrightness(val);
}

	
//Workspace
void StartMenu::on_tool_set_nextwkspace_clicked(){
  int cur = LSession::handle()->XCB->CurrentWorkspace();
  int tot = LSession::handle()->XCB->NumberOfWorkspaces();
  //qDebug()<< "Next Workspace:" << cur << tot;
  cur++;
  if(cur>=tot){ cur = 0; } //back to beginning
  //qDebug() << " - New Current:" << cur;
  LSession::handle()->XCB->SetCurrentWorkspace(cur);
  ui->label_wkspace->setText( QString(tr("Workspace %1/%2")).arg(QString::number(cur+1), QString::number(tot)) );
}

void StartMenu::on_tool_set_prevwkspace_clicked(){
  int cur = LSession::handle()->XCB->CurrentWorkspace();
  int tot = LSession::handle()->XCB->NumberOfWorkspaces();
  //qDebug()<< "Next Workspace:" << cur << tot;
  cur--;
  if(cur<0){ cur = tot-1; } //back to end
  //qDebug() << " - New Current:" << cur;
  LSession::handle()->XCB->SetCurrentWorkspace(cur);
  ui->label_wkspace->setText( QString(tr("Workspace %1/%2")).arg(QString::number(cur+1), QString::number(tot)) );	
}

	
//Locale
void StartMenu::on_combo_locale_currentIndexChanged(int){
  //Get the currently selected Locale
  if(ui->stackedWidget->currentWidget()!=ui->page_settings){ return; }
  QString locale = ui->combo_locale->currentData().toString();
  emit CloseMenu();
  LSession::processEvents();
  LSession::handle()->switchLocale(locale);
}

	
//Search
void StartMenu::on_line_search_editingFinished(){
	
}
