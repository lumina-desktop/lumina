//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "page_interface_menu.h"
#include "ui_page_interface_menu.h"
#include "../AppDialog.h"
#include "../GetPluginDialog.h"
#include "../ScriptDialog.h"
//==========
//    PUBLIC
//==========
page_interface_menu::page_interface_menu(QWidget *parent) : PageWidget(parent), ui(new Ui::page_interface_menu()){
  ui->setupUi(this);
  PINFO = new LPlugins();
  connect(ui->tool_menu_add, SIGNAL(clicked()), this, SLOT(addmenuplugin()) );
  connect(ui->tool_menu_rm, SIGNAL(clicked()), this, SLOT(rmmenuplugin()) );
  connect(ui->tool_menu_up, SIGNAL(clicked()), this, SLOT(upmenuplugin()) );
  connect(ui->tool_menu_dn, SIGNAL(clicked()), this, SLOT(downmenuplugin()) );
  connect(ui->list_menu, SIGNAL(currentRowChanged(int)), this, SLOT(checkmenuicons()) );

 updateIcons();
}

page_interface_menu::~page_interface_menu(){
  delete PINFO;
}

//================
//    PUBLIC SLOTS
//================
void page_interface_menu::SaveSettings(){
  QSettings settings("lumina-desktop","desktopsettings");
  QStringList items;
  for(int i=0; i<ui->list_menu->count(); i++){
    items << ui->list_menu->item(i)->whatsThis();
  }
  settings.setValue("menu/itemlist", items);
  emit HasPendingChanges(false);
}

void page_interface_menu::LoadSettings(int){
  emit HasPendingChanges(false);
  emit ChangePageTitle( tr("Desktop Settings") );
  QSettings settings("lumina-desktop","desktopsettings");

QStringList items = settings.value("menu/itemlist", QStringList() ).toStringList();
  if(items.isEmpty()){ items << "terminal" << "filemanager" << "applications" << "line" << "settings"; }
  //qDebug() << "Menu Items:" << items;
   ui->list_menu->clear();
   for(int i=0; i<items.length(); i++){
    if(items[i].startsWith("app::::")){
      XDGDesktop desk(items[i].section("::::",1,1));
      if(desk.type == XDGDesktop::BAD){ continue; } //invalid application file (no longer installed?)
      QListWidgetItem *item = new QListWidgetItem();
        item->setWhatsThis( items[i] );
        item->setIcon( LXDG::findIcon(desk.icon) );
        item->setText( desk.name );
        item->setToolTip( desk.comment );
      ui->list_menu->addItem(item);
      continue; //now go to the next item
    }else if(items[i].startsWith("jsonmenu::::")){
      LPI info = PINFO->menuPluginInfo( items[i].section("::::",0,0) );
      if(info.ID.isEmpty()){ continue; } //invalid plugin type (no longer available?)
      QString exec = items[i].section("::::",1,1);

      QListWidgetItem *item = new QListWidgetItem();
        item->setWhatsThis( items[i] );
        item->setIcon( LXDG::findIcon(items[i].section("::::",3,3),info.icon) );
        item->setText( items[i].section("::::",2,2) +" ("+info.name+")" );
        item->setToolTip( info.description );
      ui->list_menu->addItem(item);
      continue; //now go to the next item
    }
    LPI info = PINFO->menuPluginInfo(items[i]);
    if(info.ID.isEmpty()){ continue; } //invalid plugin
    //qDebug() << "Add Menu Item:" << info.ID;
    QListWidgetItem *item = new QListWidgetItem();
      item->setWhatsThis( info.ID );
      item->setIcon( LXDG::findIcon(info.icon,"") );
      item->setText( info.name );
      item->setToolTip( info.description );
    ui->list_menu->addItem(item);
   }
  checkmenuicons(); //update buttons
}

void page_interface_menu::updateIcons(){
  ui->tool_menu_add->setIcon( LXDG::findIcon("list-add","") );
  ui->tool_menu_rm->setIcon( LXDG::findIcon("list-remove","") );
  ui->tool_menu_up->setIcon( LXDG::findIcon("go-up","") );
  ui->tool_menu_dn->setIcon( LXDG::findIcon("go-down","") );
}

//=================
//         PRIVATE 
//=================
QString page_interface_menu::getSysApp(bool allowreset){
  AppDialog dlg(this);
    dlg.allowReset(allowreset);
    dlg.exec();
  if(dlg.appreset && allowreset){
    return "reset";
  }else{
    return dlg.appselected;
  }
}

//=================
//    PRIVATE SLOTS
//=================
void page_interface_menu::addmenuplugin(){
  GetPluginDialog dlg(this);
	dlg.LoadPlugins("menu", PINFO);
	dlg.exec();
  if(!dlg.selected){ return; } //cancelled
  QString plug = dlg.plugID;
  //Now add the item to the list
  LPI info = PINFO->menuPluginInfo(plug);
  QListWidgetItem *it;
  if(info.ID=="app"){
    //Need to prompt for the exact application to add to the menu
    // Note: whatsThis() format: "app::::< *.desktop file path >"
    QString app = getSysApp();
    if(app.isEmpty()){ return; }//nothing selected
    //Create the item for the list
    XDGDesktop desk(app);
    it = new QListWidgetItem(LXDG::findIcon(desk.icon,""), desk.name );
      it->setWhatsThis(info.ID+"::::"+desk.filePath);
      it->setToolTip( desk.comment );
  }else if(info.ID=="jsonmenu"){
    //Need to prompt for the script file, name, and icon to use
    //new ID format: "jsonmenu"::::<exec to run>::::<name>::::<icon>
    ScriptDialog SD(this);
    SD.exec();
    if(!SD.isValid()){ return; }
    it = new QListWidgetItem( LXDG::findIcon(SD.icon(),"text-x-script"), SD.name()+" ("+info.ID+")" );
    it->setWhatsThis(info.ID+"::::"+SD.command()+"::::"+SD.name()+"::::"+SD.icon());
    it->setToolTip( info.description );
  }else{
    it = new QListWidgetItem( LXDG::findIcon(info.icon,""), info.name );
    it->setWhatsThis(info.ID);
    it->setToolTip( info.description );
  }
  ui->list_menu->addItem(it);
  ui->list_menu->setCurrentRow(ui->list_menu->count()-1); //make sure it is auto-selected
  settingChanged();
}

void page_interface_menu::rmmenuplugin(){
  if(ui->list_menu->currentRow() < 0){ return; } //no selection
  delete ui->list_menu->takeItem( ui->list_menu->currentRow() );
  settingChanged();
}

void page_interface_menu::upmenuplugin(){
  int row = ui->list_menu->currentRow();
  if(row <= 0){ return; }
  ui->list_menu->insertItem(row-1, ui->list_menu->takeItem(row));
  ui->list_menu->setCurrentRow(row-1);

  checkmenuicons();
  settingChanged();
}

void page_interface_menu::downmenuplugin(){
  int row = ui->list_menu->currentRow();
  if(row < 0 || row >= (ui->list_menu->count()-1) ){ return; }
  ui->list_menu->insertItem(row+1, ui->list_menu->takeItem(row));
  ui->list_menu->setCurrentRow(row+1);

  checkmenuicons();
  settingChanged();
}

void page_interface_menu::checkmenuicons(){
  ui->tool_menu_up->setEnabled( ui->list_menu->currentRow() > 0 );
  ui->tool_menu_dn->setEnabled( ui->list_menu->currentRow() < (ui->list_menu->count()-1) );
  ui->tool_menu_rm->setEnabled( ui->list_menu->currentRow() >=0 );
}
