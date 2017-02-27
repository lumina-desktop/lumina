//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "page_interface_panels.h"
#include "ui_page_interface_panels.h"
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
  setupProfiles();

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
  QBoxLayout *panels_layout = static_cast<QHBoxLayout*>(ui->scroll_panels->widget()->layout());
  
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
  setupImports();
}

void page_interface_panels::updateIcons(){
  ui->tool_panels_add->setIcon( LXDG::findIcon("list-add","") );
  ui->tool_profile->setIcon( LXDG::findIcon("view-media-artist","") );
  ui->tool_import->setIcon( LXDG::findIcon("document-import","") );
}

//=================
//         PRIVATE 
//=================
void page_interface_panels::setupProfiles(){
  ui->tool_profile->setMenu( new QMenu(this) );
  QAction *act = ui->tool_profile->menu()->addAction("No Panels");
    act->setWhatsThis("none");
  act = ui->tool_profile->menu()->addAction("Windows");
    act->setWhatsThis("windows");
  act = ui->tool_profile->menu()->addAction("GNOME2/MATE");
    act->setWhatsThis("gnome2");
  act = ui->tool_profile->menu()->addAction("XFCE");
    act->setWhatsThis("xfce");
  act = ui->tool_profile->menu()->addAction("Mac OSX");
    act->setWhatsThis("osx");

  connect(ui->tool_profile->menu(), SIGNAL(triggered(QAction*)), this, SLOT(applyProfile(QAction*)) );
}

void page_interface_panels::setupImports(){
  if(ui->tool_import->menu()==0){ ui->tool_import->setMenu( new QMenu(this) ); }
  else{ ui->tool_import->menu()->clear(); }
  //Read all the various disk settings currently saved
  QStringList other = settings->childGroups().filter("panel_");
  qDebug() << "Found Other Settings:" << other;
  for(int i=0; i<other.length(); i++){
    other[i] = other[i].section("_",1,-1).section(".",0,-2);
  }
  other.removeDuplicates();
  QString current = QApplication::screens().at(cscreen)->name();
  for(int i=0; i<other.length(); i++){
    if(other[i]==current){ continue; } //don't show the current settings
    QAction *act = ui->tool_import->menu()->addAction(other[i]);
      act->setWhatsThis(other[i]);
  }

  connect(ui->tool_import->menu(), SIGNAL(triggered(QAction*)), this, SLOT(applyImport(QAction*)) );
  ui->tool_import->setEnabled(!ui->tool_import->menu()->isEmpty());
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
  QString cID = QApplication::screens().at(cscreen)->name();
  QString fromID = act->whatsThis();
  //QString DPrefix = "desktop-"+screenID+"/";
  qDebug() << "Import Panels from " << fromID << " to " << cID;
  //First change the number of panels on the desktop settings
  settings->setValue("desktop-"+cID+"/panels", settings->value("desktop-"+fromID+"/panels"));
 //Now move over all the panels associated with the fromID
  QStringList pans = settings->allKeys().filter("panel_"+fromID);
  for(int i=0; i<pans.length(); i++){
    QString newvar = pans[i];
      newvar.replace(fromID, cID);
    settings->setValue(newvar, settings->value(pans[i]) );
  }
  //Now flush the settings to disk and reload the interface
  settings->sync(); //save to disk right now
  QTimer::singleShot(1000, this, SLOT(LoadSettings()) );
}
