//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "MainUI.h"
#include "ui_MainUI.h"

#include <QMenu>
#include <QFileInfo>
#include "gitCompat.h"
#include "gitWizard.h"

#include <LUtils.h>
#include <LDesktopUtils.h>

#define DEBUG 0

MainUI::MainUI() : QMainWindow(), ui(new Ui::MainUI){
  //for Signal/slot we must register the Typedef of QFileInfoList
  //qRegisterMetaType<QFileInfoList>("QFileInfoList");
  qRegisterMetaType< LFileInfoList >("LFileInfoList");
  //just to silence/fix some Qt connect warnings in QtConcurrent
  //qRegisterMetaType< QVector<int> >("QVector<int>"); 
  //qRegisterMetaType< QList<QPersistentModelIndex> >("QList<QPersistentModelIndex>");
  waitingToClose = false;
	
  ui->setupUi(this);
  if(DEBUG){ qDebug() << "Initilization:"; }
  settings = LUtils::openSettings("lumina-desktop", "lumina-fm", this);

  //Reset the UI to the previously used size (if possible)
QSize orig = settings->value("preferences/MainWindowSize", QSize()).toSize();
  if(!orig.isEmpty() && orig.isValid()){
    //Make sure the old size is larger than the default size hint
    if(orig.width() < this->sizeHint().width()){ orig.setWidth(this->sizeHint().width()); }
    if(orig.height() < this->sizeHint().height()){ orig.setHeight(this->sizeHint().height()); }    
    //Also ensure the old size is smaller than the current screen size
    QSize screen = QApplication::desktop()->availableGeometry(this).size();
    if(orig.width() > screen.width()){ orig.setWidth(screen.width()); }
    if(orig.height() > screen.height()){ orig.setHeight(screen.height()); }
    //Now resize the window
    this->resize(orig);
  }
  //initialize the non-ui widgets
  if(DEBUG){ qDebug() << " - Tab Bar Setup"; }
  tabBar = new QTabBar(this);
    tabBar->setTabsClosable(true);
    tabBar->setMovable(true); //tabs are independant - so allow the user to sort them
    tabBar->setShape(QTabBar::RoundedNorth);
    tabBar->setFocusPolicy(Qt::NoFocus);
    static_cast<QBoxLayout*>(ui->centralwidget->layout())->insertWidget(0,tabBar);
  if(DEBUG){ qDebug() << " - Threading"; }
  workThread = new QThread;
    workThread->setObjectName("Lumina-fm filesystem worker");
  worker = new DirData();
    worker->zfsavailable = LUtils::isValidBinary("zfs");
    //connect(worker, SIGNAL(DirDataAvailable(QString, QString, LFileInfoList)), this, SLOT(DirDataAvailable(QString, QString, LFileInfoList)) );
    connect(worker, SIGNAL(SnapshotDataAvailable(QString, QString, QStringList)), this, SLOT(SnapshotDataAvailable(QString, QString, QStringList)) );
    worker->moveToThread(workThread);
  if(DEBUG){ qDebug() << " - Context Menu"; }
  contextMenu = new QMenu(this);
  radio_view_details = new QRadioButton(tr("Detailed List"), this);
  radio_view_list = new QRadioButton(tr("Basic List"), this);
  //radio_view_tabs = new QRadioButton(tr("Prefer Tabs"), this);
  //radio_view_cols = new QRadioButton(tr("Prefer Columns"), this);
  ui->menuView_Mode->clear();
  //ui->menuGroup_Mode->clear();
  detWA = new QWidgetAction(this);
    detWA->setDefaultWidget(radio_view_details);
  listWA = new QWidgetAction(this);
    listWA->setDefaultWidget(radio_view_list);
  //tabsWA = new QWidgetAction(this);
    //tabsWA->setDefaultWidget(radio_view_tabs);
  //colsWA = new QWidgetAction(this);
    //colsWA->setDefaultWidget(radio_view_cols);
    ui->menuView_Mode->addAction(detWA);
    ui->menuView_Mode->addAction(listWA);
    //ui->menuGroup_Mode->addAction(tabsWA);
    //ui->menuGroup_Mode->addAction(colsWA);
  //Setup the pages
  //ui->BrowserLayout->clear();
  ui->page_player->setLayout(new QVBoxLayout());
  ui->page_image->setLayout(new QVBoxLayout());
  MW = new MultimediaWidget(this);
  SW = new SlideshowWidget(this);
  ui->page_player->layout()->addWidget(MW);
  ui->page_image->layout()->addWidget(SW);

  //Setup any specialty keyboard shortcuts
  if(DEBUG){ qDebug() << " - Keyboard Shortcuts"; }
  nextTabLShort = new QShortcut( QKeySequence(tr("Shift+Left")), this);
  nextTabRShort = new QShortcut( QKeySequence(tr("Shift+Right")), this);
  togglehiddenfilesShort = new QShortcut( QKeySequence(tr("Ctrl+H")), this);
  focusDirWidgetShort = new QShortcut( QKeySequence(tr("Ctrl+L")), this);

  //Finish loading the interface
  workThread->start();
  if(DEBUG){ qDebug() << " - Icons"; }
  setupIcons();
  if(DEBUG){ qDebug() << " - Connections"; }
  setupConnections();
  if(DEBUG){ qDebug() << " - Settings"; }
  loadSettings();
  if(DEBUG){ qDebug() << " - Bookmarks"; }
  RebuildBookmarksMenu();
  if(DEBUG){ qDebug() << " - Devices"; }
  RebuildDeviceMenu();
  //Make sure we start on the browser page
  TRAY = new TrayUI(this);
  connect(TRAY, SIGNAL(JobsFinished()), this, SLOT(TrayJobsFinished()) );
  if(DEBUG){ qDebug() << " - Done with init"; }
}

MainUI::~MainUI(){
  for(int i=0; i<DWLIST.length(); i++){
    DWLIST[i]->cleanup();
  }
  workThread->quit();
  //Also ensure the work thread is stopped
//  workThread->wait();
}

void MainUI::OpenDirs(QStringList dirs){
  //Now open the dirs
  if(dirs.isEmpty()){ dirs << QDir::homePath(); }
  QStringList invalid;
  for(int i=0; i<dirs.length(); i++){
    if(dirs[i].simplified().isEmpty()){ continue; }
    //Open this directory in a viewer
    if(dirs[i].endsWith("/")){ dirs[i].chop(1); }
    if(!QFile::exists(dirs[i])){ invalid << dirs[i]; continue; }
    if(DEBUG){ qDebug() << "Open Directory:" << dirs[i]; }
    ///Get a new Unique ID
    int id = 0;
    for(int j=0; j<DWLIST.length(); j++){ 
      if(DWLIST[j]->id().section("-",1,1).toInt() >= id){ id = DWLIST[j]->id().section("-",1,1).toInt()+1; }
    }
    //Create the new DirWidget
    DirWidget *DW = new DirWidget("DW-"+QString::number(id), this);
    DW->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->BrowserLayout->addWidget(DW);
    DWLIST << DW;
    //Connect the signals/slots for it
    connect(DW, SIGNAL(OpenDirectories(QStringList)), this, SLOT(OpenDirs(QStringList)) );
    //connect(DW, SIGNAL(LoadDirectory(QString, QString)), worker, SLOT(GetDirData(QString, QString)) );
    connect(DW, SIGNAL(findSnaps(QString, QString)), worker, SLOT(GetSnapshotData(QString, QString)) );
    connect(DW, SIGNAL(PlayFiles(LFileInfoList)), this, SLOT(OpenPlayer(LFileInfoList)) );
    connect(DW, SIGNAL(ViewFiles(LFileInfoList)), this, SLOT(OpenImages(LFileInfoList)) );
    connect(DW, SIGNAL(LaunchTerminal(QString)), this, SLOT(OpenTerminal(QString)) );
    connect(DW, SIGNAL(CutFiles(QStringList)), this, SLOT(CutFiles(QStringList)) );
    connect(DW, SIGNAL(CopyFiles(QStringList)), this, SLOT(CopyFiles(QStringList)) );
    connect(DW, SIGNAL(FavoriteFiles(QStringList)), this, SLOT(FavoriteFiles(QStringList)) );
    connect(DW, SIGNAL(RenameFiles(QStringList)), this, SLOT(RenameFiles(QStringList)) );
    connect(DW, SIGNAL(RemoveFiles(QStringList)), this, SLOT(RemoveFiles(QStringList)) );
    connect(DW, SIGNAL(PasteFiles(QString,QStringList)), this, SLOT(PasteFiles(QString, QStringList)) );
    connect(DW, SIGNAL(CloseBrowser(QString)), this, SLOT(CloseBrowser(QString)) );
    connect(DW, SIGNAL(TabNameChanged(QString,QString)), this, SLOT(TabNameChanged(QString, QString)) );
    //Now create the tab for this 
    //if(radio_view_tabs->isChecked()){
      int index = tabBar->addTab( LXDG::findIcon("folder-open",""), dirs[i].section("/",-1) );
      tabBar->setTabWhatsThis( index, "DW-"+QString::number(id) );
      tabBar->setCurrentIndex(index);
    /*}else{
      //Just make sure the browser tab is visible
      bool found = false;
      for(int i=0; i<tabBar->count() && !found; i++){
        if(tabBar->tabWhatsThis(i)=="browser"){ tabBar->setCurrentIndex(i); found=true; }
      }
      if(!found){
        //Need to create the generic Browser tab
        int index = tabBar->addTab( LXDG::findIcon("folder-open",""), "Browser" );
        tabBar->setTabWhatsThis( index, "browser" );
        tabBar->setCurrentIndex(index);
      }
    }*/
    
    //Initialize the widget with the proper settings
    DW->setShowDetails(radio_view_details->isChecked()); 
    DW->setThumbnailSize(settings->value("iconsize", 32).toInt());
    DW->showHidden( ui->actionView_Hidden_Files->isChecked() );
    DW->showThumbnails( ui->actionShow_Thumbnails->isChecked() );
    //Now load the directory
    DW->ChangeDir(dirs[i]); //kick off loading the directory info
  }
  //Update visibilities
  tabChanged(tabBar->currentIndex());
  tabBar->setVisible( tabBar->count() > 1 );
  if(!invalid.isEmpty()){
    QMessageBox::warning(this, tr("Invalid Directories"), tr("The following directories are invalid and could not be opened:")+"\n"+invalid.join(", ") );
  }
  //Double check that there is at least 1 dir loaded
  //qDebug() << "OpenDirs:" << DWLIST.length() << dirs << invalid << tabBar->currentIndex();
  if(DWLIST.isEmpty()){ OpenDirs(QStringList()); }
  waitingToClose = false;
  ui->menuGit->setEnabled( GIT::isAvailable() );
  this->showNormal(); //single-instance check - make sure the window is raised again if it was minimized
}

void MainUI::setupIcons(){
  this->setWindowIcon( LXDG::findIcon("Insight-FileManager","") );
	
  //Setup all the icons using libLumina
  // File menu
  ui->actionNew_Window->setIcon( LXDG::findIcon("window-new","") );
  ui->actionNew_Tab->setIcon( LXDG::findIcon("tab-new","") );
  ui->actionSearch->setIcon( LXDG::findIcon("edit-find","") );
  ui->actionClose_Browser->setIcon( LXDG::findIcon("tab-close","") );
  ui->actionClose->setIcon( LXDG::findIcon("application-exit","") );

  // Edit menu
  ui->actionRename->setIcon( LXDG::findIcon("edit-rename","") );
  ui->actionCut_Selection->setIcon( LXDG::findIcon("edit-cut","") );
  ui->actionCopy_Selection->setIcon( LXDG::findIcon("edit-copy","") );
  ui->actionPaste->setIcon( LXDG::findIcon("edit-paste","") );
  ui->actionDelete_Selection->setIcon( LXDG::findIcon("edit-delete","") );

  // View menu
  ui->actionRefresh->setIcon( LXDG::findIcon("view-refresh","") );
  ui->menuView_Mode->setIcon( LXDG::findIcon("view-choose","") );

  // Bookmarks menu
  ui->actionManage_Bookmarks->setIcon( LXDG::findIcon("bookmarks-organize","") );
  ui->actionManage_Bookmarks->setShortcut(tr("CTRL+B"));
  ui->actionAdd_Bookmark->setIcon( LXDG::findIcon("bookmark-new","") );

  //GIT menu
  ui->actionRepo_Status->setIcon( LXDG::findIcon("git","document-edit-verify") );
  ui->actionClone_Repository->setIcon( LXDG::findIcon("git","download") );

  // External Devices menu
  ui->actionScan->setIcon( LXDG::findIcon("system-search","") );
  ui->actionScan->setShortcut(tr("CTRL+E"));
}

//==========
//    PRIVATE
//==========
void MainUI::setupConnections(){
  connect(tabBar, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)) );
  connect(tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(tabClosed(int)) );
  connect(ui->menuBookmarks, SIGNAL(triggered(QAction*)), this, SLOT(goToBookmark(QAction*)) );
  connect(ui->menuExternal_Devices, SIGNAL(triggered(QAction*)), this, SLOT(goToDevice(QAction*)) );

  //Radio Buttons
  connect(radio_view_details, SIGNAL(toggled(bool)), this, SLOT(viewModeChanged(bool)) );
  connect(radio_view_list, SIGNAL(toggled(bool)), this, SLOT(viewModeChanged(bool)) );
  //connect(radio_view_tabs, SIGNAL(toggled(bool)), this, SLOT(groupModeChanged(bool)) );
  //connect(radio_view_cols, SIGNAL(toggled(bool)), this, SLOT(groupModeChanged(bool)) );

  //Special Keyboard Shortcuts
  connect(nextTabLShort, SIGNAL(activated()), this, SLOT( prevTab() ) );
  connect(nextTabRShort, SIGNAL(activated()), this, SLOT( nextTab() ) );
  connect(togglehiddenfilesShort, SIGNAL(activated()), this, SLOT( togglehiddenfiles() ) );
  connect(focusDirWidgetShort, SIGNAL(activated()), this, SLOT( focusDirWidget() ) );
}

void MainUI::focusDirWidget()
{
  DirWidget *dir = FindActiveBrowser();
  if(dir != 0) { dir->setFocusLineDir(); }
}

void MainUI::togglehiddenfiles()
{
    //change setChecked to inverse value
    ui->actionView_Hidden_Files->setChecked( !settings->value("showhidden", true).toBool() );
    // then trigger function
    on_actionView_Hidden_Files_triggered();
}

void MainUI::loadSettings(){
  //Note: make sure this is run after all the UI elements are created and connected to slots
  // but before the first directory gets loaded
  ui->actionView_Hidden_Files->setChecked( settings->value("showhidden", false).toBool() );
    on_actionView_Hidden_Files_triggered(); //make sure to update the models too
  ui->actionShow_Thumbnails->setChecked( settings->value("showthumbnails",true).toBool());
    on_actionShow_Thumbnails_triggered(); //make sure to update models too
  //ui->actionShow_Action_Buttons->setChecked(settings->value("showactions", true).toBool() );
    //on_actionShow_Action_Buttons_triggered(); //make sure to update the UI
  //ui->actionShow_Thumbnails->setChecked( settings->value("showthumbnails", true).toBool() );
  //View Type
  //qDebug() << "View Mode:" << settings->value("viewmode","details").toString();
  bool showDetails = (settings->value("viewmode","details").toString()=="details");
  if(showDetails){ radio_view_details->setChecked(true); }
  else{ radio_view_list->setChecked(true); }
  //Grouping type
  //bool usetabs = (settings->value("groupmode","tabs").toString()=="tabs");
  //if(usetabs){ radio_view_tabs->setChecked(true); }
 // else{ radio_view_cols->setChecked(true); }
  
}

void MainUI::RebuildBookmarksMenu(){
  //Create the bookmarks menu
  QStringList BM = settings->value("bookmarks", QStringList()).toStringList();
  ui->menuBookmarks->clear();
    ui->menuBookmarks->addAction(ui->actionManage_Bookmarks);
    ui->menuBookmarks->addAction(ui->actionAdd_Bookmark);
    ui->menuBookmarks->addSeparator();
  bool changed = false;
  BM.sort(); //Sort alphabetically
  for(int i=0; i<BM.length(); i++){
    //NOTE 9/28/16: Don't do existance checks here - if a network drive is specified it can cause the loading process to hang significantly
    //if(QFile::exists(BM[i].section("::::",1,1)) ){
      QAction *act = new QAction(BM[i].section("::::",0,0),this);
        act->setWhatsThis(BM[i].section("::::",1,1));
      ui->menuBookmarks->addAction(act);
    /*}else{
      //Invalid directory - remove the bookmark
      BM.removeAt(i);
      i--;
      changed = true;
    }*/
  }
  if(changed){ settings->setValue("bookmarks",BM); }
  ui->actionManage_Bookmarks->setEnabled(BM.length()>0);
}

void MainUI::RebuildDeviceMenu(){
  //Create the External Devices Menu appropriately
  ui->menuExternal_Devices->clear();
    ui->menuExternal_Devices->addAction( ui->actionScan );
    ui->menuExternal_Devices->addSeparator();
  //Scan for externally mounted devices
  QStringList devs = LOS::ExternalDevicePaths();
    //Output Format: <type>::::<filesystem>::::<path>  (6/24/14 - version 0.4.0 )
        // <type> = [USB, HDRIVE, SDCARD, DVD, LVM, UNKNOWN]
  qDebug() << "Externally-mounted devices:" << devs;
  //Now add them to the menu appropriately
  for(int i=0; i<devs.length(); i++){
    //Skip hidden mount points (usually only for system usage - not user browsing)
    QString label, path, fs;
      //Format inputs as necesary
      path = devs[i].section("::::",2,2);
      fs = devs[i].section("::::",1,1);
      if(path.endsWith("/") && path.length()>1 ){ path.chop(1); }
      if(path == "/"){ label = tr("Root"); }
      else{ label = path.section("/",-1).simplified(); }
    if(label.startsWith(".") ){ continue; }  //don't show hidden mountpoint (not usually user-browsable)
    //Create entry for this device
    if( !fs.simplified().isEmpty()){
      //Add filesystem type to the label
      label = QString(tr("%1 (Type: %2)")).arg(label, fs);
    }
    QAction *act = new QAction(label,this);          
        act->setWhatsThis(path); //full path to mountpoint
	act->setToolTip( QString(tr("Filesystem: %1")).arg( devs[i].section("::::",1,1) ) );
	//Now set the appropriate icon
	QString type = devs[i].section("::::",0,0);
	if(type=="USB"){ type = "drive-removable-media-usb"; }
	else if(type=="HDRIVE" || type=="LVM"){ type = "drive-harddisk"; }
	else if(type=="SDCARD"){ type = "media-flash-sd-mmc"; }
	else if(type=="DVD"){ type = "media-optical"; }
	else{ type = "drive-removable-media"; }
	act->setIcon( LXDG::findIcon(type, "") );
      ui->menuExternal_Devices->addAction(act);
  }
}

DirWidget* MainUI::FindActiveBrowser(){
  //Find the current directory
  DirWidget *curB = 0;
  //Get the current tab ID to start with
  QString cur = tabBar->tabWhatsThis(tabBar->currentIndex());
  //if(cur.startsWith("#")){ cur.clear(); } //multimedia/player tab open
  
  if(DWLIST.length()==1){
    //Only 1 browser open - use it
    curB = DWLIST[0];
  }else if(cur.startsWith("DW-")){
    //This is a tab for a browser - just find the matching widget
    for(int i=0; i<DWLIST.length(); i++){
      if(DWLIST[i]->id()==cur){ curB = DWLIST[i]; break; }
    }
  }else{
    //This is a bit more complex - either showing multiple columns or a non-browser tab is active
    if(cur=="browser"){
      //Column View
      QWidget *focus = QApplication::focusWidget(); //the widget which currently has focus
      for(int i=0; i<DWLIST.length(); i++){
        if(DWLIST[i]->isAncestorOf(focus)){ curB = DWLIST[i]; break; } //This browser has focus
      }
	    
    }else{
      //Non-Browser in focus - use the fallback below
    }
  }
  //if all else fails - just use the first browser in the list (there should always be at least one)
  if(curB==0 && !DWLIST.isEmpty()){ curB = DWLIST[0];  }
  return curB;
}

//==============
//    PRIVATE SLOTS
//==============
void MainUI::DisplayStatusBar(QString msg){
	//qDebug() << "message to show in the status bar:" << msg;
	ui->statusbar->showMessage(msg);
}
	
//---------------------
//Menu Actions
//---------------------
void MainUI::on_actionNew_Window_triggered(){
  QProcess::startDetached("lumina-fm -new-instance");
}

void MainUI::on_actionNew_Tab_triggered(){
  OpenDirs(QStringList() << QDir::homePath());
}

void MainUI::on_actionSearch_triggered(){
  DirWidget *dir = FindActiveBrowser();
  if(dir==0){ return; }
  QProcess::startDetached("lumina-search -dir \""+dir->currentDir()+"\"");
}

void MainUI::on_actionClose_Browser_triggered(){
  tabClosed();
}

void MainUI::on_actionClose_triggered(){
  if(tabBar->count() > 1){
    if(QMessageBox::Yes != QMessageBox::question(this, tr("Verify Quit"), tr("You have multiple tabs open. Are you sure you want to quit?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes ) ){
      return;
    }
  }
  qDebug() << "Closing Down...";
  this->close();
}

/*void MainUI::on_actionRename_triggered(){
  DirWidget *dir = FindActiveBrowser();
  if(DEBUG){ qDebug() << "Rename Shortcut Pressed:" << dir << dir->currentDir(); }
  if(dir!=0){ QTimer::singleShot(0, dir, SLOT(renameFiles()) ); }
}

void MainUI::on_actionCut_Selection_triggered(){
  DirWidget *dir = FindActiveBrowser();
  if(DEBUG){ qDebug() << "Cut Shortcut Pressed:" << dir << dir->currentDir(); }
  if(dir!=0){ QTimer::singleShot(0, dir, SLOT(cutFiles()) );	 }
}

void MainUI::on_actionCopy_Selection_triggered(){
  DirWidget *dir = FindActiveBrowser();
  if(DEBUG){ qDebug() << "Copy Shortcut Pressed:" << dir << dir->currentDir(); }
  if(dir!=0){ QTimer::singleShot(0, dir, SLOT(TryCopySelection()) ); }
}

void MainUI::on_actionPaste_triggered(){
  DirWidget *dir = FindActiveBrowser();
  if(DEBUG){ qDebug() << "Paste Shortcut Pressed:" << dir << dir->currentDir(); }
  if(dir!=0){ QTimer::singleShot(0, dir, SLOT(TryPasteSelection()) ); }
}

void MainUI::on_actionDelete_Selection_triggered(){
  DirWidget *dir = FindActiveBrowser();
  if(DEBUG){ qDebug() << "Delete Shortcut Pressed:" << dir << dir->currentDir(); }
  if(dir!=0){ QTimer::singleShot(0, dir, SLOT(TryDeleteSelection()) ); }
}*/

void MainUI::on_actionRefresh_triggered(){
  DirWidget *cur = FindActiveBrowser();
  if(cur!=0){ cur->refresh(); }
}

void MainUI::on_actionView_Hidden_Files_triggered(){
  worker->showHidden = ui->actionView_Hidden_Files->isChecked();
  //Now save this setting for later
  settings->setValue("showhidden", ui->actionView_Hidden_Files->isChecked());
  //worker->showHidden = ui->actionView_Hidden_Files->isChecked();
  //Re-load the current browsers
  for(int i=0; i<DWLIST.length(); i++){ DWLIST[i]->showHidden( ui->actionView_Hidden_Files->isChecked() ); }//DWLIST[i]->refresh(); }

}

/*void MainUI::on_actionShow_Action_Buttons_triggered(){
  bool show = ui->actionShow_Action_Buttons->isChecked();
  settings->setValue("showactions", show);
  //for(int i=0; i<DWLIST.length(); i++){ DWLIST[i]->setShowSidebar(show); }
}*/

void MainUI::on_actionShow_Thumbnails_triggered(){
  //Now save this setting for later
  bool show = ui->actionShow_Thumbnails->isChecked();
  settings->setValue("showthumbnails", show);
  for(int i=0; i<DWLIST.length(); i++){ DWLIST[i]->showThumbnails(show); }
}

void MainUI::goToBookmark(QAction *act){
  if(act==ui->actionManage_Bookmarks){
    BMMDialog dlg(this);
      dlg.loadSettings(settings);
      dlg.exec();
    RebuildBookmarksMenu();
  }else if(act == ui->actionAdd_Bookmark){
    CreateBookMark();
  }else{
    //Find the current directory
    DirWidget *dir = FindActiveBrowser();
    if(dir!=0){ 
      dir->ChangeDir(act->whatsThis());
      return;
    }
    //If no current dir could be found - open a new tab/column
    OpenDirs(QStringList() << act->whatsThis() );
  }
}

void MainUI::goToDevice(QAction *act){
  if(act==ui->actionScan){
    RebuildDeviceMenu();
  }else{
    DirWidget *dir = FindActiveBrowser();
    if(dir!=0){ 
      dir->ChangeDir(act->whatsThis());
      return;
    }
    //If no current dir could be found - open a new tab/column
    OpenDirs(QStringList() << act->whatsThis() );
  }
}

void MainUI::viewModeChanged(bool active){
  if(!active){ return; } //on every view change, all radio buttons will call this function - only run this once though
  bool showDetails = radio_view_details->isChecked();
  if(showDetails){ settings->setValue("viewmode","details"); }
  else{ settings->setValue("viewmode","list"); }

  //Re-load the view widgets
  for(int i=0; i<DWLIST.length(); i++){
    DWLIST[i]->setShowDetails(showDetails);
  }
	
}

/*void MainUI::groupModeChanged(bool active){
  if(!active){ return; } //on every change, all radio buttons will call this function - only run this once though
  //bool usetabs = radio_view_tabs->isChecked();
 //if(usetabs){ 
    //settings->setValue("groupmode","tabs"); 
    //Now clean up all the tabs (remove the generic one and add the specific ones)
    for(int i=0; i<tabBar->count(); i++){
      //Remove all the browser tabs
      if( !tabBar->tabWhatsThis(i).startsWith("#") ){
	tabBar->removeTab(i);
	i--; //go back one to ensure nothing is missed
      }
    }
    //Create all the specific browser tabs for open browsers
    for(int i=0; i<DWLIST.length(); i++){
      qDebug() << "Add specific tab:" << DWLIST[i]->currentDir() << DWLIST[i]->id();
      int tab = tabBar->addTab( LXDG::findIcon("folder-open",""), DWLIST[i]->currentDir().section("/",-1) );
      tabBar->setTabWhatsThis(tab, DWLIST[i]->id() );
      //DWLIST[i]->setShowCloseButton(false);
    }
  }else{
    settings->setValue("groupmode","columns");
    //Now clean up the tabs (remove the specific ones and add a generic one)
    for(int i=0; i<tabBar->count(); i++){
      //Remove all the browser tabs
      if( !tabBar->tabWhatsThis(i).startsWith("#") ){
	tabBar->removeTab(i);
	i--; //go back one to ensure nothing is missed
      }
    }
    //Now create the generic "browser" tab
    int tab = tabBar->addTab( LXDG::findIcon("folder-open",""), tr("Browser") );
      tabBar->setTabWhatsThis(tab, "browser" );
    //for(int i=0; i<DWLIST.length(); i++){ DWLIST[i]->setShowCloseButton(true); }
  }
  if(tabBar->currentIndex()<0){ tabBar->setCurrentIndex(0); }
  tabBar->setVisible( tabBar->count() > 1 );
  QTimer::singleShot(20, this, SLOT(tabChanged()) );
}*/

void MainUI::on_actionLarger_Icons_triggered(){
  int size = settings->value("iconsize", 32).toInt();
  size += 16;
  for(int i=0; i<DWLIST.length(); i++){ DWLIST[i]->setThumbnailSize(size); }
  settings->setValue("iconsize", size);
}

void MainUI::on_actionSmaller_Icons_triggered(){
  int size = settings->value("iconsize", 32).toInt();
  if(size <= 16){ return; }
  size -= 16;
  for(int i=0; i<DWLIST.length(); i++){ DWLIST[i]->setThumbnailSize(size); }
  settings->setValue("iconsize", size);	
}

void MainUI::CreateBookMark(){
  QString dir = FindActiveBrowser()->currentDir();
  bool ok = false;
  QString name = QInputDialog::getText(this, tr("New Bookmark"), tr("Name:"), QLineEdit::Normal, dir, \
		&ok, 0, Qt::ImhFormattedNumbersOnly | Qt::ImhUppercaseOnly | Qt::ImhLowercaseOnly);
  if(!ok || name.isEmpty()){ return; } //cancelled
  QStringList BM = settings->value("bookmarks",QStringList()).toStringList();
  if(BM.filter(name+"::::").length() >0){
    QMessageBox::warning(this, tr("Invalid Name"), tr("This bookmark name already exists. Please choose another.") );
    QTimer::singleShot(0,this, SLOT(on_actionBookMark_triggered()));
    return;
  }
  BM.append(name+"::::"+dir);
  BM.sort(); //sort alphabetically by name
  settings->setValue("bookmarks", BM);
  //Now rebuild the bookmarks menu
  RebuildBookmarksMenu();
}

//Git Menu options
void MainUI::on_menuGit_aboutToShow(){
  QString dir = FindActiveBrowser()->currentDir();
  bool inrepo = GIT::isRepo(dir);
  ui->actionRepo_Status->setEnabled( inrepo );
  ui->actionClone_Repository->setEnabled( !inrepo );
}

void MainUI::on_actionRepo_Status_triggered(){
  QString status = GIT::status( FindActiveBrowser()->currentDir() );
  QMessageBox::information(this, tr("Git Repository Status"), status);
}

void MainUI::on_actionClone_Repository_triggered(){
  GitWizard *dlg = new GitWizard(this);
    dlg->setWorkingDir( FindActiveBrowser()->currentDir() );
  dlg->show();
}


void MainUI::tabChanged(int tab){
  if(tab<0){ tab = tabBar->currentIndex(); }
  if(tab < 0){ return; }
  //Load the directory contained in the new tab
  QString info = tabBar->tabWhatsThis(tab); //get the full directory
  if(info.isEmpty()){ return; } //unknown tab (this happens while we are in the middle of making changes to tabs - just ignore it)
  //qDebug() << "Change to Tab:" << tab << tabBar->tabText(tab);
  //qDebug() << " -- ID:" << info;
  if(info=="#MW"){ ui->stackedWidget->setCurrentWidget(ui->page_player); }
  else if(info=="#SW"){ ui->stackedWidget->setCurrentWidget(ui->page_image); }
  else{
    ui->stackedWidget->setCurrentWidget(ui->page_browser);
    //if(radio_view_tabs->isChecked()){
      for(int i=0; i<DWLIST.length(); i++){
	 DWLIST[i]->setVisible(DWLIST[i]->id()==info);     
      }
    /*}else{
      //For columns, all widgets need to be visible
      for(int i=0; i<DWLIST.length(); i++){
	 DWLIST[i]->setVisible(true);     
      }	    
    }*/
  }
  tabBar->setVisible( tabBar->count() > 1 );
}

void MainUI::tabClosed(int tab){
  if(tabBar->count()==1){ return; } //Can't close the only tab
  if(tab < 0){ tab = tabBar->currentIndex(); }
  QString info = tabBar->tabWhatsThis(tab);
  if(info=="browser"){ return; }
  //qDebug() << "Tab Closed:" << info;
   if(!info.startsWith("#")){ 
    for(int i=0; i<DWLIST.length(); i++){
      if(info == DWLIST[i]->id()){
	 DWLIST[i]->cleanup();
        delete DWLIST.takeAt(i);
	break;
      }
    }
   }else if(info=="#MW"){
      MW->Cleanup(); //prepare it to be hidden/removed
   }
  //Remove the tab (will automatically move to a different one);
  qDebug() << "Closing tab:" << tab << tabBar->tabText(tab);
  tabBar->removeTab(tab);
  tabBar->setVisible( tabBar->count() > 1 );
  if(DWLIST.isEmpty()){ OpenDirs(QStringList() << QDir::homePath() ); }
}

void MainUI::prevTab(){
  int cur = tabBar->currentIndex();
  if(cur == 0){ tabBar->setCurrentIndex( tabBar->count()-1 ); }
  else{ tabBar->setCurrentIndex( cur-1 ); }
}

void MainUI::nextTab(){
  int cur = tabBar->currentIndex();
  if(cur == (tabBar->count()-1) ){ tabBar->setCurrentIndex(0); }
  else{ tabBar->setCurrentIndex( cur+1 ); }	
}


void MainUI::SnapshotDataAvailable(QString id , QString dir, QStringList list){
  for(int i=0; i<DWLIST.length(); i++){
    if(id == DWLIST[i]->id()){
      DWLIST[i]->LoadSnaps(dir, list);
      break;
    }
  }
}

void MainUI::OpenPlayer(LFileInfoList list){
  //See if the tab is available for the multimedia player
  int tab = -1;
  for(int i=0; i<tabBar->count(); i++){
    if(tabBar->tabWhatsThis(i)=="#MW"){ tab=i; break;}
  }
  if(tab<0){
    //Need to create a new tab
    tab = tabBar->addTab(LXDG::findIcon("media-playback-start",""), tr("Multimedia"));
    tabBar->setTabWhatsThis(tab,"#MW");
    //Also clear the info in the player
    MW->ClearPlaylist();
  }
  //Load the data into the player
  MW->LoadMultimedia(list);
  //Switch to the player tab
  tabBar->setCurrentIndex(tab);
}

void MainUI::OpenImages(LFileInfoList list){
  int tab = -1;
  for(int i=0; i<tabBar->count(); i++){
    if(tabBar->tabWhatsThis(i)=="#SW"){ tab=i; break;}
  }
  if(tab<0){
    //Need to create a new tab
    tab = tabBar->addTab(LXDG::findIcon("media-slideshow",""), tr("Slideshow"));
    tabBar->setTabWhatsThis(tab,"#SW");
    //Also clear the info in the viewer
    SW->ClearImages();
  }
  //Load the data into the viewer
  SW->LoadImages(list);
  //Switch to the player tab
  tabBar->setCurrentIndex(tab);
  QTimer::singleShot(20, SW, SLOT(refresh()) );
}

void MainUI::OpenTerminal(QString dirpath){
  //we use the application defined as the default terminal 
  //QSettings sessionsettings( QSettings::UserScope, "LuminaDE","sessionsettings", this);
  //xterm remains the default
  QString defTerminal = LXDG::findDefaultAppForMime("application/terminal"); //sessionsettings.value("default-terminal", "xterm").toString();
  qDebug() << "Found default terminal:" << defTerminal;
  //Now get the exec string and run it
  QString cmd = LUtils::GenerateOpenTerminalExec(defTerminal, dirpath);
  //qDebug() << "Starting Terminal with command:" << cmd;
  QProcess::startDetached(cmd);

}

void MainUI::CutFiles(QStringList list){
  qDebug() << "Cut Files:" << list;
  if(list.isEmpty()){ return; } //nothing selected
  //Format the data string
  QList<QUrl> urilist; //Also assemble a URI list for cros-app compat (no copy/cut distinguishing)
  for(int i=0; i<list.length(); i++){
    urilist << QUrl::fromLocalFile(list[i]);
    list[i] = list[i].prepend("cut::::");
  }
  //Now save that data to the global clipboard
  QMimeData *dat = new QMimeData;
	dat->clear();
	dat->setData("x-special/lumina-copied-files", list.join("\n").toLocal8Bit());
	dat->setUrls(urilist); //the text/uri-list mimetype - built in Qt conversion/use
  QApplication::clipboard()->clear();
  QApplication::clipboard()->setMimeData(dat);
  //Update all the buttons to account for clipboard change
  //for(int i=0; i<DWLIST.length(); i++){ DWLIST[i]->refreshButtons(); }
}

void MainUI::CopyFiles(QStringList list){
  qDebug() << "Copy Files:" << list;
  if(list.isEmpty()){ return; } //nothing selected
  //Format the data string
  QList<QUrl> urilist; //Also assemble a URI list for cros-app compat (no copy/cut distinguishing)
  for(int i=0; i<list.length(); i++){
    urilist << QUrl::fromLocalFile(list[i]);
    list[i] = list[i].prepend("copy::::");
  }
  //Now save that data to the global clipboard
  QMimeData *dat = new QMimeData;
	dat->clear();
	dat->setData("x-special/lumina-copied-files", list.join("\n").toLocal8Bit());
	dat->setUrls(urilist); //the text/uri-list mimetype - built in Qt conversion/use
  QApplication::clipboard()->clear();
  QApplication::clipboard()->setMimeData(dat);
  //Update all the buttons to account for clipboard change
  //for(int i=0; i<DWLIST.length(); i++){ DWLIST[i]->refreshButtons(); }
}

void MainUI::PasteFiles(QString dir, QStringList raw){
  qDebug() << "Paste Files:" << dir;
  QStringList cut, copy, newcut, newcopy;
  if(raw.isEmpty()){
    //Pull info from the clipboard
    const QMimeData *dat = QApplication::clipboard()->mimeData();
    raw = QString(dat->data("x-special/lumina-copied-files")).split("\n");
  }
  if(!dir.endsWith("/")){ dir.append("/"); }
  for(int i=0; i<raw.length(); i++){
    if(raw[i].startsWith("cut::::")){ 
	cut << raw[i].section("::::",1,50);
	newcut << dir+raw[i].section("::::",1,50).section("/",-1);
    }
    else if(raw[i].startsWith("copy::::")){ 
	copy << raw[i].section("::::",1,50); 
	newcopy<< dir+raw[i].section("::::",1,50).section("/",-1);
    }
  }
  //bool errs = false;
  //Perform the copy/move operations
  //worker->pauseData = true; //pause any info requests
  if(!copy.isEmpty()){ 
    qDebug() << "Paste Copy:" << copy << "->" << newcopy;
    TRAY->StartOperation( TrayUI::COPY, copy, newcopy);
    /*FODialog dlg(this);
      if( !dlg.CopyFiles(copy, newcopy) ){ return; } //cancelled
      dlg.show();
      dlg.exec();
      errs = errs || !dlg.noerrors;*/
  }
  if(!cut.isEmpty()){
    qDebug() << "Paste Cut:" << cut << "->" << newcut;
    TRAY->StartOperation(TrayUI::MOVE, cut, newcut);
    /*FODialog dlg(this);
      if(!dlg.MoveFiles(cut, newcut) ){ return; } //cancelled
      dlg.show();
      dlg.exec();
      errs = errs || !dlg.noerrors;*/
  }
  //worker->pauseData = false; //resume info requests
  //Modify the clipboard appropriately
  if(!cut.isEmpty()){
    //Now clear the clipboard since those old file locations are now invalid
    QApplication::clipboard()->clear(); 
    if(!copy.isEmpty()){
      //There were also files copied: save those files back into the clipboard
       QMimeData *dat = new QMimeData;
	dat->clear();
	dat->setData("x-special/lumina-copied-files", raw.filter("copy::::").join("\n").toLocal8Bit());
	QApplication::clipboard()->setMimeData(dat);
    }
  }
  //Update all the buttons to account for clipboard change
  //for(int i=0; i<DWLIST.length(); i++){ DWLIST[i]->refresh(); }
}

void MainUI::FavoriteFiles(QStringList list){
  qDebug() << "Favorite Files:" << list;
  for(int i=0; i<list.length(); i++){
    LDesktopUtils::addFavorite(list[i]);
  }
  //Might want to make this a "toggle" instead of an add later on...
}

void MainUI::RenameFiles(QStringList list){
  qDebug() << "Rename Files:" << list;
  for(int i=0; i<list.length(); i++){
    QString fname = list[i];
    QString path = fname;
    fname = fname.section("/",-1); //turn this into just the file name
    path.chop(fname.length()); 	//turn this into the base directory path (has a "/" at the end)
    //Now prompt for the new filename
    bool ok = false;
    QString nname = QInputDialog::getText(this, tr("Rename File"),tr("New Name:"), QLineEdit::Normal, fname, &ok);
    if(!ok || nname.isEmpty()){ return; } //cancelled
    //Now check for a file extension and add it if necessary
    QString oext = fname.section(".",-1);
      if("."+oext == fname){ oext.clear(); } //hidden file without an extension
      else if(oext==fname){ oext.clear(); } //no extension
    QString next = nname.section(".",-1);
      if(next==nname){ next.clear(); } //no extension
    if(next.isEmpty() && !oext.isEmpty()){
      nname.append( "."+oext );
    }
    //Check if this filename already exists
    bool overwrite = QFile::exists(path+nname);
    if(overwrite){
      if(QMessageBox::Yes != QMessageBox::question(this, tr("Overwrite File?"), tr("An existing file with the same name will be replaced. Are you sure you want to proceed?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ){
        return; //cancelled
      }
    }
    //Now perform the move
    //Don't pause the background worker for a simple rename - this operation is extremely fast
    qDebug() << "Rename:" << path+fname << "->" << path+nname;
    TRAY->StartOperation(TrayUI::MOVE, QStringList() << path+fname, QStringList() << path+nname);
    /*FODialog dlg(this);
      dlg.setOverwrite(overwrite);
      dlg.MoveFiles(QStringList() << path+fname, QStringList() << path+nname);
      dlg.show();
      dlg.exec();*/
  } //end loop over list of files
}

void MainUI::RemoveFiles(QStringList list){
  if(list.isEmpty()){ return; } //nothing selected	
  qDebug() << "Remove Files:" << list;
  QStringList paths, names;
  for(int i=0; i<list.length(); i++){
     paths << list[i];
     names << list[i].section("/",-1);
   }
  
  //Verify permanent removal of file/dir
  QMessageBox dlgQ(QMessageBox::Question, tr("Verify Removal"), tr("WARNING: This will permanently delete the file(s) from the system!")+"\n"+tr("Are you sure you want to continue?"), QMessageBox::Yes | QMessageBox::No, this);
    dlgQ.setDetailedText(tr("Items to be removed:")+"\n\n"+names.join("\n"));
  dlgQ.exec();
  if(dlgQ.result() != QMessageBox::Yes){ return; } //cancelled   

  //Now remove the file/dir
  qDebug() << " - Delete: "<<paths;
  TRAY->StartOperation(TrayUI::DELETE, paths, QStringList());
  //worker->pauseData = true; //pause any info requests
  /*FODialog dlg(this);
    dlg.RemoveFiles(paths);
    dlg.show();
    dlg.exec();*/
  //worker->pauseData = false; //resume info requests
  //for(int i=0; i<DWLIST.length(); i++){ DWLIST[i]->refresh(); }
}

void MainUI::CloseBrowser(QString ID){
  //Find the tab associated with this browser first
  for(int i=0; i<tabBar->count(); i++){
    if(tabBar->tabWhatsThis(i)==ID){
      tabBar->removeTab(i);
      break;
    }
  }
  //Now remove the browser itself
  for(int i=0; i<DWLIST.length(); i++){
    if(DWLIST[i]->id()==ID){
      delete DWLIST.takeAt(i);
      break;
    }
  }
  //If the last browser was just closed, create a new one
  if(DWLIST.isEmpty()){
    OpenDirs(QStringList() << QDir::homePath());
  }
}

void MainUI::TabNameChanged(QString id, QString name){
  for(int i=0; i<tabBar->count(); i++){
    if(tabBar->tabWhatsThis(i)==id){
      tabBar->setTabText(i, name);
      return;
    }
  }
}

void MainUI::TrayJobsFinished(){
  if(waitingToClose){ this->close(); }
}

//=============
//  PROTECTED
//=============
void MainUI::closeEvent(QCloseEvent *ev){
  //See if the tray is active or not first
  if(TRAY!=0){
    if(TRAY->isVisible() && !waitingToClose){ 
      this->hide(); 
      ev->ignore(); 
      waitingToClose = true;
      return; 
    }
  }
  QMainWindow::closeEvent(ev); //continue normal close routine
}
