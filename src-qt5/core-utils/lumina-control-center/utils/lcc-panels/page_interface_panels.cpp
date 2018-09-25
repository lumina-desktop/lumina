//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "page_interface_panels.h"
#include "ui_page_interface_panels.h"
#include <QInputDialog>

#include "../GetPluginDialog.h"
#include "../AppDialog.h"

//==========
//    PUBLIC
//==========
page_interface_panels::page_interface_panels(QWidget *parent) : PageWidget(parent), ui(new Ui::page_interface_panels()){
  ui->setupUi(this);
  loading = false;
  PINFO = new LPlugins();
  settings = new QSettings("lumina-desktop","desktopsettings");
  connect(ui->tool_panels_add, SIGNAL(clicked()), this, SLOT(newPanel()) );
  updateIcons();

  //Create panels container
  QHBoxLayout *panels_layout = new QHBoxLayout();
  panels_layout->setContentsMargins(0,0,0,0);
  panels_layout->setAlignment(Qt::AlignLeft);
  panels_layout->addStretch();
  ui->scroll_panels->widget()->setLayout(panels_layout);
}

page_interface_panels::~page_interface_panels(){
  delete PINFO;
}

//================
//    PUBLIC SLOTS
//================
void page_interface_panels::SaveSettings(){
  QString screenID = QApplication::screens().at(cscreen)->name();
  QString DPrefix = "desktop-"+screenID+"/";
  settings->setValue(DPrefix+"panels", PANELS.length());
  for(int i=0; i<PANELS.length(); i++){
    PANELS[i]->SaveSettings(settings);
  }
  //The plugin ID's will be changed to unique ID's by the desktop - reload in a moment
  emit HasPendingChanges(false);
  settings->sync(); //save to disk right now
  QTimer::singleShot(1000, this, SLOT(LoadSettings()) );
}

void page_interface_panels::LoadSettings(int screennum){
  if(screennum>=0){
    cscreen = screennum;
  }
  loading = true;
  emit HasPendingChanges(false);
  emit ChangePageTitle( tr("Desktop Settings") );
  QString screenID = QApplication::screens().at(cscreen)->name();
  QString DPrefix = "desktop-"+screenID+"/";
  int panelnumber = settings->value(DPrefix+"panels",-1).toInt();
  if(panelnumber<0){ panelnumber = 0; }
  QHBoxLayout *panels_layout = static_cast<QHBoxLayout*>(ui->scroll_panels->widget()->layout());

  //Remove extra panels (if any)
  for(int i=panelnumber; i<PANELS.length(); i++){
    PanelWidget *tmp = PANELS.takeAt(i);
    delete tmp;
    i--;
  }

  int current_count = panels_layout->count()-1;

  //Update current panels
  for(int i=0; i<current_count; i++) {
    PANELS[i]->LoadSettings(settings, cscreen, i);
  }
  //Create new panels
  for(int i=current_count; i<panelnumber; i++){
    PanelWidget *tmp = new PanelWidget(ui->scroll_panels->widget(), this, PINFO);
    tmp->LoadSettings(settings, cscreen, i);
    PANELS << tmp;
    connect(tmp, SIGNAL(PanelChanged()), this, SLOT(panelValChanged()) );
    connect(tmp, SIGNAL(PanelRemoved(int)), this, SLOT(removePanel(int)) );
    panels_layout->insertWidget(panels_layout->count()-1, tmp);
  }

  QApplication::processEvents();
  loading = false;
  setupProfiles();
}

void page_interface_panels::updateIcons(){
  ui->tool_panels_add->setIcon( LXDG::findIcon("list-add","") );
  ui->tool_profile->setIcon( LXDG::findIcon("document-import","") );
}

//=================
//         PRIVATE
//=================
void page_interface_panels::setupProfiles(){
  //qDebug() << "Start loading profiles";
  if(ui->tool_profile->menu()==0){
    ui->tool_profile->setMenu( new QMenu(this) );
    connect(ui->tool_profile->menu(), SIGNAL(triggered(QAction*)), this, SLOT(applyProfile(QAction*)) );
  }
  else{ ui->tool_profile->menu()->clear(); }
  ui->tool_profile->menu()->addSection("Profiles");
  QAction *act = ui->tool_profile->menu()->addAction(tr("No Panels"));
    act->setWhatsThis("none");
  act = ui->tool_profile->menu()->addAction("Windows");
    act->setWhatsThis("windows");
  act = ui->tool_profile->menu()->addAction("GNOME2/MATE");
    act->setWhatsThis("gnome2");
  act = ui->tool_profile->menu()->addAction("XFCE");
    act->setWhatsThis("xfce");
  act = ui->tool_profile->menu()->addAction("Mac OSX");
    act->setWhatsThis("osx");

  //Add in any custom profiles
  //qDebug() << " - read settings";
  QStringList profilesAll = settings->childGroups().filter("panel_");
  //qDebug() << " - get current screen";
  QString current = QApplication::screens().at(cscreen)->name();
  //qDebug() << " - filter list";
  for(int i=0; i<profilesAll.length(); i++){
    profilesAll[i] = profilesAll[i].section("_",1,-1).section(".",0,-2);
  }
  //qDebug() << "Found Profiles:" << profilesAll;
  profilesAll.removeDuplicates();
  profilesAll.removeAll(current);
  QStringList profiles = profilesAll.filter("profile_");
  for(int p=0; p<2; p++){
    if(p==1){ profiles = profilesAll; } //use whats left of the total list
    ui->tool_profile->menu()->addSection( p==0 ? tr("Custom Profiles") : tr("Copy Screen") );
   for(int i=0; i<profiles.length(); i++){
    if(p==0){ profilesAll.removeAll(profiles[i]); } //handling it now
    QString title = profiles[i];
      if(title.startsWith("profile_")){ title = title.section("profile_",-1); }
    QMenu *tmp = new QMenu(ui->tool_profile->menu());
      tmp->setTitle(title);
      tmp->addAction(LXDG::findIcon("dialog-ok-apply",""), tr("Apply"))->setWhatsThis("profile_apply::::"+profiles[i]);
      tmp->addAction(LXDG::findIcon("list-remove",""), tr("Delete"))->setWhatsThis("profile_remove::::"+profiles[i]);
    ui->tool_profile->menu()->addMenu(tmp);
   }
   if(p==0){
     //Now add the option to create a new profile
     ui->tool_profile->menu()->addAction(LXDG::findIcon("list-add",""), tr("Create Profile"))->setWhatsThis("profile_new");
   }
  }
}

//=================
//    PRIVATE SLOTS
//=================
void page_interface_panels::panelValChanged(){
  ui->tool_panels_add->setEnabled(PANELS.length() < 12);
  if(!loading){ settingChanged(); }
}

void page_interface_panels::newPanel(){
  //if(panelnumber<0){ panelnumber=0; } //just in case
  //panelnumber++;
  //Now create a new Panel widget with this number
  PanelWidget *tmp = new PanelWidget(ui->scroll_panels->widget(), this, PINFO);
    tmp->LoadSettings(settings, cscreen, PANELS.length());
    PANELS << tmp;
    connect(tmp, SIGNAL(PanelChanged()), this, SLOT(panelValChanged()) );
    connect(tmp, SIGNAL(PanelRemoved(int)), this, SLOT(removePanel(int)) );
    static_cast<QBoxLayout*>(ui->scroll_panels->widget()->layout())->insertWidget(PANELS.length()-1, tmp);
     //update the widget first (2 necessary for scroll below to work)
    ui->scroll_panels->update();
    QApplication::processEvents();
    QApplication::processEvents();
    ui->scroll_panels->ensureWidgetVisible(tmp);
    panelValChanged();
}

void page_interface_panels::removePanel(int pan){
  //connected to a signal from the panel widget
  bool changed = false;
  for(int i=0; i<PANELS.length(); i++){
    int num = PANELS[i]->PanelNumber();
    if(num==pan){
      delete PANELS.takeAt(i);
      i--;
      changed = true;
    }else if(num > pan){
      PANELS[i]->ChangePanelNumber(num-1);
      changed = true;
    }
  }
  if(!changed){ return; } //nothing done
  panelValChanged();
}

void page_interface_panels::applyProfile(QAction *act){
  QString wt = act->whatsThis();
  if(wt.startsWith("profile_")){
    //qDebug() << "Got Profile Action:" << wt;
    if(wt=="profile_new"){
      bool ok = false;
      QString pname = QInputDialog::getText(this, tr("Create Profile"), tr("Name:"), QLineEdit::Normal,  "", &ok,Qt::WindowFlags(), Qt::ImhUppercaseOnly | Qt::ImhLowercaseOnly );
      if(!ok || pname.isEmpty()){ return; } //cancelled
      pname = pname.replace(".","_").replace("/","_");
      qDebug() << " - Make new profile:" << pname;
      pname.prepend("profile_");
      settings->setValue("desktop-"+pname+"/panels", PANELS.length());
      for(int i=0; i<PANELS.length(); i++){
        PANELS[i]->SaveSettings(settings, pname);
      }
      settings->sync(); //save to disk right now
      setupProfiles();
    }else if(wt.startsWith("profile_apply::::") ){
     applyImport(wt.section("::::",-1) );
    }else if(wt.startsWith("profile_remove::::") ){
      QString pname = wt.section("::::",-1);
      QStringList keys = settings->allKeys().filter(pname);
      for(int i=0; i<keys.length(); i++){
        if(keys[i].section("/",0,0).contains(pname)){ settings->remove(keys[i]); }
      }
      setupProfiles();
    }
    return;
  }
  //Manually saving settings based on built-in profile
  QString screenID = QApplication::screens().at(cscreen)->name();
  QString DPrefix = "desktop-"+screenID+"/";
  QString PPrefix = "panel_"+screenID+"."; //NEED TO APPEND PANEL NUMBER (0+)
  qDebug() << "Apply Profile:" << act->whatsThis() << "To Monitor:" << screenID;
  if(act->whatsThis()=="none"){
    settings->setValue(DPrefix+"panels", 0); //number of panels
  }else if(act->whatsThis()=="windows"){
    settings->setValue(DPrefix+"panels", 1); //number of panels
    //Panel 1 settings (index 0)
    settings->setValue(PPrefix+"0/customColor", false);
    settings->setValue(PPrefix+"0/height", qRound(QApplication::screens().at(cscreen)->virtualSize().height()*0.04)); //4% of screen height
    settings->setValue(PPrefix+"0/hidepanel", false);
    settings->setValue(PPrefix+"0/lengthPercent", 100);
    settings->setValue(PPrefix+"0/location", "bottom");
    settings->setValue(PPrefix+"0/pinLocation", "center");
    settings->setValue(PPrefix+"0/pluginlist", QStringList() << "systemstart" << "taskmanager" << "spacer" << "systemtray" << "clock");
  }else if(act->whatsThis()=="gnome2"){
    settings->setValue(DPrefix+"panels", 2); //number of panels
    //Panel 1 settings (index 0)
    settings->setValue(PPrefix+"0/customColor", false);
    settings->setValue(PPrefix+"0/height", qRound(QApplication::screens().at(cscreen)->virtualSize().height()*0.02)); //2% of screen height
    settings->setValue(PPrefix+"0/hidepanel", false);
    settings->setValue(PPrefix+"0/lengthPercent", 100);
    settings->setValue(PPrefix+"0/location", "top");
    settings->setValue(PPrefix+"0/pinLocation", "center");
    settings->setValue(PPrefix+"0/pluginlist", QStringList() << "appmenu" << "desktopbar" << "spacer" << "systemtray" << "clock" << "systemdashboard");
    //Panel 2 settings (index 1)
    settings->setValue(PPrefix+"1/customColor", false);
    settings->setValue(PPrefix+"1/height", qRound(QApplication::screens().at(cscreen)->virtualSize().height()*0.02)); //2% of screen height
    settings->setValue(PPrefix+"1/hidepanel", false);
    settings->setValue(PPrefix+"1/lengthPercent", 100);
    settings->setValue(PPrefix+"1/location", "bottom");
    settings->setValue(PPrefix+"1/pinLocation", "center");
    settings->setValue(PPrefix+"1/pluginlist", QStringList() << "homebutton" << "taskmanager-nogroups" << "spacer" << "desktopswitcher");
}else if(act->whatsThis()=="xfce"){
    settings->setValue(DPrefix+"panels", 2); //number of panels
    //Panel 1 settings (index 0)
    settings->setValue(PPrefix+"0/customColor", false);
    settings->setValue(PPrefix+"0/height", qRound(QApplication::screens().at(cscreen)->virtualSize().height()*0.02)); //2% of screen height
    settings->setValue(PPrefix+"0/hidepanel", false);
    settings->setValue(PPrefix+"0/lengthPercent", 100);
    settings->setValue(PPrefix+"0/location", "top");
    settings->setValue(PPrefix+"0/pinLocation", "center");
    settings->setValue(PPrefix+"0/pluginlist", QStringList() << "appmenu" << "taskmanager-nogroups" << "spacer" << "desktopswitcher" << "clock" << "systemtray" << "systemdashboard");
    //Panel 2 settings (index 1)
    settings->setValue(PPrefix+"1/customColor", false);
    settings->setValue(PPrefix+"1/height", qRound(QApplication::screens().at(cscreen)->virtualSize().height()*0.04)); //4% of screen height
    settings->setValue(PPrefix+"1/hidepanel", false);
    settings->setValue(PPrefix+"1/lengthPercent", 20);
    settings->setValue(PPrefix+"1/location", "bottom");
    settings->setValue(PPrefix+"1/pinLocation", "center");
    settings->setValue(PPrefix+"1/pluginlist", QStringList() << "applauncher::lumina-fm.desktop" << "line"<<"spacer" << "desktopbar" << "spacer" << "line" << "applauncher::lumina-search.desktop");
}else if(act->whatsThis()=="osx"){
    settings->setValue(DPrefix+"panels", 2); //number of panels
    //Panel 1 settings (index 0)
    settings->setValue(PPrefix+"0/customColor", false);
    settings->setValue(PPrefix+"0/height", qRound(QApplication::screens().at(cscreen)->virtualSize().height()*0.02)); //2% of screen height
    settings->setValue(PPrefix+"0/hidepanel", false);
    settings->setValue(PPrefix+"0/lengthPercent", 100);
    settings->setValue(PPrefix+"0/location", "top");
    settings->setValue(PPrefix+"0/pinLocation", "center");
    settings->setValue(PPrefix+"0/pluginlist", QStringList() << "systemdashboard" <<  "spacer" << "systemtray" << "clock" << "applauncher::lumina-search.desktop");
    //Panel 2 settings (index 1)
    settings->setValue(PPrefix+"1/customColor", false);
    settings->setValue(PPrefix+"1/height", qRound(QApplication::screens().at(cscreen)->virtualSize().height()*0.04)); //4% of screen height
    settings->setValue(PPrefix+"1/hidepanel", false);
    settings->setValue(PPrefix+"1/lengthPercent", 80);
    settings->setValue(PPrefix+"1/location", "bottom");
    settings->setValue(PPrefix+"1/pinLocation", "center");
    settings->setValue(PPrefix+"1/pluginlist", QStringList() << "systemstart" << "applauncher::lumina-fm.desktop" << "desktopbar"<<"spacer" << "line"<< "taskmanager");

  }else{
    qDebug() << " - unknown profile! ("+act->whatsThis()+")";
    return;
  }
  //Now flush the settings to disk and reload the interface
  settings->sync(); //save to disk right now
  QTimer::singleShot(1000, this, SLOT(LoadSettings()) );
}

void page_interface_panels::applyImport(QAction *act){
  applyImport(act->whatsThis());
}

void page_interface_panels::applyImport(QString fromID){
  QString cID = QApplication::screens().at(cscreen)->name();
  //QString DPrefix = "desktop-"+screenID+"/";
  qDebug() << "Import Panels from " << fromID << " to " << cID;
  //First find all the values associated with this ID
    int pannum = settings->value("desktop-"+fromID+"/panels").toInt();
    QStringList pans = settings->allKeys().filter("panel_"+fromID);
    fromID.prepend("panel_");

  //save the number of panels which is active
  settings->setValue("desktop-"+cID+"/panels", pannum);
 //Now move over all the panel settings associated with the fromID
  cID.prepend("panel_");
  for(int i=0; i<pans.length(); i++){
    QString newvar = pans[i];
      newvar.replace(fromID, cID);
    settings->setValue(newvar, settings->value(pans[i]) );
  }
  //Now flush the settings to disk and reload the interface
  settings->sync(); //save to disk right now
  QTimer::singleShot(1000, this, SLOT(LoadSettings()) );
}
