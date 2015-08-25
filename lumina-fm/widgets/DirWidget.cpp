//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "DirWidget.h"
#include "ui_DirWidget.h"

#include <QMessageBox>
#include <QCursor>
#include <QClipboard>
#include <QMimeData>

#include <LuminaOS.h>
#include <LuminaXDG.h>
#include <LuminaUtils.h>

DirWidget::DirWidget(QString objID, QWidget *parent) : QWidget(parent), ui(new Ui::DirWidget){
  ui->setupUi(this); //load the designer file
  ID = objID;
  //Assemble the toolbar for the widget
  toolbar = new QToolBar(this);
    toolbar->setContextMenuPolicy(Qt::CustomContextMenu);
    toolbar->setFloatable(false);
    toolbar->setMovable(false);
    toolbar->setOrientation(Qt::Horizontal);
    toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    //toolbar->setIconSize(QSize(32,32));
  ui->toolbar_layout->addWidget(toolbar);
  // - Add the buttons to the toolbar
  toolbar->addAction(ui->actionBack);
  toolbar->addAction(ui->actionUp);
  toolbar->addAction(ui->actionHome);
  line_dir = new QLineEdit(this);
    toolbar->addWidget(line_dir);
  toolbar->addAction(ui->actionStopLoad);
  toolbar->addAction(ui->actionClose_Browser);
  //Create the keyboard shortcuts
  copyFilesShort = new QShortcut( QKeySequence(tr("Ctrl+C")), this);
  pasteFilesShort = new QShortcut( QKeySequence(tr("Ctrl+V")), this);
  cutFilesShort = new QShortcut( QKeySequence(tr("Ctrl+X")), this);
  deleteFilesShort = new QShortcut( QKeySequence(tr("Delete")), this);
  refreshShort = new QShortcut( QKeySequence(tr("F5")), this);
  //Create the filesystem watcher
  watcher = new QFileSystemWatcher(this);
  //Now update the rest of the UI
  canmodify = false; //initial value
  contextMenu = new QMenu(this);
  setShowDetails(true);
  setShowThumbnails(true);
  UpdateIcons();
  UpdateText();	
  setupConnections();
}

DirWidget::~DirWidget(){
	
}

void DirWidget::ChangeDir(QString dirpath){
  emit LoadDirectory(ID, dirpath);
}

void DirWidget::setDirCompleter(QCompleter *comp){
  line_dir->setCompleter(comp);
}

QString DirWidget::id(){
  return ID;
}

QString DirWidget::currentDir(){
  return CDIR;	
}

void DirWidget::setShowDetails(bool show){
  showDetails = show;
  ui->listWidget->setVisible(!showDetails);
  ui->treeWidget->setVisible(showDetails);
  this->refresh();
}

void DirWidget::setShowSidebar(bool show){
  ui->group_actions->setVisible(show);
}

void DirWidget::setShowThumbnails(bool show){
  showThumbs = show;
  this->refresh();
}

void DirWidget::setDetails(QList<DETAILTYPES> list){
  listDetails = list;
  //Need to re-create the header item as well
    QTreeWidgetItem *it = new QTreeWidgetItem();
    int nmcol = -1; int typecol = -1;
    for(int t=0; t<listDetails.length(); t++){
      switch(listDetails[t]){
	  case NAME:
	    it->setText(t,tr("Name"));
	    nmcol = t;
	    break;
	  case SIZE:
	    it->setText(t,tr("Size"));
	    break;
	  case TYPE:
	    it->setText(t, tr("Type"));
	    typecol = t;
	    break;
	  case DATEMOD:
	    it->setText(t, tr("Date Modified") );
	    break;
	  case DATECREATE:
	    it->setText(t, tr("Date Created") );
	    break;
	}
    }	  
    ui->treeWidget->setHeaderItem(it);
    //Now reset the sorting (alphabetically, dirs first)
    if(nmcol>=0){ ui->treeWidget->sortItems(nmcol, Qt::AscendingOrder); } // sort by name
    if(typecol>=0){ ui->treeWidget->sortItems(typecol, Qt::AscendingOrder); } //sort by type first
    
  if(CDIR.isEmpty() || !showDetails){ return; } //don't need to reload dir if details are not visible
  this->refresh();
}

void DirWidget::setThumbnailSize(int px){
  bool larger = ui->listWidget->iconSize().height() < px;
  ui->listWidget->setIconSize(QSize(px,px));
  ui->treeWidget->setIconSize(QSize(px,px));
  if(CDIR.isEmpty() || !larger ){ return; } //don't need to reload icons unless the new size is larger
  this->refresh();
}

void DirWidget::setShowCloseButton(bool show){
  ui->actionClose_Browser->setVisible(show);
}

// ================
//    PUBLIC SLOTS
// ================
void DirWidget::LoadDir(QString dir, QList<LFileInfo> list){
  if(dir.isEmpty()){ return; } //nothing to do
  qDebug() << "Load Dir:" << dir;
  QString lastdir = CDIR; //for some checks later
  QString lastbasedir = normalbasedir;
  CDIR = dir;
  if(CDIR.endsWith("/")){ CDIR.chop(1); }
  CLIST = list; //save for laterr
  canmodify = QFileInfo(CDIR).isWritable();
  //Hide the extra buttons for a moment
  ui->tool_goToPlayer->setVisible(false);
  ui->tool_goToImages->setVisible(false);
  //Determine if this is an internal ZFS snapshot
  bool loadsnaps = false;
  if( dir.contains(ZSNAPDIR) ){
    //This is a zfs snapshot - only update the saved paths necessary to rotate between snapshots/system
    snaprelpath = dir.section(ZSNAPDIR,1,1000).section("/",1,1000); //the relative path inside the snapshot
    if(snaprelpath.endsWith("/")){ snaprelpath.chop(1); }
    normalbasedir = dir.section(ZSNAPDIR,0,0)+"/"+snaprelpath; //Update the new base directory
    if(normalbasedir.endsWith("/")){ normalbasedir.chop(1); }
    line_dir->setText(normalbasedir);
    //See if this was a manual move to the directory, or an internal move
    QString tmp = dir.section(ZSNAPDIR,0,0);
    if(tmp != snapbasedir.section(ZSNAPDIR,0,0)){
      loadsnaps = true; //different snapshot loaded - need to update internally
    }
  }else{
    //This is a normal directory - prompt for snapshot information
    line_dir->setText(CDIR);
    normalbasedir = CDIR;
    snapbasedir.clear();
    loadsnaps = true;
  }
  if(loadsnaps){
    //kick this off while still loading the dir contents
    ui->group_snaps->setEnabled(false); //to prevent the snap updates to be automatically used
    ui->group_snaps->setVisible(false);
    ui->slider_snap->setRange(1,1);
    emit findSnaps(ID, normalbasedir); 
  }
  //Now update the history for this browser
  if(!history.isEmpty() && history.last() == normalbasedir && lastbasedir!=normalbasedir ){
    //We went back one - remove this from the history
    history.takeLast();
    ui->actionBack->setEnabled(!history.isEmpty());
  }else if(lastbasedir!=normalbasedir){ //not a refresh or internal snapshot change
    history << normalbasedir;
    ui->actionBack->setEnabled(history.length()>1);
  }
  //Clear the current watcher
  if(!watcher->directories().isEmpty()){ watcher->removePaths(watcher->directories()); }
  if(!watcher->files().isEmpty()){ watcher->removePaths(watcher->files()); }
  watcher->addPath(CDIR);
  ui->actionStopLoad->setVisible(true);
  stopload = false;
  //Clear the display widget
  if(showDetails){ ui->treeWidget->clear(); }
  else{ ui->listWidget->clear(); }
  //Now fill the display widget
  bool hasimages, hasmultimedia;
  hasimages = hasmultimedia = false;
  for(int i=0; i<list.length(); i++){
    if(stopload){ ui->actionStopLoad->setVisible(false); return; } //stop right now
    hasimages = hasimages || list[i].isImage();
    hasmultimedia = hasmultimedia || list[i].isAVFile();
    //watcher->addPath(list[i].absoluteFilePath());
    if(showDetails){
      //Now create all the individual items for the details tree
      QTreeWidgetItem *it = new QTreeWidgetItem();
	it->setWhatsThis(0, list[i].fileName());
      for(int t=0; t<listDetails.length(); t++){
        switch(listDetails[t]){
	  case NAME:
	    it->setText(t,list[i].fileName());
	    it->setStatusTip(t, list[i].fileName());
	    if(list[i].isImage()){
	      if(showThumbs){ it->setIcon(t, QIcon( QPixmap(list[i].absoluteFilePath()).scaledToHeight(ui->treeWidget->iconSize().height()) ) ); }
	      else{ it->setIcon(t, LXDG::findIcon(list[i].iconfile(),"image-x-generic") ); }
	    }else{
	      it->setIcon(t, LXDG::findIcon(list[i].iconfile(),"unknown") );
	    }
	    break;
	  case SIZE:
	    if(!list[i].isDir()){
	      it->setText(t, LUtils::BytesToDisplaySize(list[i].size()) );
	    }
	    break;
	  case TYPE:
	    it->setText(t, list[i].mimetype());
	    break;
	  case DATEMOD:
	    it->setText(t, list[i].lastModified().toString(Qt::DefaultLocaleShortDate) );
	    break;
	  case DATECREATE:
	    it->setText(t, list[i].created().toString(Qt::DefaultLocaleShortDate) );
	    break;
	}
      }
      ui->treeWidget->addTopLevelItem(it);
      if(lastdir == CDIR+"/"+it->whatsThis(0)){ 
	ui->treeWidget->setCurrentItem(it);
	ui->treeWidget->scrollToItem(it);
      }
    }else{
	//Create all the individual items for the basic list
	QListWidgetItem *it = new QListWidgetItem();
	    it->setWhatsThis(list[i].fileName());
	    it->setText(list[i].fileName());
	    it->setStatusTip(list[i].fileName());
	    if(list[i].isImage()){
	      if(showThumbs){ it->setIcon(QIcon( QPixmap(list[i].absoluteFilePath()).scaledToHeight(ui->treeWidget->iconSize().height()) ) ); }
	      else{ it->setIcon(LXDG::findIcon(list[i].iconfile(),"image-x-generic") ); }
	    }else{
	      it->setIcon(LXDG::findIcon(list[i].iconfile(),"unknown") );
	    }
	ui->listWidget->addItem(it);
	if(lastdir == CDIR+"/"+it->whatsThis()){ 
	  ui->listWidget->setCurrentItem(it);
	  ui->listWidget->scrollToItem(it);
	}
    }
    QApplication::processEvents(); //keep the UI snappy while loading a directory
  }
  ui->actionStopLoad->setVisible(false);
  //Another check to ensure the current item is visible
  if(showDetails){
    if(ui->treeWidget->currentItem()!=0){ ui->treeWidget->scrollToItem(ui->treeWidget->currentItem()); }
    for(int t=0; t<ui->treeWidget->columnCount(); t++){ui->treeWidget->resizeColumnToContents(t); }
  }else{
    if(ui->listWidget->currentItem()!=0){ ui->listWidget->scrollToItem(ui->listWidget->currentItem()); }
  }
  //Now Re-enable buttons as necessary
  ui->tool_goToPlayer->setVisible(hasmultimedia);
  ui->tool_goToImages->setVisible(hasimages);
  if(canmodify){ ui->label_status->setText(""); }
  else{ ui->label_status->setText(tr("Restricted Access")); }
}

void DirWidget::LoadSnaps(QString basedir, QStringList snaps){
  //Save these value internally for use later
  snapbasedir = basedir;
  snapshots = snaps;
  //Now update the UI as necessary
  
  ui->slider_snap->setRange(0, snaps.length());
  if(CDIR.contains(ZSNAPDIR)){
    //The user was already within a snapshot - figure out which one and set the slider appropriately
    int index = snaps.indexOf( CDIR.section(ZSNAPDIR,1,1).section("/",0,0) );
    if(index < 0){ index = snaps.length()+1; } //unknown - load the system (should never happen)
    ui->slider_snap->setValue(index);
  }else{
    ui->slider_snap->setValue(snaps.length()+1); //last item (normal system)
  }
  QApplication::processEvents(); //let the slider changed signal get thrown away before we re-enable the widget
  ui->group_snaps->setEnabled(!snaps.isEmpty());
  ui->group_snaps->setVisible(!snaps.isEmpty());
  ui->tool_snap_newer->setEnabled(ui->slider_snap->value() < ui->slider_snap->maximum());
  ui->tool_snap_older->setEnabled(ui->slider_snap->value() > ui->slider_snap->minimum());
	
}

void DirWidget::refresh(){
  if(!CDIR.isEmpty() && ~ID.isEmpty()){ 
    stopload = true; //just in case it is still loading
    emit LoadDirectory(ID, CDIR); 
  }
}

void DirWidget::refreshButtons(){
  SelectionChanged();
}

//Theme change functions
void DirWidget::UpdateIcons(){
  //ui->tool_addNewFile->setIcon( LXDG::findIcon("document-new",""));
  //ui->tool_addToDir->setIcon( LXDG::findIcon("folder-new","") );

  //Snapshot buttons
  ui->tool_snap_newer->setIcon(LXDG::findIcon("go-next-view","") );
  ui->tool_snap_older->setIcon(LXDG::findIcon("go-previous-view","") );
  //Bottom-Action Buttons
  ui->tool_goToImages->setIcon( LXDG::findIcon("fileview-preview","") );
  ui->tool_goToPlayer->setIcon( LXDG::findIcon("applications-multimedia","") );	
  //Side-Action Buttons
  ui->tool_act_run->setIcon( LXDG::findIcon("run-build-file","") );
  ui->tool_act_runwith->setIcon( LXDG::findIcon("run-build-configure","") );
  ui->tool_act_cut->setIcon( LXDG::findIcon("edit-cut","") );
  ui->tool_act_copy->setIcon( LXDG::findIcon("edit-copy","") );
  ui->tool_act_paste->setIcon( LXDG::findIcon("edit-paste","") );
  ui->tool_act_rename->setIcon( LXDG::findIcon("edit-rename","") );
  ui->tool_act_rm->setIcon( LXDG::findIcon("edit-delete","") );
  ui->tool_act_fav->setIcon( LXDG::findIcon("bookmark-toolbar","") );
  //ToolBar Buttons
  ui->actionBack->setIcon( LXDG::findIcon("go-previous","") );
  ui->actionUp->setIcon( LXDG::findIcon("go-up","") );
  ui->actionHome->setIcon( LXDG::findIcon("go-home","") );
  ui->actionStopLoad->setIcon( LXDG::findIcon("dialog-cancel","") );
  ui->actionClose_Browser->setIcon( LXDG::findIcon("dialog-close","") );
}

void DirWidget::UpdateText(){
  ui->retranslateUi(this);
}

void DirWidget::UpdateButtons(){
  SelectionChanged();
}

// =================
//       PRIVATE
// =================
void DirWidget::setupConnections(){
  //Info routines
  connect(ui->treeWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OpenContextMenu()) );
  connect(ui->listWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OpenContextMenu()) );
  connect(ui->treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(SelectionChanged()) );
  connect(ui->listWidget, SIGNAL(itemSelectionChanged()), this, SLOT(SelectionChanged()) );

  //Activation routines	
  connect(ui->treeWidget, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(on_tool_act_run_clicked()) );
  connect(ui->listWidget, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(on_tool_act_run_clicked()) );
  connect(line_dir, SIGNAL(returnPressed()), this, SLOT(dir_changed()) );

  //Keyboard Shortcuts
  connect(copyFilesShort, SIGNAL(activated()), this, SLOT( on_tool_act_copy_clicked() ) );
  connect(cutFilesShort, SIGNAL(activated()), this, SLOT( on_tool_act_cut_clicked() ) );
  connect(pasteFilesShort, SIGNAL(activated()), this, SLOT( on_tool_act_paste_clicked() ) );
  connect(deleteFilesShort, SIGNAL(activated()), this, SLOT( on_tool_act_rm_clicked() ) );
  connect(refreshShort, SIGNAL(activated()), this, SLOT( refresh()) );
  //Filesystem Watcher
  connect(watcher, SIGNAL(directoryChanged(const QString&)), this, SLOT(refresh()) );
  connect(watcher, SIGNAL(fileChanged(const QString&)), this, SLOT(refresh()) ); //just in case
}

QStringList DirWidget::currentSelection(){
  QStringList out;
  if(showDetails){
    QList<QTreeWidgetItem*> sel = ui->treeWidget->selectedItems();
    for(int i=0; i<sel.length(); i++){
      out << sel[i]->whatsThis(0);
    }
  }else{
    QList<QListWidgetItem*> sel = ui->listWidget->selectedItems();
    for(int i=0; i<sel.length(); i++){
      out << sel[i]->whatsThis();
    }
  }
  out.removeDuplicates();
  return out;
}

// =================
//    PRIVATE SLOTS
// =================
//UI BUTTONS
// -- Left Action Buttons
void DirWidget::on_tool_act_cut_clicked(){
  QStringList sel = currentSelection();
  if(sel.isEmpty()){ return; }
  for(int i=0; i<sel.length(); i++){ sel[i] = CDIR+"/"+sel[i]; } //use absolute paths
  emit CutFiles(sel);
}

void DirWidget::on_tool_act_copy_clicked(){
  QStringList sel = currentSelection();
  if(sel.isEmpty()){ return; }
  for(int i=0; i<sel.length(); i++){ sel[i] = CDIR+"/"+sel[i]; } //use absolute paths
  emit CopyFiles(sel);	
}

void DirWidget::on_tool_act_fav_clicked(){
  QStringList sel = currentSelection();
  if(sel.isEmpty()){ return; }
  for(int i=0; i<sel.length(); i++){ sel[i] = CDIR+"/"+sel[i]; } //use absolute paths
  emit FavoriteFiles(sel);	
}

void DirWidget::on_tool_act_paste_clicked(){
  emit PasteFiles(CDIR);
}

void DirWidget::on_tool_act_rename_clicked(){
  QStringList sel = currentSelection();
  if(sel.isEmpty()){ return; }
  for(int i=0; i<sel.length(); i++){ sel[i] = CDIR+"/"+sel[i]; } //use absolute paths
  emit RenameFiles(sel);		
}

void DirWidget::on_tool_act_rm_clicked(){
  QStringList sel = currentSelection();
  if(sel.isEmpty()){ return; }
  for(int i=0; i<sel.length(); i++){ sel[i] = CDIR+"/"+sel[i]; } //use absolute paths
  emit RemoveFiles(sel);	
}

void DirWidget::on_tool_act_run_clicked(){
  QStringList sel = currentSelection();
  if(sel.isEmpty()){ return; }
  QStringList dirs;
  for(int i=0; i<sel.length(); i++){ 
    sel[i] = CDIR+"/"+sel[i]; //use absolute paths
    if(QFileInfo(sel[i]).isDir()){
      dirs << sel[i];
    }else{
      QProcess::startDetached("lumina-open \""+sel[i]+"\"");
    }
  }	  
  if(!dirs.isEmpty()){
    emit LoadDirectory(ID, dirs.takeFirst()); //load the first directory in this widget
  }
  if(!dirs.isEmpty()){
    emit OpenDirectories(dirs);
  }
}

void DirWidget::on_tool_act_runwith_clicked(){
  QStringList sel = currentSelection();
  if(sel.isEmpty()){ return; }
  for(int i=0; i<sel.length(); i++){ 
    QProcess::startDetached("lumina-open -select \""+CDIR+"/"+sel[i]+"\""); //use absolute paths
  }	  
}

// -- Bottom Action Buttons
void DirWidget::on_tool_goToImages_clicked(){
  QStringList sel = currentSelection();
  if(sel.isEmpty()){ emit ViewFiles(CLIST); }
  else{ 
    //Just use the files from the current selection
    LFileInfoList list;
    for(int i=0; i<CLIST.length(); i++){
      if(CLIST[i].isImage() && sel.contains(CLIST[i].fileName()) ){
        list << CLIST[i]; //add to the list
      }
    }
    if(!list.isEmpty()){ emit ViewFiles(list); }
  }
}

void DirWidget::on_tool_goToPlayer_clicked(){
  QStringList sel = currentSelection();
  if(sel.isEmpty()){ emit PlayFiles(CLIST); }
  else{ 
    //Just use the files from the current selection
    LFileInfoList list;
    for(int i=0; i<CLIST.length(); i++){
      if(CLIST[i].isAVFile() && sel.contains(CLIST[i].fileName()) ){
        list << CLIST[i]; //add to the list
      }
    }
    if(!list.isEmpty()){ emit PlayFiles(list); }
  }
}

// -- Top Snapshot Buttons
void DirWidget::on_tool_snap_newer_clicked(){
  ui->slider_snap->setValue( ui->slider_snap->value()+1 );
}

void DirWidget::on_tool_snap_older_clicked(){
  ui->slider_snap->setValue( ui->slider_snap->value()-1 );
}

void DirWidget::on_slider_snap_valueChanged(int val){
  //Update the snapshot interface
  ui->tool_snap_newer->setEnabled(val < ui->slider_snap->maximum());
  ui->tool_snap_older->setEnabled(val > ui->slider_snap->minimum());
  if(val >= snapshots.length() || val < 0){ 
    ui->label_snap->setText(tr("Current"));
  }else{
    ui->label_snap->setText( QFileInfo(snapbasedir+snapshots[val]).created().toString(Qt::DefaultLocaleShortDate) );
  }
  //Exit if a non-interactive snapshot change
  if(!ui->group_snaps->isEnabled()){ return; } //internal change - do not try to change the actual info
  //Determine which snapshot is now selected
  QString dir;
  //qDebug() << "Changing snapshot:" << CDIR << val;
  stopload = true; //stop any currently-loading procedures
  if(val >= snapshots.length() || val < 0){ //active system selected
    //qDebug() << " - Load Active system:" << normalbasedir;
    dir = normalbasedir;
  }else{
    dir = snapbasedir+snapshots[val]+"/";
    if(snaprelpath.isEmpty()){
      //Need to figure out the relative path within the snapshot
      snaprelpath = CDIR.section(snapbasedir.section(ZSNAPDIR,0,0), 1,1000);
      //qDebug() << " - new snapshot-relative path:" << snaprelpath;
    }
    dir.append(snaprelpath);
    dir.replace("//","/"); //just in case any duplicate slashes from all the split/combining
    //qDebug() << " - Load Snapshot:" << dir;
  }
  //Make sure this directory exists, and back up as necessary
  
  while(!QFile::exists(dir) && !dir.isEmpty()){
    dir = dir.section("/",0,-2); //back up one dir
  }
  if(dir.isEmpty()){ return; }
  //Load the newly selected snapshot
  emit LoadDirectory(ID, dir);
}

//Top Toolbar buttons
void DirWidget::on_actionBack_triggered(){
  if(history.isEmpty()){ return; } //cannot do anything
  QString dir = history.takeLast();
  if(dir == normalbasedir){
    dir = history.last();
  }
  emit LoadDirectory(ID, dir);
}

void DirWidget::on_actionUp_triggered(){
  QString dir = CDIR.section("/",0,-2);
  //Quick check to ensure the directory exists
  while(!QFile::exists(dir) && !dir.isEmpty()){
    dir = dir.section("/",0,-2); //back up one additional dir
  }
  emit LoadDirectory(ID, dir);
}

void DirWidget::on_actionHome_triggered(){
  emit LoadDirectory(ID, QDir::homePath());
}

void DirWidget::on_actionStopLoad_triggered(){
  stopload = true;
  ui->actionStopLoad->setVisible(false);
}

void DirWidget::dir_changed(){
  QString dir = line_dir->text();
  //Quick check to ensure the directory exists
  while(!QFile::exists(dir) && !dir.isEmpty()){
    dir = dir.section("/",0,-2); //back up one additional dir
  }
  emit LoadDirectory(ID, dir);
}

void DirWidget::on_actionClose_Browser_triggered(){
  emit CloseBrowser(ID);
}

// - Other Actions without a specific button on the side
void DirWidget::fileCheckSums(){
  QStringList files = currentSelection();
  if(files.isEmpty()){ return; }
  for(int i=0; i<files.length(); i++){ files[i] = CDIR+"/"+files[i]; } //use absolute paths
  qDebug() << "Run Checksums:" << files;
  QStringList info = LOS::Checksums(files);
  qDebug() << " - Info:" << info;
  if(info.isEmpty() || (info.length() != files.length()) ){ return; }
  for(int i=0; i<info.length(); i++){
    info[i] = QString("%2  \t(%1)").arg(files[i].section("/",-1), info[i]);
  }
  /*QMessageBox dlg(this);
    dlg.setWindowFlags( Qt::Dialog );
    dlg.setWindowTitle( tr("File Checksums") );
    dlg.setDetailedText(info.join("\n"));
  dlg.exec();*/
  QMessageBox::information(this, tr("File Checksums"), info.join("\n") );
}

void DirWidget::fileProperties(){
  QStringList sel = currentSelection();
  if(sel.isEmpty()){ return; }
  if(!LUtils::isValidBinary("lumina-fileinfo")){
    //It should never get to this point due to checks earlier - but just in case...
    QMessageBox::warning(this, tr("Missing Utility"), tr("The \"lumina-fileinfo\" utility could not be found on the system. Please install it first.") );
    return;
  }
  for(int i=0; i<sel.length(); i++){ 
    QProcess::startDetached("lumina-fileinfo \""+CDIR+"/"+sel[i]+"\""); //use absolute paths
  }
}

void DirWidget::openTerminal(){
  emit LaunchTerminal(CDIR);
}

void DirWidget::NewFile(){
	
}

void DirWidget::NewDir(){
	
}

//Browser Functions
void DirWidget::OpenContextMenu(){
  //First generate the context menu based on the selection
  QStringList sel = currentSelection();
  contextMenu->clear();
  if(!sel.isEmpty()){  
    contextMenu->addAction(LXDG::findIcon("run-build-file",""), tr("Open"), this, SLOT(on_tool_act_run_clicked()) );
    contextMenu->addAction(LXDG::findIcon("run-build-configure",""), tr("Open With..."), this, SLOT(on_tool_act_runwith_clicked()) );

    contextMenu->addAction(LXDG::findIcon("edit-rename",""), tr("Rename"), this, SLOT(on_tool_act_rename_clicked()) )->setEnabled(canmodify);
    contextMenu->addAction(LXDG::findIcon("document-encrypted",""), tr("View Checksums"), this, SLOT(fileCheckSums()) );
    contextMenu->addSeparator();
  }
  //Now add the general selection options
  contextMenu->addAction(LXDG::findIcon("edit-cut",""), tr("Cut Selection"), this, SLOT(on_tool_act_cut_clicked()) )->setEnabled(canmodify && !sel.isEmpty());
  contextMenu->addAction(LXDG::findIcon("edit-copy",""), tr("Copy Selection"), this, SLOT(on_tool_act_copy_clicked()) )->setEnabled(!sel.isEmpty());
  contextMenu->addAction(LXDG::findIcon("edit-paste",""), tr("Paste"), this, SLOT(on_tool_act_paste_clicked()) )->setEnabled(QApplication::clipboard()->mimeData()->hasFormat("x-special/lumina-copied-files") && canmodify);
  contextMenu->addSeparator();
  contextMenu->addAction(LXDG::findIcon("edit-delete",""), tr("Delete Selection"), this, SLOT(on_tool_act_rm_clicked()) )->setEnabled(canmodify&&!sel.isEmpty());
  if(LUtils::isValidBinary("lumina-fileinfo")){
    contextMenu->addSeparator();
    contextMenu->addAction(LXDG::findIcon("edit-find-replace",""), tr("File Properties"), this, SLOT(fileProperties()) )->setEnabled(!sel.isEmpty());
  }
  contextMenu->addSeparator();
  contextMenu->addAction(LXDG::findIcon("system-search",""), tr("Open Terminal here"), this, SLOT(openTerminal()));
  
  //Now open the menu at the current cursor location
  contextMenu->popup(QCursor::pos());
}

void DirWidget::SelectionChanged(){
  //Go through and enable/disable the buttons as necessary
  bool hasselection = !currentSelection().isEmpty();
  ui->tool_act_copy->setEnabled(hasselection);
  ui->tool_act_cut->setEnabled(hasselection && canmodify);
  ui->tool_act_fav->setEnabled(hasselection);
  ui->tool_act_paste->setEnabled(canmodify && QApplication::clipboard()->mimeData()->hasFormat("x-special/lumina-copied-files") );
  ui->tool_act_rename->setEnabled(hasselection && canmodify);
  ui->tool_act_rm->setEnabled(hasselection && canmodify);
  ui->tool_act_run->setEnabled(hasselection);
  ui->tool_act_runwith->setEnabled(hasselection);
}
