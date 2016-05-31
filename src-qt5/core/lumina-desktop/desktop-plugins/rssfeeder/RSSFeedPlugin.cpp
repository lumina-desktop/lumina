//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "RSSFeedPlugin.h"
#include "ui_RSSFeedPlugin.h"

#include <LuminaXDG.h>
#include "LSession.h"
#include <LuminaUtils.h>
#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QtConcurrent>

RSSFeedPlugin::RSSFeedPlugin(QWidget* parent, QString ID) : LDPlugin(parent, ID), ui(new Ui::RSSFeedPlugin()){
  ui->setupUi(this);
  //Load the global settings 
  setprefix = "rssreader/"; //this structure/prefix should be used for *all* plugins of this type

  //Create the options menu
  optionsMenu = new QMenu(this);
  ui->tool_options->setMenu(optionsMenu);
  //Setup any signal/slot connections
  connect(ui->push_back1, SIGNAL(clicked()), this, SLOT(backToFeeds()) );
  connect(ui->push_back2, SIGNAL(clicked()), this, SLOT(backToFeeds()) );
  connect(ui->push_back3, SIGNAL(clicked()), this, SLOT(backToFeeds()) );
  connect(ui->push_save_settings, SIGNAL(clicked()), this, SLOT(saveSettings()) );

  updateOptionsMenu();
  QTimer::singleShot(0,this, SLOT(loadIcons()) );
  //qDebug() << " - Done with init";
}

RSSFeedPlugin::~RSSFeedPlugin(){

}

//================
//     PRIVATE
//================
void RSSFeedPlugin::updateOptionsMenu(){
  optionsMenu->clear();
  optionsMenu->addAction(LXDG::findIcon("list-add",""), tr("Add RSS Feed"), this, SLOT(openFeedNew()) );
  optionsMenu->addAction(LXDG::findIcon("help-about",""), tr("View Feed Details"), this, SLOT(openFeedInfo()) );
  optionsMenu->addAction(LXDG::findIcon("configure",""), tr("Settings"), this, SLOT(openSettings()) );
  optionsMenu->addSeparator();
  optionsMenu->addAction(LXDG::findIcon("download",""), tr("Update Feeds Now"), this, SLOT(resync()) );
}

//Simplification functions for loading feed info onto widgets
void RSSFeedPlugin::updateFeed(QString ID){

}

void RSSFeedPlugin::updateFeedInfo(QString ID){

}

//================
//  PRIVATE SLOTS
//================
void RSSFeedPlugin::loadIcons(){
  ui->tool_options->setIcon( LXDG::findIcon("configure","") );
  ui->tool_gotosite->setIcon( LXDG::findIcon("applications-internet","") );
  ui->push_back1->setIcon( LXDG::findIcon("go-previous","") );
  ui->push_back2->setIcon( LXDG::findIcon("go-previous","") );
  ui->push_back3->setIcon( LXDG::findIcon("go-previous","") );
  ui->push_rm_feed->setIcon( LXDG::findIcon("list-remove","") );
  ui->push_add_url->setIcon( LXDG::findIcon("list-add","") );
  ui->push_save_settings->setIcon( LXDG::findIcon("document-save","") );
}

//GUI slots
// - Page management
void RSSFeedPlugin::backToFeeds(){
  ui->stackedWidget->setCurrentWidget(ui->page_feed);
}

void RSSFeedPlugin::openFeedInfo(){
  ui->stackedWidget->setCurrentWidget(ui->page_feed_info);
  
}

void RSSFeedPlugin::openFeedNew(){
  ui->line_new_url->setText("");
  ui->stackedWidget->setCurrentWidget(ui->page_new_feed);
}

void RSSFeedPlugin::openSettings(){
  //Sync the widget with the current settings
  QSettings *set = LSession::handle()->DesktopPluginSettings();

  ui->check_manual_sync->setChecked( set->value(setprefix+"manual_sync_only", false).toBool() );
  int DI = set->value(setprefix+"default_interval_minutes", 60).toInt();
  if(DI<1){ DI = 60; }
  if( (DI%60) == 0 ){DI = DI/60; ui->combo_sync_units->setCurrentIndex(1); } //hourly setting
  else{ ui->combo_sync_units->setCurrentIndex(1);  } //minutes setting
  ui->spin_synctime->setValue(DI);

  //Now show the page
  ui->stackedWidget->setCurrentWidget(ui->page_settings);
}

// - Feed Management
void RSSFeedPlugin::addNewFeed(){
  if(ui->line_new_url->text().isEmpty()){ return; } //nothing to add
  //Validate the URL
  QUrl url(ui->line_new_url->text());
  if(!url.isValid()){
    ui->line_new_url->setFocus();
    return;
  }

  //Set this URL as the current selection
  ui->combo_feed->setWhatsThis(url.toString()); //hidden field - will trigger an update in a moment
   //Add the URL to the backend
   
  UpdateFeedList(); //now re-load the feeds which are available

  //Now go back to the main page
  backToFeeds();
}

void RSSFeedPlugin::removeFeed(){
  QString ID = ui->page_feed_info->whatsThis();
  if(ID.isEmpty()){ return; }
  //Remove from the RSS feed object
  
  //Update the feed list
  UpdateFeedList(); //now re-load the feeds which are available
  //Now go back to the main page
  backToFeeds();
}

// - Feed Interactions
void RSSFeedPlugin::currentFeedChanged(){
  QString ID = ui->combo_feed->currentData().toString();
  if(ID.isEmpty()){ return; } //no feed selected
}

void RSSFeedPlugin::openFeedPage(){ //Open main website for feed
  QString ID = ui->combo_feed->currentData().toString();
  //Find the data associated with this feed
  QString url;

  //Now launch the browser
  if(!url.isEmpty()){
    QProcess::startDetached("lumina-open \""+url+"\"");
  }
}

void RSSFeedPlugin::saveSettings(){
  QSettings *set = LSession::handle()->DesktopPluginSettings();
  set->setValue(setprefix+"manual_sync_only", ui->check_manual_sync->isChecked() );
  int DI = ui->spin_synctime->value();
  if(ui->combo_sync_units->currentIndex()==1){ DI = DI*60; } //convert from hours to minutes
  set->setValue(setprefix+"default_interval_minutes", DI);
  set->sync();
  //Now prod the feed object that something changed
  
  //Now go back to the feeds
  backToFeeds();
}

//Feed Object interactions
void RSSFeedPlugin::UpdateFeedList(){

}

void RSSFeedPlugin::RSSItemChanged(QString ID){

}
//==================
//   PUBLIC SLOTS
//==================
void RSSFeedPlugin::LocaleChange(){
 ui->retranslateUi(this);
  updateOptionsMenu();
}
void RSSFeedPlugin::ThemeChange(){
  QTimer::singleShot(0,this, SLOT(loadIcons()));
  updateOptionsMenu();
}
