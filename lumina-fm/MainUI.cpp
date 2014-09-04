//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "MainUI.h"
#include "ui_MainUI.h"

MainUI::MainUI() : QMainWindow(), ui(new Ui::MainUI){
  ui->setupUi(this);
  //Be careful about the QSettings setup, it must match the lumina-desktop setup
  QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, QDir::homePath()+"/.lumina");
  settings = new QSettings( QSettings::UserScope, "LuminaDE", "lumina-fm", this);
  //initialize the non-ui widgets
  tabBar = new QTabBar(this);
    tabBar->setTabsClosable(true);
    tabBar->setMovable(true); //tabs are independant - so allow the user to sort them
    tabBar->setShape(QTabBar::RoundedNorth);
    tabBar->setFocusPolicy(Qt::NoFocus);
    ui->verticalLayout_browser->insertWidget(0,tabBar);
  currentDir = new QLineEdit(this);
    ui->toolBar->insertWidget(ui->actionBookMark, currentDir);
    currentDir->setFocusPolicy(Qt::StrongFocus);
  fsmod = new QFileSystemModel(this);
    fsmod->setRootPath("/");
    ui->tree_dir_view->setModel(fsmod);
    ui->tree_dir_view->sortByColumn(0, Qt::AscendingOrder);
    ui->tree_dir_view->setColumnWidth(0,200);
    ui->tree_dir_view->setColumnWidth(1,80); //size column should be small
    ui->tree_dir_view->setColumnWidth(2,80); //type column should be small
    ui->list_dir_view->setModel(fsmod);
  dirCompleter = new QCompleter(fsmod, this);
    dirCompleter->setModelSorting( QCompleter::CaseInsensitivelySortedModel );
    currentDir->setCompleter(dirCompleter);
  snapmod = new QFileSystemModel(this);
    ui->tree_zfs_dir->setModel(snapmod);
    ui->tree_zfs_dir->sortByColumn(0, Qt::AscendingOrder);
  iconProv = new MimeIconProvider();
    fsmod->setIconProvider(iconProv);
    snapmod->setIconProvider(iconProv);
  fswatcher = new QFileSystemWatcher(this);
    connect(fswatcher, SIGNAL(directoryChanged(QString)), this, SLOT(reloadDirectory()) );
  contextMenu = new QMenu(this);
  radio_view_details = new QRadioButton(tr("Detailed List"), this);
  radio_view_list = new QRadioButton(tr("Basic List"), this);
  radio_view_icons = new QRadioButton(tr("Icons"), this);
  detWA = new QWidgetAction(this);
    detWA->setDefaultWidget(radio_view_details);
  listWA = new QWidgetAction(this);
    listWA->setDefaultWidget(radio_view_list);
  icoWA = new QWidgetAction(this);
    icoWA->setDefaultWidget(radio_view_icons);
    ui->menuView->addAction(detWA);
    ui->menuView->addAction(listWA);
    ui->menuView->addAction(icoWA);
  //Setup the special Phonon widgets
  mediaObj = new Phonon::MediaObject(this);
    mediaObj->setTickInterval(200); //1/5 second ticks for updates
  videoDisplay = new Phonon::VideoWidget(this);
    videoDisplay->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->videoLayout->addWidget(videoDisplay);
    Phonon::createPath(mediaObj, videoDisplay);
    videoDisplay->setVisible(false);
  audioOut = new Phonon::AudioOutput(Phonon::VideoCategory, this);
    Phonon::createPath(mediaObj, audioOut);
  playerSlider = new Phonon::SeekSlider(this);
    playerSlider->setMediaObject(mediaObj);
    ui->videoControlLayout->insertWidget(4, playerSlider);
    ui->tool_player_stop->setEnabled(false); //nothing to stop yet
    ui->tool_player_pause->setVisible(false); //nothing to pause yet
    
  //Setup any specialty keyboard shortcuts
  nextTabLShort = new QShortcut( QKeySequence(tr("Shift+Left")), this);
  nextTabRShort = new QShortcut( QKeySequence(tr("Shift+Right")), this);
  closeTabShort = new QShortcut( QKeySequence(tr("Ctrl+W")), this);
  copyFilesShort = new QShortcut( QKeySequence(tr("Ctrl+C")), this);
  pasteFilesShort = new QShortcut( QKeySequence(tr("Ctrl+V")), this);
  deleteFilesShort = new QShortcut( QKeySequence(tr("Delete")), this);
  //Finish loading the interface
  setupIcons();
  setupConnections();
  loadSettings();
  RebuildBookmarksMenu();
  RebuildDeviceMenu();
  //Make sure we start on the browser page
  goToBrowserPage();
}

MainUI::~MainUI(){
	
}

void MainUI::OpenDirs(QStringList dirs){
  for(int i=0; i<dirs.length(); i++){
    //Add this directory in a new tab
    if(dirs[i].endsWith("/")){ dirs[i].chop(1); }
    qDebug() << "Open Directory:" << dirs[i];
    int index = tabBar->addTab( dirs[i].section("/",-1) );
    tabBar->setTabWhatsThis( index, dirs[i] );
    if(index==0){ setCurrentDir(dirs[i]); }//display this as the current dir
  }
  tabBar->setVisible( tabBar->count() > 1 );
}

//==========
//    PRIVATE
//==========
void MainUI::setupIcons(){
  this->setWindowIcon( LXDG::findIcon("Insight-FileManager","") );
	
  //Setup all the icons using libLumina
  ui->actionClose->setIcon( LXDG::findIcon("application-exit","") );
  ui->actionNew_Tab->setIcon( LXDG::findIcon("tab-new-background","") );
  //ui->action_Preferences->setIcon( LXDG::findIcon("configure","") );
  ui->actionUpDir->setIcon( LXDG::findIcon("go-up","") );
  ui->actionBack->setIcon( LXDG::findIcon("go-previous","") );
  ui->actionHome->setIcon( LXDG::findIcon("go-home","") );
  ui->actionBookMark->setIcon( LXDG::findIcon("bookmarks","") );
  ui->actionBackToBrowser->setIcon( LXDG::findIcon("go-previous","") );
  ui->actionManage_Bookmarks->setIcon( LXDG::findIcon("bookmarks-organize","") );
  ui->actionScan->setIcon( LXDG::findIcon("system-search","") );
	
  //Browser page
  ui->tool_addToDir->setIcon( LXDG::findIcon("folder-new","") );
  ui->tool_goToImages->setIcon( LXDG::findIcon("fileview-preview","") );
  ui->tool_goToPlayer->setIcon( LXDG::findIcon("applications-multimedia","") );
  ui->tool_goToRestore->setIcon( LXDG::findIcon("document-revert","") );
	
  //Multimedia Player page
  ui->tool_player_next->setIcon( LXDG::findIcon("media-skip-forward","") );
  ui->tool_player_prev->setIcon( LXDG::findIcon("media-skip-backward","") );
  ui->tool_player_pause->setIcon( LXDG::findIcon("media-playback-pause","") );
  ui->tool_player_play->setIcon( LXDG::findIcon("media-playback-start","") );
  ui->tool_player_stop->setIcon( LXDG::findIcon("media-playback-stop","") );
	
  //Slideshow page
  ui->tool_image_goBegin->setIcon( LXDG::findIcon("go-first-view","") );
  ui->tool_image_goEnd->setIcon( LXDG::findIcon("go-last-view","") );
  ui->tool_image_goPrev->setIcon( LXDG::findIcon("go-previous-view","") );
  ui->tool_image_goNext->setIcon( LXDG::findIcon("go-next-view","") );
  
  //ZFS Restore page
  ui->tool_zfs_nextSnap->setIcon( LXDG::findIcon("go-next-view","") );
  ui->tool_zfs_prevSnap->setIcon( LXDG::findIcon("go-previous-view","") );
  ui->tool_zfs_restoreItem->setIcon( LXDG::findIcon("document-revert","") );
}

void MainUI::setupConnections(){
  connect(QApplication::instance(), SIGNAL(focusChanged(QWidget*, QWidget*)), this, SLOT(startEditDir(QWidget*, QWidget*)) );
  connect(tabBar, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)) );
  connect(tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(tabClosed(int)) );
  connect(ui->menuBookmarks, SIGNAL(triggered(QAction*)), this, SLOT(goToBookmark(QAction*)) );
  connect(ui->menuExternal_Devices, SIGNAL(triggered(QAction*)), this, SLOT(goToDevice(QAction*)) );
  connect(currentDir, SIGNAL(returnPressed()), this, SLOT(goToDirectory()));
  connect(radio_view_details, SIGNAL(toggled(bool)), this, SLOT(viewModeChanged(bool)) );
  connect(radio_view_list, SIGNAL(toggled(bool)), this, SLOT(viewModeChanged(bool)) );
  connect(radio_view_icons, SIGNAL(toggled(bool)), this, SLOT(viewModeChanged(bool)) );
	
  //Tree Widget interaction
  connect(ui->tree_dir_view, SIGNAL(activated(const QModelIndex&)), this, SLOT(ItemRun(const QModelIndex&)) );	
  connect(ui->list_dir_view, SIGNAL(activated(const QModelIndex&)), this, SLOT(ItemRun(const QModelIndex&)) );
  connect(ui->tree_dir_view, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OpenContextMenu(const QPoint&)) );
  connect(ui->list_dir_view, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OpenContextMenu(const QPoint&)) );
  
  //Page Switching
  connect(ui->tool_goToPlayer, SIGNAL(clicked()), this, SLOT(goToMultimediaPage()) );
  connect(ui->tool_goToRestore, SIGNAL(clicked()), this, SLOT(goToRestorePage()) );
  connect(ui->tool_goToImages, SIGNAL(clicked()), this, SLOT(goToSlideshowPage()) );
  connect(ui->actionBackToBrowser, SIGNAL(triggered()), this, SLOT(goToBrowserPage()) );
	
  //Slideshow page
  connect(ui->combo_image_name, SIGNAL(currentIndexChanged(int)), this, SLOT(showNewPicture()) );
  connect(ui->tool_image_goBegin, SIGNAL(clicked()), this, SLOT(firstPicture()) );
  connect(ui->tool_image_goEnd, SIGNAL(clicked()), this, SLOT(lastPicture()) );
  connect(ui->tool_image_goNext, SIGNAL(clicked()), this, SLOT(nextPicture()) );
  connect(ui->tool_image_goPrev, SIGNAL(clicked()), this, SLOT(prevPicture()) );
	
  //ZFS Restore page
  connect(ui->slider_zfs_snapshot, SIGNAL(valueChanged(int)), this, SLOT(showSnapshot()) );
  connect(ui->tool_zfs_nextSnap, SIGNAL(clicked()), this, SLOT(nextSnapshot()) );
  connect(ui->tool_zfs_prevSnap, SIGNAL(clicked()), this, SLOT(prevSnapshot()) );
  connect(ui->tool_zfs_restoreItem, SIGNAL(clicked()), this, SLOT(restoreItems()) );
  
  //Multimedia Player page
  connect(ui->tool_player_next, SIGNAL(clicked()), this, SLOT(playerNext()));
  connect(ui->tool_player_prev, SIGNAL(clicked()), this, SLOT(playerPrevious()));
  connect(ui->tool_player_pause, SIGNAL(clicked()), this, SLOT(playerPause()));
  connect(ui->tool_player_play, SIGNAL(clicked()), this, SLOT(playerStart()));
  connect(ui->tool_player_stop, SIGNAL(clicked()), this, SLOT(playerStop()));
  connect(ui->combo_player_list, SIGNAL(currentIndexChanged(int)), this, SLOT(playerFileChanged()) );
  connect(mediaObj, SIGNAL(finished()), this, SLOT(playerFinished()) );
  connect(mediaObj, SIGNAL(tick(qint64)), this, SLOT(playerTimeChanged(qint64)) );
  connect(mediaObj, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(playerStateChanged(Phonon::State, Phonon::State)) );
  connect(mediaObj, SIGNAL(hasVideoChanged(bool)), this, SLOT(playerVideoAvailable(bool)) );
  //Special Keyboard Shortcuts
  connect(nextTabLShort, SIGNAL(activated()), this, SLOT( prevTab() ) );
  connect(nextTabRShort, SIGNAL(activated()), this, SLOT( nextTab() ) );
  connect(closeTabShort, SIGNAL(activated()), this, SLOT( tabClosed() ) );
  connect(copyFilesShort, SIGNAL(activated()), this, SLOT( CopyItems() ) );
  connect(pasteFilesShort, SIGNAL(activated()), this, SLOT( PasteItems() ) );
  connect(deleteFilesShort, SIGNAL(activated()), this, SLOT( RemoveItem() ) );
}

void MainUI::loadSettings(){
  //Note: make sure this is run after all the UI elements are created and connected to slots
  // but before the first directory gets loaded
  ui->actionView_Hidden_Files->setChecked( settings->value("showhidden", false).toBool() );
  on_actionView_Hidden_Files_triggered(); //make sure to update the models too
  QString view = settings->value("viewmode","details").toString();
  if(view=="icons"){ radio_view_icons->setChecked(true); }
  else if(view=="list"){ radio_view_list->setChecked(true); }
  else{ radio_view_details->setChecked(true); }
}

void MainUI::RebuildBookmarksMenu(){
  //Create the bookmarks menu
  ui->menuBookmarks->clear();
    ui->menuBookmarks->addAction(ui->actionManage_Bookmarks);
    ui->menuBookmarks->addSeparator();
  QStringList BM = settings->value("bookmarks", QStringList()).toStringList();
  ui->menuBookmarks->clear();
    ui->menuBookmarks->addAction(ui->actionManage_Bookmarks);
    ui->menuBookmarks->addSeparator();
  bool changed = false;
  BM.sort(); //Sort alphabetically
  for(int i=0; i<BM.length(); i++){
    if(QFile::exists(BM[i].section("::::",1,1)) ){
      QAction *act = new QAction(BM[i].section("::::",0,0),this);
        act->setWhatsThis(BM[i].section("::::",1,1));
      ui->menuBookmarks->addAction(act);
    }else{
      //Invalid directory - remove the bookmark
      BM.removeAt(i);
      i--;
      changed = true;
    }
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
        // <type> = [USB, HDRIVE, SDCARD, DVD, UNKNOWN]
	
  //Now add them to the menu appropriately
  for(int i=0; i<devs.length(); i++){
    QAction *act = new QAction(devs[i].section("::::",2,2).section("/",-1),this);
        act->setWhatsThis(devs[i].section("::::",2,2)); //full path to mountpoint
	act->setToolTip( QString(tr("Filesystem: %1")).arg( devs[i].section("::::",1,1) ) );
	//Now set the appropriate icon
	QString type = devs[i].section("::::",0,0);
	if(type=="USB"){ type = "drive-removable-media-usb"; }
	else if(type=="HDRIVE"){ type = "drive-harddisk"; }
	else if(type=="SDCARD"){ type = "media-flash-sd-mmc"; }
	else if(type=="DVD"){ type = "media-optical"; }
	else{ type = "drive-removable-media"; }
	act->setIcon( LXDG::findIcon(type, "") );
      ui->menuExternal_Devices->addAction(act);
  }
}

bool MainUI::checkUserPerms(){
  if(!isUserWritable){
    QMessageBox::warning(this, tr("Invalid Permissions"), tr("You do not have permission to edit this directory!") );
  }
  return isUserWritable;
}

QString MainUI::msToText(qint64 ms){
  QString disp;
  if(ms>3600000){
    disp.append( QString::number(ms/3600000)+":" ); 
    ms = ms%3600000; 
  }
  if(ms>60000){
    disp.append( QString::number(ms/60000)+":" );
    ms = ms%60000;
  }else{
    disp.append("0:");
  }
  if(ms>1000){
    if(ms>=10000){ disp.append( QString::number(ms/1000) ); }
    else{ disp.append( "0"+QString::number(ms/1000) ); }
  }else{
    disp.append("00");
  }
  return disp;
}

QString MainUI::getCurrentDir(){
  return currentDir->whatsThis();
}

void MainUI::setCurrentDir(QString dir){
  if(dir.isEmpty()){ return; }
  QFileInfo info(dir);
  if(!info.isDir() || !info.exists() ){ 
    qDebug() << "Invalid Directory:" << dir;
    //Try to just go up the dir tree one level
    dir.chop(dir.section("/",-1).length());
    if(!QFile::exists(dir)){
      //Still bad dir - try to return to previously shown dir
      if(currentDir->whatsThis().isEmpty()){ return; } //nothing to return to
      else{ dir = currentDir->whatsThis(); }
    }
  }
  //qDebug() << "Show Directory:" << dir;
  //qDebug() << "Dir Info:" << dir;
  //qDebug() << " - RWXLOG:" << info.isReadable() << info.isWritable() << info.isExecutable() << info.isSymLink() << info.ownerId() << info.groupId();
  isUserWritable = info.isWritable();
  if(dir.endsWith("/") && dir!="/" ){ dir.chop(1); }
  QString rawdir = dir;
  //dir.replace(QDir::homePath()+"/", "~/");
  currentDir->setText(dir);  
  //Update the directory viewer and update the line edit
    keepFocus = !currentDir->hasFocus();
    currentDir->setWhatsThis(dir); //save the full path internally
    if(radio_view_details->isChecked()){
      ui->tree_dir_view->setRootIndex(fsmod->index(dir));
    }else{
      ui->list_dir_view->setRootIndex(fsmod->index(dir));
    }
  //Setup the directory watcher here
  if( !fswatcher->directories().isEmpty() ){ fswatcher->removePaths(fswatcher->directories()); }
  fswatcher->addPath(dir);
  //Adjust the tab data
  tabBar->setTabWhatsThis( tabBar->currentIndex(), rawdir );
  if(dir!="/"){ tabBar->setTabText( tabBar->currentIndex(), dir.section("/",-1) ); }
  else{ tabBar->setTabText( tabBar->currentIndex(), dir); }
  QStringList history = tabBar->tabData(tabBar->currentIndex()).toStringList();
  if(history.isEmpty() || history.first()!=rawdir){ history.prepend(rawdir); history.removeAll(""); }
  //qDebug() << "History:" << history;
  tabBar->setTabData(tabBar->currentIndex(), history);
  //Now adjust the items as necessary
  QTimer::singleShot(0, this, SLOT(checkForMultimediaFiles()));
  QTimer::singleShot(0, this, SLOT(checkForBackups()));
  QTimer::singleShot(0, this, SLOT(checkForPictures()));
  if(isUserWritable){ ui->label_dir_stats->setText(""); }
  else{ ui->label_dir_stats->setText(tr("Limited Access Directory")); }
  ui->tool_addToDir->setVisible(isUserWritable);
  ui->actionUpDir->setEnabled(dir!="/");
  ui->actionBack->setEnabled(history.length() > 1);
  ui->actionBookMark->setEnabled( rawdir!=QDir::homePath() && settings->value("bookmarks", QStringList()).toStringList().filter("::::"+rawdir).length()<1 );
  RebuildDeviceMenu(); //keep this refreshed
}

QFileInfoList MainUI::getSelectedItems(){
  QFileInfoList out;
  if(radio_view_details->isChecked()){
    QModelIndexList items = ui->tree_dir_view->selectionModel()->selectedIndexes();
    for(int i=0; i<items.length(); i++){
      if(!out.contains(fsmod->fileInfo(items[i]))){
        out << fsmod->fileInfo(items[i]);
      }
    }
  }else{
    QModelIndexList items = ui->list_dir_view->selectionModel()->selectedIndexes();
    for(int i=0; i<items.length(); i++){
      if(!out.contains(fsmod->fileInfo(items[i]))){
        out <<  fsmod->fileInfo(items[i]);
      }
    }
  }	
  return out;
}

//==============
//    PRIVATE SLOTS
//==============
//General button check functions
void MainUI::checkForMultimediaFiles(){
  ui->tool_goToPlayer->setVisible(false);
  //Check for multimedia files not implemented yet!
  QDir dir(getCurrentDir());
  if(multiFilter.isEmpty()){
    QStringList mimes = Phonon::BackendCapabilities::availableMimeTypes();
    mimes = mimes.filter("audio/") + mimes.filter("video/");
    for(int i=0; i<mimes.length(); i++){
      multiFilter << LXDG::findFilesForMime(mimes[i]);
    }
    multiFilter.removeDuplicates();
    qDebug() << "Supported Multimedia Formats:" << multiFilter;
  }
  QStringList files = dir.entryList(multiFilter, QDir::Files | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase);
  if(!files.isEmpty() && !multiFilter.isEmpty()){
    ui->combo_player_list->clear();
    ui->combo_player_list->addItems(files);
    ui->tool_goToPlayer->setVisible(true);
  }else{
    ui->tool_goToPlayer->setVisible(false);
  }
  
}

void MainUI::checkForBackups(){
  ui->tool_goToRestore->setVisible(false);
  //Check for ZFS snapshots not implemented yet!
  snapDirs.clear(); //clear the internal variable
  if(!isUserWritable){ return; } //cannot restore files into a non-writable directory
  //Now recursively try to find snapshots of this directory
  QString cdir = getCurrentDir();
  QDir dir(cdir);
  bool found = false;
  while(dir.absolutePath()!="/" && !found){
    if(dir.exists(".zfs/snapshot")){ found = true;}
    else{ dir.cdUp(); }
    QApplication::processEvents(); //keep the UI snappy
  }
  //Now find the snapshots that contain this directory and save them
  if(found){
    QString reldir = getCurrentDir();
	  reldir.remove(dir.absolutePath());
    dir.cd(".zfs/snapshot");
    snapmod->setRootPath(dir.canonicalPath()); //set the base snapshot dir as the new root
    snapDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time);
    //Check that the current directory exists in each snapshot
    for(int i=0; i<snapDirs.length(); i++){
      if( !dir.exists(snapDirs[i]+"/"+reldir) ){
        snapDirs.removeAt(i);
	i--;
      }else{
	snapDirs[i] = QFileInfo(dir, snapDirs[i]+"/"+reldir).created().toString("yyyyMMddhhmmsszzz")+"::::"+snapDirs[i];
      }
      QApplication::processEvents(); //keep the UI snappy
    }
    snapDirs.sort();
    //Sort the snapshots by time (newest last) and format them
    for(int i=0; i<snapDirs.length(); i++){
      snapDirs[i] = dir.absolutePath()+"/"+snapDirs[i].section("::::",1,50)+"/"+reldir;
      QApplication::processEvents(); //keep the UI snappy
    }
    //qDebug() << "Found snapshots:" << snapDirs;
  }
  //Now enable the button if any snapshots available
  ui->tool_goToRestore->setVisible(!snapDirs.isEmpty());
}

void MainUI::checkForPictures(){
  ui->tool_goToImages->setVisible(false);
  QDir dir(getCurrentDir());
  if(imgFilter.isEmpty()){
    QList<QByteArray> fmt = QImageReader::supportedImageFormats();
    for(int i=0; i<fmt.length(); i++){ imgFilter << "*."+QString(fmt[i]).toLower(); }
    qDebug() << "Supported Image Formats:" << imgFilter;
  }
  QStringList pics = dir.entryList(imgFilter, QDir::Files | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase);
  if(!pics.isEmpty()){
    ui->combo_image_name->clear();
    ui->combo_image_name->addItems(pics);
    ui->tool_goToImages->setVisible(true);	  
  }
	
}

//-----------------------------------
//General page switching
//-----------------------------------
void MainUI::goToMultimediaPage(){
  //Make toolbar items disappear appropriately	
  ui->actionBackToBrowser->setVisible(true);
  ui->actionBack->setVisible(false);
  ui->actionUpDir->setVisible(false);
  ui->actionHome->setVisible(false);
  ui->actionBookMark->setVisible(false);
  currentDir->setEnabled(false);
  //Disable all the UI elements specifically for the Browser side of things
  ui->actionNew_Tab->setEnabled(false);
  ui->menuView->setEnabled(false);
  ui->menuBookmarks->setEnabled(false);
  ui->menuExternal_Devices->setEnabled(false);
  //Start the player on the first selected item
  QFileInfoList sel = getSelectedItems();
  if(!sel.isEmpty()){
    QStringList names;
    for(int i=0; i<sel.length(); i++){ names << sel[i].fileName(); }
    //start the slideshow on the first selected picture
    for(int i=0; i<ui->combo_player_list->count(); i++){
      if(names.contains( ui->combo_player_list->itemText(i) )){ ui->combo_player_list->setCurrentIndex(i); break; }
    }
  }
  //Now go to the Multimedia player
  ui->label_player_novideo->setText(tr("Click Play to Start"));
  ui->stackedWidget->setCurrentWidget(ui->page_audioPlayer);
}

void MainUI::goToRestorePage(){
  //Make toolbar items disappear appropriately
  ui->actionBackToBrowser->setVisible(true);	
  ui->actionBack->setVisible(false);
  ui->actionUpDir->setVisible(false);
  ui->actionHome->setVisible(false);
  ui->actionBookMark->setVisible(false);
  currentDir->setEnabled(false);
  //Disable all the UI elements specifically for the Browser side of things
  ui->actionNew_Tab->setEnabled(false);
  ui->menuView->setEnabled(false);
  ui->menuBookmarks->setEnabled(false);
  ui->menuExternal_Devices->setEnabled(false);
  //Load all the info into the file restore page
  ui->slider_zfs_snapshot->setRange(1,snapDirs.length());
  ui->slider_zfs_snapshot->setValue(snapDirs.length());
  //Now go to the file restore page
  showSnapshot(); //Make sure it is updated for the current directory
  ui->stackedWidget->setCurrentWidget(ui->page_zfs);	
}

void MainUI::goToSlideshowPage(){
  //Make toolbar items disappear appropriately	
  ui->actionBackToBrowser->setVisible(true);
  ui->actionBack->setVisible(false);
  ui->actionUpDir->setVisible(false);
  ui->actionHome->setVisible(false);
  ui->actionBookMark->setVisible(false);
  currentDir->setEnabled(false);
  //Disable all the UI elements specifically for the Browser side of things
  ui->actionNew_Tab->setEnabled(false);
  ui->menuView->setEnabled(false);
  ui->menuBookmarks->setEnabled(false);
  ui->menuExternal_Devices->setEnabled(false);
  QFileInfoList sel = getSelectedItems();
  if(!sel.isEmpty()){
    QStringList names;
    for(int i=0; i<sel.length(); i++){ names << sel[i].fileName(); }
    //start the slideshow on the first selected picture
    for(int i=0; i<ui->combo_image_name->count(); i++){
      if(names.contains( ui->combo_image_name->itemText(i) )){ ui->combo_image_name->setCurrentIndex(i); break; }
    }
  }
  //Now go to the Slideshow player
  ui->stackedWidget->setCurrentWidget(ui->page_image_view);
  showNewPicture(); //make sure it is up to date with the widget size
}

void MainUI::goToBrowserPage(){
  //Make toolbar items re-appear appropriately	
  ui->actionBackToBrowser->setVisible(false);
  ui->actionBack->setVisible(true);
  ui->actionUpDir->setVisible(true);
  ui->actionHome->setVisible(true);
  ui->actionBookMark->setVisible(true);
  currentDir->setEnabled(true);
  //Disable all the UI elements specifically for the Browser side of things
  ui->actionNew_Tab->setEnabled(true);
  ui->menuView->setEnabled(true);
  ui->menuBookmarks->setEnabled(true);
  ui->menuExternal_Devices->setEnabled(true);
  //Now go to the browser
  if(ui->stackedWidget->currentWidget()==ui->page_audioPlayer){ mediaObj->stop(); }
  ui->stackedWidget->setCurrentWidget(ui->page_browser);	
}
	
//---------------------
//Menu Actions
//---------------------
void MainUI::on_actionNew_Tab_triggered(){
  OpenDirs(QStringList() << QDir::homePath());
  //Now go to that tab (always last)
  tabBar->setCurrentIndex(tabBar->count()-1);
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

void MainUI::on_actionView_Hidden_Files_triggered(){
  if(ui->actionView_Hidden_Files->isChecked()){
    fsmod->setFilter( QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Hidden );
    snapmod->setFilter( QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Hidden );
  }else{
    fsmod->setFilter( QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs );
    snapmod->setFilter( QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs );
  }
  //Now save this setting for later
  settings->setValue("showhidden", ui->actionView_Hidden_Files->isChecked());
  //Re-load the view widget
  setCurrentDir(getCurrentDir());
}

void MainUI::goToBookmark(QAction *act){
  if(act==ui->actionManage_Bookmarks){
    BMMDialog dlg(this);
      dlg.loadSettings(settings);
      dlg.exec();
    RebuildBookmarksMenu();
  }else{
    setCurrentDir(act->whatsThis());
  }
}

void MainUI::goToDevice(QAction *act){
  if(act==ui->actionScan){
    RebuildDeviceMenu();
  }else{
    setCurrentDir(act->whatsThis());
  }
}

void MainUI::viewModeChanged(bool active){
  if(!active){ return; } //on every view change, all 3 radio buttons will call this function - only run this once though
  if(radio_view_details->isChecked()){
    ui->tree_dir_view->setVisible(true);
    ui->list_dir_view->setVisible(false);
	settings->setValue("viewmode","details");
  }else if(radio_view_list->isChecked()){
    ui->tree_dir_view->setVisible(false);
    ui->list_dir_view->setVisible(true);
    ui->list_dir_view->setViewMode( QListView::ListMode );
	ui->list_dir_view->setUniformItemSizes(false);
	settings->setValue("viewmode","list");
  }else{  //icons
    ui->tree_dir_view->setVisible(false);
    ui->list_dir_view->setVisible(true);
    ui->list_dir_view->setViewMode( QListView::IconMode );
	ui->list_dir_view->setUniformItemSizes(true);
	settings->setValue("viewmode","icons");
  }
  //Re-load the view widget
  setCurrentDir(getCurrentDir());
	
}

//-----------------------
//Toolbar Actions
//-----------------------
void MainUI::on_actionBack_triggered(){
  QStringList history = tabBar->tabData(tabBar->currentIndex()).toStringList();
  if(history.length() <= 1){ return; } //need the second item
  history.removeAt(0); //remove the first item (the current dir)
  tabBar->setTabData(tabBar->currentIndex(), history); //re-write the saved history
  setCurrentDir(history.first()); //go to the previous entry in the history
}

void MainUI::on_actionUpDir_triggered(){
  QString dir =  getCurrentDir();
    dir.chop( dir.section("/",-1).length() );
    setCurrentDir(dir);
}

void MainUI::on_actionHome_triggered(){
  setCurrentDir(QDir::homePath());
}

void MainUI::on_actionBookMark_triggered(){
  QString dir = getCurrentDir();
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
  ui->actionBookMark->setEnabled(false); //already bookmarked
}

//-----------------------------
//Browser Functions
//-----------------------------
void MainUI::startEditDir(QWidget *old, QWidget *now){
  if(now==currentDir){
    //The dir edit just got focus
    QString dir = currentDir->text();
      dir.replace("~/", QDir::homePath()+"/");
      currentDir->setText(dir);
      //Try to move to the end
      currentDir->selectAll();
  }else if(old==currentDir){
    QString dir = currentDir->text();
      setCurrentDir(dir);
  }
}

void MainUI::goToDirectory(){
  QString dir = currentDir->text();
  dir.replace("~/",QDir::homePath()+"/");
  setCurrentDir(dir);
}

void MainUI::reloadDirectory(){
  setCurrentDir( getCurrentDir() );
}

void MainUI::on_tool_addToDir_clicked(){
  bool ok = false;
  QString newdir = QInputDialog::getText(this, tr("New Directory"), tr("Name:"), QLineEdit::Normal, "", \
		&ok, 0, Qt::ImhFormattedNumbersOnly | Qt::ImhUppercaseOnly | Qt::ImhLowercaseOnly);
  if(!ok || newdir.isEmpty()){ return; }
  QString full = getCurrentDir();
  if(!full.endsWith("/")){ full.append("/"); }
  QDir dir(full); //open the current dir
  full.append(newdir); //append the new name to the current dir
  //Verify that the new dir does not already exist
  if(dir.exists(full)){
    QMessageBox::warning(this, tr("Invalid Name"), tr("A file or directory with that name already exists! Please pick a different name."));
    QTimer::singleShot(0,this, SLOT(on_tool_addToDir_clicked()) ); //repeat this function
  }else{
    if(!dir.mkdir(newdir) ){
      QMessageBox::warning(this, tr("Error Creating Directory"), tr("The directory could not be created. Please ensure that you have the proper permissions to modify the current directory."));
    }
  }
}

void MainUI::tabChanged(int tab){
  //Load the directory contained in the new tab
  qDebug() << "Change to Tab:" << tab << tabBar->tabText(tab);
  QString dir = tabBar->tabWhatsThis(tab); //get the full directory
  setCurrentDir(dir); //display this as the current dir
}

void MainUI::tabClosed(int tab){
  if(tabBar->count()==1){ return; } //Can't close the only tab
  if(tab < 0){ tab = tabBar->currentIndex(); }
  //Remove the tab (will automatically move to a different one);
  qDebug() << "Closing tab:" << tab << tabBar->tabText(tab);
  tabBar->removeTab(tab);
  tabBar->setVisible( tabBar->count() > 1 );
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

void MainUI::ItemRun(const QModelIndex &index){
  //This is called when the user double clicks a file/directory
  QString val = fsmod->filePath(index).section("/",-1);
  QString itemPath = getCurrentDir();
  if( !itemPath.endsWith("/")){ itemPath.append("/"); }
  itemPath.append(val);
  if(fsmod->isDir(index)){
    setCurrentDir( itemPath );
  }else{
    //Must be a file, try to run it
    QProcess::startDetached("lumina-open "+itemPath);
  }
}

void MainUI::ItemRun(QTreeWidgetItem *item){
  //This is called when the user double clicks a file/directory
  QString val = item->text(0);
  QString itemPath = getCurrentDir();
  if( !itemPath.endsWith("/")){ itemPath.append("/"); }
  itemPath.append(val);
  if(QFileInfo(itemPath).isDir()){
    setCurrentDir( itemPath );
  }else{
    //Must be a file, try to run it
    QProcess::startDetached("lumina-open "+itemPath);
  }
}

void MainUI::ItemRun(QListWidgetItem *item){
  //This is called when the user double clicks a file/directory
  QString val = item->text();
  QString itemPath = getCurrentDir();
  if( !itemPath.endsWith("/")){ itemPath.append("/"); }
  itemPath.append(val);
  if(QFileInfo(itemPath).isDir()){
    setCurrentDir( itemPath );
  }else{
    //Must be a file, try to run it
    QProcess::startDetached("lumina-open "+itemPath);
  }
}

void MainUI::OpenContextMenu(const QPoint &pt){
  QFileInfo info; 
  if(radio_view_details->isChecked()){ 
    QModelIndex it = ui->tree_dir_view->indexAt(pt);
    if(!it.isValid()){ CItem.clear();}
    else{ 
      info = fsmod->fileInfo(it); 
      CItem = info.absoluteFilePath(); 
    }
  }else{ 
    QModelIndex it = ui->list_dir_view->indexAt(pt);
    if(!it.isValid()){ CItem.clear();}
    else{ 
      info = fsmod->fileInfo(it); 
      CItem = info.absoluteFilePath(); 
    }
  }
  //Create the context menu
  contextMenu->clear();
  if(!CItem.isEmpty()){  
    if(info.isDir()){
      contextMenu->addAction(LXDG::findIcon("tab-new-background",""), tr("Open in new tab"), this, SLOT(OpenDir()) );
    }else{
      contextMenu->addAction(LXDG::findIcon("quickopen-file",""), tr("Open"), this, SLOT(OpenItem()) );
      contextMenu->addAction(tr("Open With..."), this, SLOT(OpenItemWith()) );
    }
    contextMenu->addAction(LXDG::findIcon("edit-rename",""), tr("Rename"), this, SLOT(RenameItem()) )->setEnabled(info.isWritable());
    contextMenu->addSeparator();
  }
  bool hasSelection = !getSelectedItems().isEmpty();
  //Now add the general selection options
  contextMenu->addAction(LXDG::findIcon("edit-cut",""), tr("Cut Selection"), this, SLOT(CutItems()) )->setEnabled(info.isWritable() && hasSelection);
  contextMenu->addAction(LXDG::findIcon("edit-copy",""), tr("Copy Selection"), this, SLOT(CopyItems()) )->setEnabled(hasSelection);
  contextMenu->addAction(LXDG::findIcon("edit-paste",""), tr("Paste"), this, SLOT(PasteItems()) )->setEnabled(QApplication::clipboard()->mimeData()->hasFormat("x-special/lumina-copied-files") && isUserWritable);
  contextMenu->addSeparator();
  contextMenu->addAction(LXDG::findIcon("edit-delete",""), tr("Delete Selection"), this, SLOT(RemoveItem()) )->setEnabled(info.isWritable()&&hasSelection);
  //Now show the menu
  if(radio_view_details->isChecked()){
    contextMenu->popup(ui->tree_dir_view->mapToGlobal(pt));
  }else{
    contextMenu->popup(ui->list_dir_view->mapToGlobal(pt));
  }
}

//-------------------------------
//Slideshow Functions
//-------------------------------
void MainUI::showNewPicture(){
  if( !ui->label_image->isVisible() ){ return; } //don't update if not visible - can cause strange resizing issues
  QString file = getCurrentDir();
  if(!file.endsWith("/")){ file.append("/"); }
  file.append(ui->combo_image_name->currentText());
  //qDebug() << "Show Image:" << file;
  QPixmap pix(file);
  if(pix.size().width() > ui->label_image->contentsRect().width() || pix.size().height() > ui->label_image->contentsRect().height()){ 
    pix = pix.scaled(ui->label_image->contentsRect().size(), Qt::KeepAspectRatio, Qt::SmoothTransformation); 
  }
  ui->label_image->setPixmap(pix);
  //Now set/load the buttons
  ui->tool_image_goBegin->setEnabled(ui->combo_image_name->currentIndex()>0);
  ui->tool_image_goPrev->setEnabled(ui->combo_image_name->currentIndex()>0);
  ui->tool_image_goEnd->setEnabled(ui->combo_image_name->currentIndex()<(ui->combo_image_name->count()-1));
  ui->tool_image_goNext->setEnabled(ui->combo_image_name->currentIndex()<(ui->combo_image_name->count()-1));
  ui->label_image_index->setText( QString::number(ui->combo_image_name->currentIndex()+1)+"/"+QString::number(ui->combo_image_name->count()) );
}

void MainUI::firstPicture(){
  ui->combo_image_name->setCurrentIndex(0);
}

void MainUI::prevPicture(){
  ui->combo_image_name->setCurrentIndex( ui->combo_image_name->currentIndex()-1 );
}

void MainUI::nextPicture(){
  ui->combo_image_name->setCurrentIndex( ui->combo_image_name->currentIndex()+1 );
}

void MainUI::lastPicture(){
  ui->combo_image_name->setCurrentIndex( ui->combo_image_name->count()-1 );
}

//----------------------------------
//ZFS Restore Functions
//----------------------------------
void MainUI::snapshotLoaded(){
  ui->tree_zfs_dir->resizeColumnToContents(0);
}

void MainUI::showSnapshot(){
  ui->tool_zfs_prevSnap->setEnabled(ui->slider_zfs_snapshot->value()!=1);
  ui->tool_zfs_nextSnap->setEnabled(ui->slider_zfs_snapshot->value()!=ui->slider_zfs_snapshot->maximum());
  ui->label_zfs_snap->setText( snapDirs[ui->slider_zfs_snapshot->value()-1].section("/.zfs/snapshot/",1,1).section("/",0,0) );
  //Load the dir contents
  ui->tree_zfs_dir->setRootIndex(snapmod->index(snapDirs[ui->slider_zfs_snapshot->value()-1]));
}

void MainUI::prevSnapshot(){
  ui->slider_zfs_snapshot->setValue(ui->slider_zfs_snapshot->value()-1);
}

void MainUI::nextSnapshot(){
  ui->slider_zfs_snapshot->setValue(ui->slider_zfs_snapshot->value()+1);
}

void MainUI::restoreItems(){
   //Get the selected items
   QStringList sel; //= getSelectedItems();
   QModelIndexList items = ui->tree_zfs_dir->selectionModel()->selectedIndexes();
   for(int i=0; i<items.length(); i++){
     sel << snapmod->filePath(items[i]).section("/",-1);
   }
   sel.removeDuplicates();
   if(sel.isEmpty()){ return; } //nothing selected
   if(!checkUserPerms()){ return; }
   //Get the directories
   QString snapdir = snapDirs[ui->slider_zfs_snapshot->value()-1];
   QString basedir = getCurrentDir();
   if(!basedir.endsWith("/")){ basedir.append("/"); }
   if(!snapdir.endsWith("/")){ snapdir.append("/"); }
   //Fill out the lists appropriately
   QStringList resto;
   qDebug() << "Items Selected:" << sel;
   for(int i=0; i<sel.length(); i++){
     resto << basedir+sel[i];
     sel[i] = snapdir+sel[i];
   }
   qDebug() << "Restore Items:" << sel << "\n" << resto;
   //Restore the items
   FODialog dlg(this);
	dlg.setOverwrite(ui->check_zfs_overwrite->isChecked());
	dlg.RestoreFiles(sel, resto);
   dlg.exec();
   if(dlg.noerrors){
     QMessageBox::information(this, tr("Success"), tr("Successfully restored selection") );
   }
}

//----------------------------
// Multimedia Player
//----------------------------
void MainUI::playerStart(){
  if(ui->stackedWidget->currentWidget()!=ui->page_audioPlayer){ return; } //don't play if not in the player
  
  if(mediaObj->state()==Phonon::PausedState){
    mediaObj->play();
  }else{ // if(mediaObj->state()==Phonon::StoppedState || mediaObj->state()==Phonon::ErrorState || (playerFile->fileName().section("/",-1) != ui->combo_player_list->currentText()) || playerFile->isOpen() ){
    mediaObj->stop();
    //Get the selected file path
    QString filePath = getCurrentDir();
    if(!filePath.endsWith("/")){ filePath.append("/"); }
    filePath.append( ui->combo_player_list->currentText() );
    //if(playerFile->isOpen()){ playerFile->close(); }
    //playerFile->setFileName(filePath);
    //if(playerFile->open(QIODevice::ReadOnly)){
      mediaObj->setCurrentSource( QUrl(filePath) );
      playerSlider->setMediaObject(mediaObj);
      mediaObj->play();
    //}
  }
}

void MainUI::playerStop(){
  mediaObj->stop();
}

void MainUI::playerPause(){
  mediaObj->pause();
}

void MainUI::playerNext(){
  ui->combo_player_list->setCurrentIndex( ui->combo_player_list->currentIndex()+1);
}

void MainUI::playerPrevious(){
  ui->combo_player_list->setCurrentIndex( ui->combo_player_list->currentIndex()-1);	
}

void MainUI::playerFinished(){
  //playerFile->close();
  if(ui->combo_player_list->currentIndex()<(ui->combo_player_list->count()-1) && ui->check_player_gotonext->isChecked()){
    ui->combo_player_list->setCurrentIndex( ui->combo_player_list->currentIndex()+1 );
  }else{
    ui->label_player_novideo->setText(tr("Finished"));
  }
}

void MainUI::playerStateChanged(Phonon::State newstate, Phonon::State oldstate){
  //This function keeps track of updating the visuals of the player
  bool running = false;
  bool showVideo = false;
  QString msg;
  switch(newstate){
    case Phonon::LoadingState:
	running=true;
	ui->label_player_novideo->setText(tr("Loading File..."));
        break;
    case Phonon::PlayingState:
	running=true;
	showVideo = mediaObj->hasVideo();
	msg = mediaObj->metaData(Phonon::TitleMetaData).join(" ");
	if(msg.simplified().isEmpty()){ msg = ui->combo_player_list->currentText(); }
	ui->label_player_novideo->setText(tr("Playing:")+"\n"+msg);
	break;
    case Phonon::BufferingState:
	running=true;
	showVideo=true; //don't blank the screen
	break;
    case Phonon::PausedState:
	showVideo=videoDisplay->isVisible(); //don't change the screen
	break;
    case Phonon::StoppedState:
	if(oldstate==Phonon::LoadingState){ mediaObj->play(); }
	else{ ui->label_player_novideo->setText(tr("Stopped")); }
        break;
    case Phonon::ErrorState:
	ui->label_player_novideo->setText(tr("Error Playing File")+"\n("+mediaObj->errorString()+")");
        break;
  }
  ui->tool_player_play->setVisible(!running);
  ui->tool_player_pause->setVisible(running);
  ui->tool_player_stop->setEnabled(running);
  ui->label_player_novideo->setVisible(!showVideo);
  videoDisplay->setVisible(showVideo);
}

void MainUI::playerVideoAvailable(bool showVideo){
  ui->label_player_novideo->setVisible(!showVideo);
  videoDisplay->setVisible(showVideo);	
}

void MainUI::playerTimeChanged(qint64 ctime){
  if(playerTTime=="0:00" || playerTTime.isEmpty()){ playerTTime = msToText(mediaObj->totalTime()); } //only calculate as necessary
  //qDebug() << "Time:" << msToText(ctime) << playerTTime << mediaObj->isSeekable() << mediaObj->hasVideo();
  ui->label_player_runstats->setText( msToText(ctime)+"/"+playerTTime );
}

void MainUI::playerFileChanged(){
  ui->tool_player_next->setEnabled( ui->combo_player_list->count() > (ui->combo_player_list->currentIndex()+1) );	
  ui->tool_player_prev->setEnabled( (ui->combo_player_list->currentIndex()-1) >= 0 );
  if(ui->stackedWidget->currentWidget()!=ui->page_audioPlayer){ return; } //don't play if not in the player
  //If one is playing, so ahead and start playing the new selection
  if(ui->check_player_gotonext->isChecked() ){
    QTimer::singleShot(0,this,SLOT(playerStart()));
  }
}

//----------------------------------
// Context Menu Actions
//----------------------------------
void MainUI::OpenItem(){
  if(CItem.isEmpty()){ return; }
  qDebug() << "Opening File:" << CItem;
  QProcess::startDetached("lumina-open \""+CItem+"\"");
}

void MainUI::OpenItemWith(){
  if(CItem.isEmpty()){ return; }
  qDebug() << "Opening File:" << CItem;
  QProcess::startDetached("lumina-open -select \""+CItem+"\"");	
}

void MainUI::OpenDir(){
  if(CItem.isEmpty()){ return; }
  OpenDirs(QStringList() << CItem);		
}

void MainUI::RemoveItem(){
  //Only let this run if viewing the browser page
  if(ui->stackedWidget->currentWidget()!=ui->page_browser){ return; }
   if(!checkUserPerms()){ return; }
   //Get the selected items
   QStringList paths, names;
   QFileInfoList sel = getSelectedItems();
   for(int i=0; i<sel.length(); i++){
     paths << sel[i].absoluteFilePath();
     names << sel[i].fileName();
   }
   if(sel.isEmpty()){ return; } //nothing selected
  //Verify permanent removal of file/dir
  if(QMessageBox::Yes != QMessageBox::question(this, tr("Verify Removal"), tr("WARNING: This will permanently delete the file(s) from the system!")+"\n"+tr("Are you sure you want to continue?")+"\n\n"+names.join("\n"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ){
    return; //cancelled
  }
  //Now remove the file/dir
  qDebug() << "Delete: "<<paths;
  FODialog dlg(this);
    dlg.RemoveFiles(paths);
    dlg.exec();
}

void MainUI::RenameItem(){
  //Only let this run if viewing the browser page
  if(ui->stackedWidget->currentWidget()!=ui->page_browser){ return; }
  if(CItem.isEmpty()){ return; }
  if(!checkUserPerms()){ return; }
  QString fname = CItem;
  QString path = fname;
    fname = fname.section("/",-1); //turn this into just the file name
    path.chop(fname.length()); 	//turn this into the base directory path (has a "/" at the end)
  //Now prompt for the new filename
  bool ok = false;
  QString nname = QInputDialog::getText(this, tr("Rename File"),tr("New Name:"), QLineEdit::Normal, fname, &ok);
  if(!ok || nname.isEmpty()){ return; } //cancelled
  //Now check for a file extension and add it if necessary
  QString oext = fname.section(".",-1);
    if(oext==fname){ oext.clear(); } //no extension
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
  qDebug() << "Rename:" << path+fname << "->" << path+nname;
  FODialog dlg(this);
    dlg.setOverwrite(overwrite);
    dlg.MoveFiles(QStringList() << path+fname, QStringList() << path+nname);
    dlg.exec();
}

void MainUI::CutItems(){
  //Only let this run if viewing the browser page
  if(ui->stackedWidget->currentWidget()!=ui->page_browser){ return; }
  if(!checkUserPerms()){ return; }
  //Get all the selected Items 
  QFileInfoList sel = getSelectedItems();
  QStringList items;
  if(sel.isEmpty()){ return; } //nothing selected
  /*for(int i=0; i<sel.length(); i++){
    sel[i].prepend(base);
  }
  qDebug() << "Cut Items:" << sel;*/
  //Format the data string
  for(int i=0; i<sel.length(); i++){
    items << "cut::::"+sel[i].absoluteFilePath();
    //sel[i] = sel[i].prepend("cut::::");
  }
  
  //Now save that data to the global clipboard
  QMimeData *dat = new QMimeData;
	dat->clear();
	dat->setData("x-special/lumina-copied-files", items.join("\n").toLocal8Bit());
  QApplication::clipboard()->clear();
  QApplication::clipboard()->setMimeData(dat);
}

void MainUI::CopyItems(){
  //Only let this run if viewing the browser page
  if(ui->stackedWidget->currentWidget()!=ui->page_browser){ return; }
  //Get all the selected Items 
  QFileInfoList sel = getSelectedItems();
  QStringList items;
  if(sel.isEmpty()){ return; } //nothing selected
  /*QString base = getCurrentDir();
  if(!base.endsWith("/")){ base.append("/"); }
  for(int i=0; i<sel.length(); i++){
    sel[i].prepend(base);
  }
  qDebug() << "Copy Items:" << sel;*/
  //Format the data string
  for(int i=0; i<sel.length(); i++){
    items << "copy::::"+sel[i].absoluteFilePath();
    //sel[i] = sel[i].prepend("copy::::");
  }
  //Now save that data to the global clipboard
  QMimeData *dat = new QMimeData;
	dat->clear();
	dat->setData("x-special/lumina-copied-files", items.join("\n").toLocal8Bit());
  QApplication::clipboard()->clear();
  QApplication::clipboard()->setMimeData(dat);	
}

void MainUI::PasteItems(){
  //Only let this run if viewing the browser page
  if(ui->stackedWidget->currentWidget()!=ui->page_browser){ return; }
  const QMimeData *dat = QApplication::clipboard()->mimeData();
  if(!dat->hasFormat("x-special/lumina-copied-files")){ return; } //nothing to paste
  if(!checkUserPerms()){ return; }
  QStringList cut, copy, newcut, newcopy;
  QStringList raw = QString(dat->data("x-special/lumina-copied-files")).split("\n");
  QString base = getCurrentDir();
  if(!base.endsWith("/")){ base.append("/"); }
  for(int i=0; i<raw.length(); i++){
    if(raw[i].startsWith("cut::::")){ 
	cut << raw[i].section("::::",1,50);
	newcut << base+raw[i].section("::::",1,50).section("/",-1);
    }
    else if(raw[i].startsWith("copy::::")){ 
	copy << raw[i].section("::::",1,50); 
	newcopy<< base+raw[i].section("::::",1,50).section("/",-1);
    }
  }
  bool errs = false;
  //Perform the copy/move operations
  if(!copy.isEmpty()){ 
    qDebug() << "Paste Copy:" << copy << "->" << newcopy;
    FODialog dlg(this);
      dlg.CopyFiles(copy, newcopy);
      dlg.exec();
      errs = errs || !dlg.noerrors;
  }
  if(!cut.isEmpty()){
    qDebug() << "Paste Cut:" << cut << "->" << newcut;
    FODialog dlg(this);
      dlg.MoveFiles(cut, newcut);
      dlg.exec();
      errs = errs || !dlg.noerrors;
  }
  //Modify the clipboard appropriately
  if(!errs && !cut.isEmpty()){
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
	
}
