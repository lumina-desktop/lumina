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
  canmodify = false; //initial value
  contextMenu = new QMenu(this);
  setShowDetails(true);
  UpdateIcons();
  UpdateText();	
  setupConnections();
}

DirWidget::~DirWidget(){
	
}

QString DirWidget::id(){
  return ID;
}

void DirWidget::setShowDetails(bool show){
  showDetails = show;
  ui->listWidget->setVisible(!showDetails);
  ui->treeWidget->setVisible(showDetails);
}

void DirWidget::setShowSidebar(bool show){
  ui->group_actions->setVisible(show);
}

void DirWidget::setDetails(QList<DETAILTYPES> list){
  listDetails = list;
  emit LoadDirectory(ID, CDIR);
}

// ================
//    PUBLIC SLOTS
// ================
void DirWidget::LoadDir(QString dir, QList<LFileInfo> list){
  if(dir.isEmpty()){ return; } //nothing to do
  QString lastdir = CDIR; //for some checks later
  canmodify = QFileInfo(CDIR).isWritable();
  CLIST = list; //save for later
  CDIR = dir;
  //Hide the extra buttons for a moment
  ui->tool_goToPlayer->setVisible(false);
  ui->tool_goToImages->setVisible(false);
  //Determine if this is an internal ZFS snapshot
  bool loadsnaps = false;
  if( dir.contains(ZSNAPDIR) ){
    //This is a zfs snapshot - only update the saved paths necessary to rotate between snapshots/system
    snaprelpath = dir.section(ZSNAPDIR,1,1000).section("/",1,1000); //the relative path inside the snapshot
    normalbasedir = dir.section(ZSNAPDIR,0,0)+"/"+snaprelpath; //Update the new base directory
    //See if this was a manual move to the directory, or an internal move
    QString tmp = dir.section(ZSNAPDIR,0,0);
    if(tmp != snapbasedir.section(ZSNAPDIR,0,0)){
      loadsnaps = true; //different snapshot loaded - need to update internally
    }
  }else{
    //This is a normal directory - prompt for snapshot information
    normalbasedir = dir;
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
  //Clear the display widget
  if(showDetails){ 
    ui->treeWidget->clear(); 
    //Need to re-create the header item as well
    QTreeWidgetItem *it = new QTreeWidgetItem();
    for(int t=0; t<listDetails.length(); t++){
      switch(listDetails[t]){
	  case NAME:
	    it->setText(t,tr("Name"));
	    break;
	  case SIZE:
	    it->setText(t,tr("Size"));
	    break;
	  case TYPE:
	    it->setText(t, tr("Type"));
	  case DATEMOD:
	    it->setText(t, tr("Date Modified") );
	    break;
	  case DATECREATE:
	    it->setText(t, tr("Date Created") );
	    break;
	}
    }	  
    ui->treeWidget->setHeaderItem(it);
  }else{ ui->listWidget->clear(); }
  //Now fill the display widget
  bool hasimages, hasmultimedia;
  hasimages = hasmultimedia = false;
  for(int i=0; i<list.length(); i++){
    hasimages = hasimages || list[i].isImage();
    hasmultimedia = hasmultimedia || list[i].isAVFile();
    if(showDetails){
      //Now create all the individual items for the details tree
      QTreeWidgetItem *it = new QTreeWidgetItem();
	it->setWhatsThis(0, list[i].fileName());
      for(int t=0; t<listDetails.length(); t++){
        switch(listDetails[t]){
	  case NAME:
	    it->setText(t,list[i].fileName());
	    if(list[i].isImage()){
	      it->setIcon(t, QIcon( QPixmap(list[i].absoluteFilePath()).scaledToHeight(ui->treeWidget->iconSize().height()) ) );
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
	    if(list[i].isImage()){
	      it->setIcon(QIcon( QPixmap(list[i].absoluteFilePath()).scaledToHeight(ui->listWidget->iconSize().height()) ) );
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
  //Another check to ensure the current item is visible
  if(showDetails){
    if(ui->treeWidget->currentItem()!=0){ ui->treeWidget->scrollToItem(ui->treeWidget->currentItem()); }
  }else{
    if(ui->listWidget->currentItem()!=0){ ui->listWidget->scrollToItem(ui->listWidget->currentItem()); }
  }
  //Now Re-enable buttons as necessary
  ui->tool_goToPlayer->setVisible(hasimages);
  ui->tool_goToImages->setVisible(hasmultimedia);
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
  //Tree Widget interaction
  connect(ui->treeWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OpenContextMenu()) );
  connect(ui->listWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OpenContextMenu()) );
  connect(ui->treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(SelectionChanged()) );
  connect(ui->listWidget, SIGNAL(itemSelectionChanged()), this, SLOT(SelectionChanged()) );
	
  /*connect(ui->tree_dir_view, SIGNAL(activated(const QModelIndex&)), this, SLOT(ItemRun(const QModelIndex&)) );	
  connect(ui->list_dir_view, SIGNAL(activated(const QModelIndex&)), this, SLOT(ItemRun(const QModelIndex&)) );
  connect(ui->tree_dir_view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(ItemSelectionChanged()) );
  connect(ui->list_dir_view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(ItemSelectionChanged()) );*/
	
	
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
  emit ViewFiles(CLIST);
}

void DirWidget::on_tool_goToPlayer_clicked(){
  emit PlayFiles(CLIST);
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
  qDebug() << "Changing snapshot:" << CDIR << val;
  if(val >= snapshots.length() || val < 0){ //active system selected
    qDebug() << " - Load Active system:" << normalbasedir;
    dir = normalbasedir;
  }else{
    dir = snapbasedir+snapshots[val]+"/";
    if(snaprelpath.isEmpty()){
      //Need to figure out the relative path within the snapshot
      snaprelpath = CDIR.section(snapbasedir.section(ZSNAPDIR,0,0), 1,1000);
      qDebug() << " - new snapshot-relative path:" << snaprelpath;
    }
    dir.append(snaprelpath);
    qDebug() << " - Load Snapshot:" << dir;
  }
  //Make sure this directory exists, and back up as necessary
  while(!QFile::exists(dir) && !dir.isEmpty()){
    dir = dir.section("/",0,-1); //back up one dir
  }
  if(dir.isEmpty()){ return; }
  //Load the newly selected snapshot
  emit LoadDirectory(ID, dir);
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
