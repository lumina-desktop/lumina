//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "page_interface_panels.h"
#include "ui_page_interface_panels.h"
#include "getPage.h"
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
  
//First clean any current panels
  for(int i=0; i<PANELS.length(); i++){ delete PANELS.takeAt(i); i--; }
  //Now create new panels
  if(ui->scroll_panels->widget()->layout()==0){ 
    ui->scroll_panels->widget()->setLayout( new QHBoxLayout() ); 
    ui->scroll_panels->widget()->layout()->setContentsMargins(0,0,0,0);
  }
  ui->scroll_panels->widget()->layout()->setAlignment(Qt::AlignLeft);
  //Clear anything left over in the layout
  for(int i=0; i<ui->scroll_panels->widget()->layout()->count(); i++){
    delete ui->scroll_panels->widget()->layout()->takeAt(i);
  }
  for(int i=0; i<panelnumber; i++){
    PanelWidget *tmp = new PanelWidget(ui->scroll_panels->widget(), this, PINFO);
    tmp->LoadSettings(settings, cscreen, i);
    PANELS << tmp;
    connect(tmp, SIGNAL(PanelChanged()), this, SLOT(panelValChanged()) );
    connect(tmp, SIGNAL(PanelRemoved(int)), this, SLOT(removePanel(int)) );
    ui->scroll_panels->widget()->layout()->addWidget(tmp);
  }
  static_cast<QHBoxLayout*>(ui->scroll_panels->widget()->layout())->addStretch();

  QApplication::processEvents();
  loading = false;
}

void page_interface_panels::updateIcons(){
  ui->tool_panels_add->setIcon( LXDG::findIcon("list-add","") );
}

//=================
//         PRIVATE 
//=================
/*XDGDesktop page_interface_panels::getSysApp(bool allowreset){
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
}*/

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
