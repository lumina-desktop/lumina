//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "MainUI.h"
#include "ui_MainUI.h"

#include "ConfigUI.h"

#include <LUtils.h>

MainUI::MainUI() : QMainWindow(), ui(new Ui::MainUI){
  ui->setupUi(this); //load the designer file
  //setupIcons();
  ui->radio_apps->setChecked(true); //always default to starting here
  ui->tool_stop->setVisible(false); //no search running initially
  ui->tool_configure->setVisible(false); //app search initially set
	
  livetime = new QTimer(this);
    livetime->setInterval(500); //1/2 second for live searches
    livetime->setSingleShot(true);
    
  workthread = new QThread(this);
	workthread->setObjectName("Lumina Search Process");
	
  searcher = new Worker();
    searcher->moveToThread(workthread);
	
  closeShort = new QShortcut(QKeySequence(tr("Esc")), this);
	
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
  connect(closeShort, SIGNAL(activated()), this, SLOT( close() ) );
  
  //Setup the settings file
  settings = LUtils::openSettings("lumina-desktop", "lumina-search",this);
  searcher->startDir = settings->value("StartSearchDir", QDir::homePath()).toString();
  searcher->skipDirs = settings->value("SkipSearchDirs", QStringList()).toStringList();
  updateDefaultStatusTip();
  this->show();
  workthread->start();
  QTimer::singleShot(0,this, SLOT(setupIcons()) );
}

MainUI::~MainUI(){
  searcher->StopSearch();
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

// ===============
//    PUBLIC FUNCTIONS (for input handling primarily)
// ===============
void MainUI::disableExcludes(){
  searcher->skipDirs.clear();
  updateDefaultStatusTip();
}

void MainUI::setSearchDirectory(QString path){
  ui->radio_files->setChecked(true);
  searcher->startDir = path;
  updateDefaultStatusTip();
}

void MainUI::setSearchTerm(QString text){
  ui->line_search->setText(text);
}

//==============
//  PRIVATE
//==============
void MainUI::updateDefaultStatusTip(){
  if(ui->radio_files->isChecked()){
    QString txt = tr("Search: %1 -- Smart: %2");
    QString dir = searcher->startDir;
      dir.replace(QDir::homePath(), "~");
    QString smart = searcher->skipDirs.isEmpty() ? tr("Off"): tr("On");
    txt = txt.arg(dir,smart);
    ui->statusbar->showMessage(txt);
  }
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
    dlg.loadInitialValues( settings->value("StartSearchDir",QDir::homePath()).toString(), settings->value("SkipSearchDirs",QStringList()).toStringList());
    dlg.exec();
  if(dlg.newStartDir.isEmpty()){ return; }//cancelled
  QString startdir = dlg.newStartDir;
  QStringList skipdirs = dlg.newSkipDirs;
	
  //Save these values for later (if selected)
  if(dlg.newDefaults){
    //save these values as the new defaults
    settings->setValue("StartSearchDir", startdir);
    settings->setValue("SkipSearchDirs", skipdirs);
  }
  
  //Set these values in the searcher
  searcher->startDir = startdir;
  searcher->skipDirs = skipdirs;
  updateDefaultStatusTip();
}

void MainUI::searchChanged(){
  if(livetime->isActive()){ livetime->stop(); }
  livetime->start();
}
	
//Worker Interaction
void MainUI::startSearch(){
  ui->listWidget->clear();
  stopSearch(); //just in case a search is still running
  if(ui->line_search->text().isEmpty()){ updateDefaultStatusTip(); return; } //nothing to search for
  
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
    XDGDesktop desk(path);
    if( !desk.isValid() ){delete it; return; } //invalid file
    it->setText(desk.name);
    it->setIcon( LXDG::findIcon(desk.icon, "application-x-desktop") );
  }else{
    if(QFileInfo(path).isDir()){ 
      it->setIcon( LXDG::findIcon("inode-directory","") );
      it->setText( path.replace(QDir::homePath(), "~") );
    }else{ 
      if(QFileInfo(path).isExecutable()){
	it->setIcon( LXDG::findIcon("application-x-executable","") ); 
      }else{
        it->setIcon( LXDG::findMimeIcon(path.section("/",-1).section(".",-1)) ); 
      }
      it->setText( path.section("/",-1) );
    }
    
  }
  //Now add it to the widget
  ui->listWidget->addItem(it);
  if(ui->listWidget->count()>100){ searcher->StopSearch(); } //just in case
}

void MainUI::stopSearch(){
  searcher->StopSearch();
  ui->tool_stop->setVisible(false);
  ui->tool_configure->setVisible(ui->radio_files->isChecked());
  updateDefaultStatusTip();
}

void MainUI::searchMessage(QString msg){
  ui->statusbar->showMessage(msg,2000);
}

void MainUI::searchFinished(){
  ui->tool_stop->setVisible(false);
  ui->tool_configure->setVisible(ui->radio_files->isChecked());
  updateDefaultStatusTip();
}
