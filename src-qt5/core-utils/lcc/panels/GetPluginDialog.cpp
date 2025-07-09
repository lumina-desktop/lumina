//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "GetPluginDialog.h"
#include "ui_GetPluginDialog.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <LuminaXDG.h>

GetPluginDialog::GetPluginDialog(QWidget* parent) : QDialog(parent), ui(new Ui::GetPluginDialog){
  ui->setupUi(this);
  selected = false;
  connect(ui->list_plugins, SIGNAL(itemSelectionChanged()), this, SLOT(pluginchanged()) );
}

GetPluginDialog::~GetPluginDialog(){
  delete ui;
}

void GetPluginDialog::LoadPlugins(QString type, LPlugins *DB){
  ui->list_plugins->clear();
  QStringList plugs;
  if(type=="panel"){ plugs = DB->panelPlugins(); }
  else if(type=="desktop"){ plugs = DB->desktopPlugins(); }
  else if(type=="menu"){ plugs = DB->menuPlugins(); }
  
  for(int i=0; i<plugs.length(); i++){
    LPI info;
    if(type=="panel"){ info = DB->panelPluginInfo(plugs[i]); }
    else if(type=="desktop"){ info = DB->desktopPluginInfo(plugs[i]); }
    else if(type=="menu"){ info = DB->menuPluginInfo(plugs[i]); }
    
    QListWidgetItem *it = new QListWidgetItem( LXDG::findIcon(info.icon,""), info.name );
    it->setWhatsThis(info.ID);
    it->setToolTip(info.description);
    ui->list_plugins->addItem(it);
  }
}

void GetPluginDialog::pluginchanged(){
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled( ui->list_plugins->currentItem()!=0 );
}

void GetPluginDialog::accept(){
  if(ui->list_plugins->currentItem()!=0){
    selected = true;
    plugID = ui->list_plugins->currentItem()->whatsThis();
  }
  QDialog::accept();
}
