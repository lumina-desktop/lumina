//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "MainUI.h"
#include "ui_MainUI.h"

#include "ConfigUI.h"

MainUI::MainUI() : QMainWindow(), ui(new Ui::MainUI){
  ui->setupUi(this); //load the designer file
  //setupIcons();
  ui->radio_apps->setChecked(true); //always default to starting here
  ui->tool_stop->setVisible(false); //no search running initially
  ui->tool_configure->setVisible(false); //app search initially set
	
  livetime = new QTimer(this);
    livetime->setInterval(300); //1/3 second for live searches
    livetime->setSingleShot(true);
    
  workthread = new QThread(this);
	workthread->setObjectName("Lumina Search Process");
	
  searcher = new Worker();
    searcher->moveToThread(workthread);
	
  //Setup the connections
  connect(livetime, SIGNAL(timeout()), this, SLOT(startSearch()) );
  connect(this, SIGNAL(SearchTerm(QString, bool)), searcher, SLOT(StartSearch(QString, bool)) );
  connect(searcher, SIGNAL(FoundItem(QString)), this, SLOT(foundSearchItem(QString)) );
  connect(searcher, SIGNAL(SearchUpdate(QString)), this, SLOT(searchMessage(QString)) );
  connect(searcher, SIGNAL(SearchDone()), this, SLOT(searchFinished()) );
  connect(ui->tool_stop, SIGNAL(clicked()), this, SLOT(stopSearch()) );
  connect(ui->push_done, SIGNAL(clicked()), this, SLOT(closeApplication()) );
  connect(ui->push_launch, SIGNAL(clicked()), this, SLOT(LaunchItem()) );
  connect(ui->line_search, SIGNAL(textEdited(QString)), this, SLOT(searchChanged()) );
  connect(ui->line_search, SIGNAL(returnPressed()), this, SLOT(LaunchItem()) );
  connect(ui->radio_apps, SIGNAL(toggled(bool)), this, SLOT(searchTypeChanged()) );
  connect(ui->listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(LaunchItem(QListWidgetItem*)) );
  connect(ui->listWidget, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(LaunchItem(QListWidgetItem*)) );
  connect(ui->tool_configure, SIGNAL(clicked()), this, SLOT(configureSearch()) );
  
  //Setup the settings file
  //TODO: load Json instead of Settings
  QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, QDir::homePath()+"/.lumina");
  settings = new QSettings("LuminaDE", "lumina-search",this);
  searcher->startDir = settings->value("StartSearchDir", QDir::homePath()).toString();
  searcher->skipDirs = settings->value("SkipSearchDirs", QStringList()).toStringList();
  
  this->show();
  workthread->start();
  QTimer::singleShot(0,this, SLOT(setupIcons()) );
}

MainUI::~MainUI(){
  workthread->quit();
  workthread->wait();	
}

void MainUI::setupIcons(){
  //Setup the icons
  this->setWindowIcon( LXDG::findIcon("edit-find","") );
  ui->push_launch->setIcon( LXDG::findIcon("quickopen","") );
  ui->push_done->setIcon( LXDG::findIcon("window-close","") );
  ui->tool_stop->setIcon( LXDG::findIcon("dialog-cancel","") );
  ui->tool_configure->setIcon( LXDG::findIcon("configure","") );
}

//==============
//  PRIVATE SLOTS
//==============
void MainUI::LaunchItem(){
int index = ui->listWidget->currentRow();
if(index<0 && ui->listWidget->count()>0){ index = 0; } //grab the first item instead
else if(index<0){ return; } //no items available/selected
QString item = ui->listWidget->item(index)->whatsThis();
QProcess::startDetached("lumina-open \""+item+"\"");
//Close the search utility if an application was launched (quick launch functionality)
if(ui->radio_apps->isChecked()){ this->close(); }
}

void MainUI::LaunchItem(QListWidgetItem *item){
  QProcess::startDetached("lumina-open \""+item->whatsThis()+"\"");
}

void MainUI::searchTypeChanged(){
  startSearch();	
}
	
void MainUI::configureSearch(){
  ConfigUI dlg(this);
    dlg.loadInitialValues(); 
    dlg.exec();
  if(dlg.newStartDir.isEmpty()){ return; }//cancelled
  QString startdir = dlg.newStartDir;
  QStringList skipdirs = dlg.newSkipDirs;
  //TODO: add setName
	
  //Save these values for later
  settings->setValue("StartSearchDir", startdir);
  settings->setValue("SkipSearchDirs", skipdirs);
	
  //TOTO: change from settings to Json
  //Set these values in the searcher
  searcher->startDir = startdir;
  searcher->skipDirs = skipdirs;
}

void MainUI::searchChanged(){
  if(livetime->isActive()){ livetime->stop(); }
  livetime->start();
}
	
//Worker Interaction
void MainUI::startSearch(){
  ui->listWidget->clear();
  stopSearch(); //just in case a search is still running
  if(ui->line_search->text().isEmpty()){ return; } //nothing to search for
  
  //emit the proper signal for the worker
  if(!workthread->isRunning()){ workthread->start(); } //make sure the thread is running
  emit SearchTerm(ui->line_search->text(), ui->radio_apps->isChecked());
  ui->tool_stop->setVisible(true);
  ui->tool_configure->setVisible(false);
}

void MainUI::foundSearchItem(QString path){
   //To get the worker's results
  QListWidgetItem *it = new QListWidgetItem;
    it->setWhatsThis(path);
    it->setToolTip(path);
  //Now setup the visuals
  if(path.simplified().endsWith(".desktop")){
    bool ok = false;
    XDGDesktop desk = LXDG::loadDesktopFile(path,ok);
    if( !ok  || !LXDG::checkValidity(desk) ){delete it; return; } //invalid file
    it->setText(desk.name);
    it->setIcon( LXDG::findIcon(desk.icon, "application-x-desktop") );
  }else{
    if(QFileInfo(path).isDir()){ 
      it->setIcon( LXDG::findIcon("inode-directory","") );
      it->setText( path.replace(QDir::homePath(), "~") );
    }else{ 
      it->setIcon( LXDG::findMimeIcon(path.section("/",-1).section(".",-1)) ); 
      it->setText( path.section("/",-1) );
    }
    
  }
  //Now add it to the widget
  ui->listWidget->addItem(it);
}

void MainUI::stopSearch(){
  searcher->StopSearch();
  ui->tool_stop->setVisible(false);
  ui->tool_configure->setVisible(ui->radio_files->isChecked());
}

void MainUI::searchMessage(QString msg){
  ui->statusbar->showMessage(msg,2000);
}

void MainUI::searchFinished(){
  ui->tool_stop->setVisible(false);
  ui->tool_configure->setVisible(ui->radio_files->isChecked());
}
