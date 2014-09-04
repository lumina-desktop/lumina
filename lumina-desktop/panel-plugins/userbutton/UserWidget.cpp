//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "UserWidget.h"
#include "ui_UserWidget.h"
#include "../../LSession.h"
#include "../../AppMenu.h"

UserWidget::UserWidget(QWidget* parent) : QWidget(parent), ui(new Ui::UserWidget){
  ui->setupUi(this);
  this->setContentsMargins(0,0,0,0);
  sysapps = LSession::applicationMenu()->currentAppHash(); //get the raw info
  //Setup the Icons
    // - favorites tab
    ui->tabWidget->setTabIcon(0, LXDG::findIcon("favorites","") );
    ui->tabWidget->setTabText(0,"");
    // - apps tab
    ui->tabWidget->setTabIcon(1, LXDG::findIcon("system-run","") );
    ui->tabWidget->setTabText(1,"");
    // - home tab
    ui->tabWidget->setTabIcon(2, LXDG::findIcon("user-home","") );
    ui->tabWidget->setTabText(2,"");
    // - config tab
    ui->tabWidget->setTabIcon(3, LXDG::findIcon("preferences-system","") );
    ui->tabWidget->setTabText(3,"");
  ui->tool_fav_apps->setIcon( LXDG::findIcon("system-run","") );
  ui->tool_fav_dirs->setIcon( LXDG::findIcon("folder","") );
  ui->tool_fav_files->setIcon( LXDG::findIcon("document-multiple","") );
  ui->tool_desktopsettings->setIcon( LXDG::findIcon("preferences-desktop","") );
  ui->tool_config_screensaver->setIcon( LXDG::findIcon("preferences-desktop-screensaver","") );	

  //Connect the signals/slots
  connect(ui->tool_desktopsettings, SIGNAL(clicked()), this, SLOT(openDeskSettings()) );
  connect(ui->tool_config_screensaver, SIGNAL(clicked()), this, SLOT(openScreenSaverConfig()) );
  connect(ui->tool_fav_apps, SIGNAL(clicked()), this, SLOT(FavChanged()) );
  connect(ui->tool_fav_files, SIGNAL(clicked()), this, SLOT(FavChanged()) );
  connect(ui->tool_fav_dirs, SIGNAL(clicked()), this, SLOT(FavChanged()) );
  connect(ui->combo_app_cats, SIGNAL(currentIndexChanged(int)), this, SLOT(updateApps()) );
  //Setup the special buttons
  if(QFile::exists(APPSTORE)){
    //Now load the info
    bool ok = false;
    XDGDesktop store = LXDG::loadDesktopFile(APPSTORE, ok);
    if(ok){
      ui->tool_app_store->setIcon( LXDG::findIcon(store.icon, "") );
      ui->tool_app_store->setText( store.name );
      connect(ui->tool_app_store, SIGNAL(clicked()), this, SLOT(openStore()) );
    }
    ui->tool_app_store->setVisible(ok); //availability
  }else{
    ui->tool_app_store->setVisible(false); //not available
  }
  if(QFile::exists(CONTROLPANEL)){
    //Now load the info
    bool ok = false;
    XDGDesktop cpan = LXDG::loadDesktopFile(CONTROLPANEL, ok);
    if(ok){
      ui->tool_controlpanel->setIcon( LXDG::findIcon(cpan.icon, "") );
      connect(ui->tool_controlpanel, SIGNAL(clicked()), this, SLOT(openControlPanel()) );
    }
    ui->tool_controlpanel->setVisible(ok); //availability
  }else{
    ui->tool_controlpanel->setVisible(false); //not available
  }
  if(QFile::exists(QTCONFIG)){
    ui->tool_qtconfig->setVisible(true);
    ui->tool_qtconfig->setIcon( LXDG::findIcon("preferences-desktop-theme","") );
    connect(ui->tool_qtconfig, SIGNAL(clicked()), this, SLOT(openQtConfig()) );
  }else{
    ui->tool_qtconfig->setVisible(false);
  }
  
}

UserWidget::~UserWidget(){
}

//===========
//     PRIVATE
//===========
void UserWidget::ClearScrollArea(QScrollArea *area){
  QWidget *wgt = area->takeWidget();
  delete wgt; //delete the widget and all children
  area->setWidget( new QWidget() ); //create a new widget in the scroll area
  area->widget()->setContentsMargins(0,0,0,0);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(2);
    layout->setContentsMargins(3,1,3,1);
    layout->setDirection(QBoxLayout::TopToBottom);
    area->widget()->setLayout(layout);
}

//============
//  PRIVATE SLOTS
//============
void UserWidget::UpdateMenu(){
  ui->tabWidget->setCurrentWidget(ui->tab_fav);
  ui->tool_fav_apps->setChecked(true);
  ui->tool_fav_dirs->setChecked(false);
  ui->tool_fav_files->setChecked(false);
  cfav = 0; //favorite apps
  updateFavItems();
  updateHome();
  updateAppCategories();
  updateApps();
}

void UserWidget::LaunchItem(QString cmd){
  if(!cmd.isEmpty()){
    qDebug() << "Launch Application:" << cmd;
    LSession::LaunchApplication(cmd);
    emit CloseMenu(); //so the menu container will close
  }
}

void UserWidget::FavChanged(){
  //uncheck the current item for a moment
  if(cfav==0){ ui->tool_fav_apps->setChecked(false); }
  else if(cfav==1){ ui->tool_fav_dirs->setChecked(false); }
  if(cfav==2){ ui->tool_fav_files->setChecked(false); }
  //Now check what other item is now the only one checked
  if(ui->tool_fav_apps->isChecked() && !ui->tool_fav_dirs->isChecked() && !ui->tool_fav_files->isChecked() ){
    cfav = 0;
  }else if(!ui->tool_fav_apps->isChecked() && ui->tool_fav_dirs->isChecked() && !ui->tool_fav_files->isChecked() ){
    cfav = 1;
  }else if(!ui->tool_fav_apps->isChecked() && !ui->tool_fav_dirs->isChecked() && ui->tool_fav_files->isChecked() ){
    cfav = 2;
  }else{
    //Re-check the old item (something invalid)
    ui->tool_fav_apps->setChecked(cfav==0);
    ui->tool_fav_dirs->setChecked(cfav==1);
    ui->tool_fav_files->setChecked(cfav==2);
  }
  updateFavItems();
}

void UserWidget::updateFavItems(){
  ClearScrollArea(ui->scroll_fav);
  QStringList items;
  QDir homedir = QDir( QDir::homePath()+"/Desktop");
  if(ui->tool_fav_apps->isChecked()){ items = homedir.entryList(QStringList()<<"*.desktop", QDir::Files | QDir::NoDotAndDotDot, QDir::Name); }
  else if(ui->tool_fav_dirs->isChecked()){ items = homedir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name); }
  else{ 
    //Files
    items = homedir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);  
    for(int i=0; i<items.length(); i++){
      if(items[i].endsWith(".desktop")){ items.removeAt(i); i--; }
    }
  }
  for(int i=0; i<items.length(); i++){
    UserItemWidget *it = new UserItemWidget(ui->scroll_fav->widget(), homedir.absoluteFilePath(items[i]), ui->tool_fav_dirs->isChecked());
    ui->scroll_fav->widget()->layout()->addWidget(it);
    connect(it, SIGNAL(RunItem(QString)), this, SLOT(LaunchItem(QString)) );
    connect(it, SIGNAL(NewShortcut()), this, SLOT(updateFavItems()) );
    connect(it, SIGNAL(RemovedShortcut()), this, SLOT(updateFavItems()) );
  }
  static_cast<QBoxLayout*>(ui->scroll_fav->widget()->layout())->addStretch();
}

//Apps Tab
void UserWidget::updateAppCategories(){
  ui->combo_app_cats->clear();
  QStringList cats = sysapps->keys();
  cats.sort();
  for(int i=0; i<cats.length(); i++){
    QString name, icon;
    if(cats[i] == "Multimedia"){ name = tr("Multimedia"); icon = "applications-multimedia"; }
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
    else{ name = tr("Unsorted"); icon = "applications-other"; }
    ui->combo_app_cats->addItem( LXDG::findIcon(icon,""), name, cats[i] );
  }
}

void UserWidget::updateApps(){
  if(ui->combo_app_cats->currentIndex() < 0){ return; } //no cat
  QString cat = ui->combo_app_cats->itemData( ui->combo_app_cats->currentIndex() ).toString();
  QList<XDGDesktop> items = sysapps->value(cat);
  ClearScrollArea(ui->scroll_apps);
  for(int i=0; i<items.length(); i++){
    UserItemWidget *it = new UserItemWidget(ui->scroll_apps->widget(), items[i]);
    ui->scroll_apps->widget()->layout()->addWidget(it);
    connect(it, SIGNAL(RunItem(QString)), this, SLOT(LaunchItem(QString)) );
    connect(it, SIGNAL(NewShortcut()), this, SLOT(updateFavItems()) );
    connect(it, SIGNAL(RemovedShortcut()), this, SLOT(updateFavItems()) );
  }
  static_cast<QBoxLayout*>(ui->scroll_apps->widget()->layout())->addStretch();
}

//Home Tab
void UserWidget::updateHome(){
  ClearScrollArea(ui->scroll_home);
  QDir homedir = QDir::home();
  QStringList items = homedir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name); 
  for(int i=0; i<items.length(); i++){
    //qDebug() << "New Home subdir:" << homedir.absoluteFilePath(items[i]);
    UserItemWidget *it = new UserItemWidget(ui->scroll_home->widget(), homedir.absoluteFilePath(items[i]), true);
    ui->scroll_home->widget()->layout()->addWidget(it);
    connect(it, SIGNAL(RunItem(QString)), this, SLOT(LaunchItem(QString)) );
    connect(it, SIGNAL(NewShortcut()), this, SLOT(updateFavItems()) );
    connect(it, SIGNAL(RemovedShortcut()), this, SLOT(updateFavItems()) );
  }
  static_cast<QBoxLayout*>(ui->scroll_home->widget()->layout())->addStretch();
}
