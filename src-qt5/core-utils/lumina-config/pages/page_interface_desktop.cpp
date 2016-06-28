//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "page_interface_desktop.h"
#include "ui_page_interface_desktop.h"
#include "getPage.h"
#include "../GetPluginDialog.h"
#include "../AppDialog.h"

//==========
//    PUBLIC
//==========
page_interface_desktop::page_interface_desktop(QWidget *parent) : PageWidget(parent), ui(new Ui::page_interface_desktop()){
  ui->setupUi(this);
  PINFO = new LPlugins();
  connect(ui->tool_desktop_addplugin, SIGNAL(clicked()), this, SLOT(deskplugadded()) );
  connect(ui->tool_desktop_rmplugin, SIGNAL(clicked()), this, SLOT(deskplugremoved()) );
  connect(ui->check_desktop_autolaunchers, SIGNAL(clicked()), this, SLOT(settingChanged()) );
  updateIcons();
}

page_interface_desktop::~page_interface_desktop(){
  delete PINFO;
}

//================
//    PUBLIC SLOTS
//================
void page_interface_desktop::SaveSettings(){
  QSettings settings("lumina-desktop","desktopsettings");
  QString DPrefix = "desktop-"+QString::number(cscreen)+"/";

  QStringList plugs;
  for(int i=0; i<ui->list_desktop_plugins->count(); i++){
    plugs << ui->list_desktop_plugins->item(i)->whatsThis();
  }
  if(settings.value(DPrefix+"pluginlist",QStringList()).toStringList() != plugs){
    settings.setValue(DPrefix+"pluginlist", plugs);
  }
  //The plugin ID's will be changed to unique ID's by the desktop - reload in a moment
  emit HasPendingChanges(false);
  QTimer::singleShot(1000, this, SLOT(LoadSettings()) );
}

void page_interface_desktop::LoadSettings(int screennum){
  if(screennum>=0){
    cscreen = screennum;
  }
  emit HasPendingChanges(false);
  emit ChangePageTitle( tr("Desktop Settings") );
  QSettings settings("lumina-desktop","desktopsettings");
  QString DPrefix = "desktop-"+QString::number(cscreen)+"/";

  QStringList dplugs = settings.value(DPrefix+"pluginlist",QStringList()).toStringList();
  ui->list_desktop_plugins->clear();
  for(int i=0; i<dplugs.length(); i++){
    QListWidgetItem* it = new QListWidgetItem();
    it->setWhatsThis(dplugs[i]); //save the full thing instantly
    //Now load the rest of the info about the plugin
    QString num;
    if(dplugs[i].contains("---")){ 
      num = dplugs[i].section("---",1,1).section(".",1,1).simplified(); //Skip the screen number
      if(num=="1"){ num.clear(); } //don't bother showing the number
      dplugs[i] = dplugs[i].section("---",0,0);
    }
    if(dplugs[i].startsWith("applauncher::")){
      bool ok = false;
      XDGDesktop app = LXDG::loadDesktopFile(dplugs[i].section("::",1,50), ok);
      if(!ok){ continue; } //invalid for some reason
      //Now fill the item with the necessary info
      it->setText(app.name);
      it->setIcon(LXDG::findIcon(app.icon,"") );
      it->setToolTip(app.comment);
    }else{
      //Load the info for this plugin
      LPI info = PINFO->desktopPluginInfo(dplugs[i]);
      if( info.ID.isEmpty() ){ continue; } //invalid plugin for some reason
      it->setText(info.name);
      it->setToolTip(info.description);
      it->setIcon( LXDG::findIcon(info.icon,"") );
    }
    if(!num.isEmpty()){ it->setText( it->text()+" ("+num+")"); } //append the number
    ui->list_desktop_plugins->addItem(it);
  }
}

void page_interface_desktop::updateIcons(){
  ui->tool_desktop_addplugin->setIcon( LXDG::findIcon("list-add","") );
  ui->tool_desktop_rmplugin->setIcon( LXDG::findIcon("list-remove","") );
}

//=================
//         PRIVATE 
//=================
XDGDesktop page_interface_desktop::getSysApp(bool allowreset){
  AppDialog dlg(this, LXDG::sortDesktopNames( LXDG::systemDesktopFiles() ) );
    dlg.allowReset(allowreset);
    dlg.exec();
  XDGDesktop desk;
  if(dlg.appreset && allowreset){
    desk.filePath = "reset"; //special internal flag
  }else{
    desk = dlg.appselected;
  }
  return desk;
}

//=================
//    PRIVATE SLOTS
//=================
void page_interface_desktop::deskplugadded(){
  GetPluginDialog dlg(this);
    dlg.LoadPlugins("desktop", PINFO);
    dlg.exec();
  if( !dlg.selected ){ return; } //cancelled
  QString newplug = dlg.plugID;
  QListWidgetItem *it = new QListWidgetItem();
  if(newplug=="applauncher"){
    //Prompt for the application to add
    XDGDesktop app = getSysApp();
    if(app.filePath.isEmpty()){ return; } //cancelled
    newplug.append("::"+app.filePath);
    //Now fill the item with the necessary info
    it->setWhatsThis(newplug);
    it->setText(app.name);
    it->setIcon(LXDG::findIcon(app.icon,"") );
    it->setToolTip(app.comment);
  }else{
    //Load the info for this plugin
    LPI info = PINFO->desktopPluginInfo(newplug);
    if( info.ID.isEmpty() ){ return; } //invalid plugin for some reason (should never happen)
    it->setWhatsThis(newplug);
    it->setText(info.name);
    it->setToolTip(info.description);
    it->setIcon( LXDG::findIcon(info.icon,"") );
  }
  ui->list_desktop_plugins->addItem(it);
  ui->list_desktop_plugins->scrollToItem(it);
  settingChanged();
}

void page_interface_desktop::deskplugremoved(){
  QList<QListWidgetItem*> sel = ui->list_desktop_plugins->selectedItems();
  if(sel.isEmpty()){ return; } //nothing to do
  for(int i=0; i<sel.length(); i++){
    delete sel[i];
  }
  settingChanged();
}
