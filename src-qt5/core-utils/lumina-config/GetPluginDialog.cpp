//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "GetPluginDialog.h"
#include "ui_GetPluginDialog.h"

#include <LuminaXDG.h>

GetPluginDialog::GetPluginDialog(QWidget *parent) : QDialog(parent), ui(new Ui::GetPluginDialog()){
  ui->setupUi(this);
  selected = false; //nothing selected by default
  //Now center the window on the parent
  if(parent!=0){
    QWidget *top = parent;
    while(!top->isWindow()){ top = top->parentWidget(); }
    QPoint center = top->geometry().center();
    this->move(center.x()-(this->width()/2), center.y()-(this->height()/2) );
  }
  //Load the icons
  ui->push_cancel->setIcon( LXDG::findIcon("dialog-cancel","") );
  ui->push_accept->setIcon( LXDG::findIcon("dialog-ok","") );
  this->setWindowIcon( LXDG::findIcon("preferences-plugin") );
  //Connect the signals/slots
  connect(ui->combo_list, SIGNAL(currentIndexChanged(int)), this, SLOT(pluginchanged()) );
  connect(ui->push_cancel, SIGNAL(clicked()), this, SLOT(close()) );
  connect(ui->push_accept, SIGNAL(clicked()), this, SLOT(accept()) );
}

GetPluginDialog::~GetPluginDialog(){
	
}

void GetPluginDialog::LoadPlugins(QString type, LPlugins *DB){
  //Special data format: <Visible Name>::::<ID>::::<icon>::::<description>
  QStringList data;
  if(type.toLower()=="menu"){
    QStringList plugs = DB->menuPlugins();
    for(int i=0; i<plugs.length(); i++){
      LPI dat = DB->menuPluginInfo(plugs[i]);
      data << dat.name+"::::"+dat.ID+"::::"+dat.icon+"::::"+dat.description;
    }
  }else if(type.toLower()=="desktop"){
    QStringList plugs = DB->desktopPlugins();
    for(int i=0; i<plugs.length(); i++){
      LPI dat = DB->desktopPluginInfo(plugs[i]);
      data << dat.name+"::::"+dat.ID+"::::"+dat.icon+"::::"+dat.description;
    }	  
  }else if(type.toLower()=="panel"){
    QStringList plugs = DB->panelPlugins();
    for(int i=0; i<plugs.length(); i++){
      LPI dat = DB->panelPluginInfo(plugs[i]);
      data << dat.name+"::::"+dat.ID+"::::"+dat.icon+"::::"+dat.description;
    }	  
  }
  data.sort(); //this will sort them according to visible name
  ui->combo_list->clear();
  for(int i=0; i<data.length(); i++){
    ui->combo_list->addItem( LXDG::findIcon(data[i].section("::::",2,2),""), data[i].section("::::",0,0) , data[i]);
  }
  if(!data.isEmpty()){
    ui->combo_list->setCurrentIndex(0);
  }
}

void GetPluginDialog::pluginchanged(){
 //Load the description of the currently selected plugin
  if(ui->combo_list->count() < 1){ ui->label_desc->clear(); }
  else{
    ui->label_desc->setText( ui->combo_list->currentData().toString().section("::::",3,50) );
  }
  ui->push_accept->setEnabled(ui->combo_list->currentIndex()>=0);
}

void GetPluginDialog::accept(){
  plugID = ui->combo_list->currentData().toString().section("::::",1,1);
  selected = true;
  this->close();
}
