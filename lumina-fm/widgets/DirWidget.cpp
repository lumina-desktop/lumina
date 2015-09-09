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
#include <QTimer>
#include <QInputDialog>
#include <QScrollBar>

#include <LuminaOS.h>
#include <LuminaXDG.h>
#include <LuminaUtils.h>

#include "../ScrollDialog.h"

#ifndef DEBUG
#define DEBUG 0
#endif

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
  //Add the browser widgets
  listWidget = new DDListWidget(this);
  treeWidget = new DDTreeWidget(this);
  ui->browser_layout->addWidget(listWidget);
  ui->browser_layout->addWidget(treeWidget);
  //Create the keyboard shortcuts
  copyFilesShort = new QShortcut( QKeySequence(tr("Ctrl+C")), this);
  pasteFilesShort = new QShortcut( QKeySequence(tr("Ctrl+V")), this);
  cutFilesShort = new QShortcut( QKeySequence(tr("Ctrl+X")), this);
  deleteFilesShort = new QShortcut( QKeySequence(tr("Delete")), this);
  refreshShort = new QShortcut( QKeySequence(tr("F5")), this);
  //Create the filesystem watcher
  watcher = new QFileSystemWatcher(this);
  synctimer = new QTimer(this);
    synctimer->setInterval(100); // 1/10 second pause (combine simultaneous signals from the watcher)
    synctimer->setSingleShot(true);
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
  stopload = true; //just in case it is still loading
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
  listWidget->setVisible(!showDetails);
  treeWidget->setVisible(showDetails);
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
    treeWidget->setHeaderItem(it);
    //Now reset the sorting (alphabetically, dirs first)
    if(nmcol>=0){ treeWidget->sortItems(nmcol, Qt::AscendingOrder); } // sort by name
    if(typecol>=0){ treeWidget->sortItems(typecol, Qt::AscendingOrder); } //sort by type first
    
  if(CDIR.isEmpty() || !showDetails){ return; } //don't need to reload dir if details are not visible
  this->refresh();
}

void DirWidget::setThumbnailSize(int px){
  bool larger = listWidget->iconSize().height() < px;
  listWidget->setIconSize(QSize(px,px));
  treeWidget->setIconSize(QSize(px,px));
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
  QTime time;
  if(DEBUG){time.start(); }
  qDebug() << "Load Dir:" << dir;
  QString lastdir = CDIR; //for some checks later
  QString lastbasedir = normalbasedir;
  CDIR = dir;
  if(CDIR.endsWith("/") && CDIR.length() != 1){ CDIR.chop(1); }
  CLIST = list; //save for laterr
  canmodify = QFileInfo(CDIR).isWritable();
  if(DEBUG){ qDebug() << "Clear UI:" <<time.elapsed(); }
  //Clear the status text
  if(!canmodify){ui->label_status->setText(tr("(Limited Access) ")); }
  else{ ui->label_status->setText(""); }
  //Hide the extra buttons for a moment
  ui->tool_goToPlayer->setVisible(false);
  ui->tool_goToImages->setVisible(false);
  ui->tool_new_dir->setVisible(canmodify);
  ui->tool_new_file->setVisible(canmodify);
  //Set the drag/drop info as appripriate
  if(canmodify){
    listWidget->setWhatsThis(CDIR);
    treeWidget->setWhatsThis(CDIR);
  }else{
    listWidget->setWhatsThis("");
    treeWidget->setWhatsThis("");
  }
  //Determine if this is an internal ZFS snapshot
  bool loadsnaps = false;
  if(DEBUG){ qDebug() << "Load Snap Info:" << time.elapsed(); }
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
  if(DEBUG){ qDebug() << "Update History:" <<time.elapsed(); }
  //Now update the history for this browser
  //qDebug() << "History:" << history << normalbasedir << lastbasedir;
  if(!history.isEmpty() && history.last() == normalbasedir && lastbasedir!=normalbasedir ){
    //We went back one - remove this from the history
    history.takeLast();
    ui->actionBack->setEnabled(!history.isEmpty());
    //qDebug() << " - Duplicate: removed item";
  }else if(lastbasedir!=normalbasedir){ //not a refresh or internal snapshot change
    //qDebug() << " - New History Item:" << normalbasedir;
    history << normalbasedir;
    ui->actionBack->setEnabled(history.length()>1);
  }
  if(DEBUG){ qDebug() << "Update Watcher:" << time.elapsed(); }
  //Clear the current watcher
  if(!watcher->directories().isEmpty()){ watcher->removePaths(watcher->directories()); }
  if(!watcher->files().isEmpty()){ watcher->removePaths(watcher->files()); }
  watcher->addPath(CDIR);
  ui->actionStopLoad->setVisible(true);
  stopload = false;
  //Clear the display widget (if a new directory)
    if(DEBUG){ qDebug() << "Clear Browser Widget:" << time.elapsed(); }
  double scrollpercent = -1;
  if(lastbasedir != normalbasedir){
    if(showDetails){ treeWidget->clear(); }
    else{ listWidget->clear(); }
    QApplication::processEvents(); //make sure it is cleared right away
  }else{
    //Need to be smarter about which items need to be removed
    // - compare the old/new lists and remove any items not in the new listing (new items taken care of below)
    QStringList newfiles; //just the filenames
    for(int i=0; i<CLIST.length(); i++){ newfiles << CLIST[i].fileName(); }
    if(showDetails){
      for(int i=0; i<treeWidget->topLevelItemCount(); i++){
        if( !newfiles.contains(treeWidget->topLevelItem(i)->whatsThis(0).section("/",-1)) ){
	  delete treeWidget->takeTopLevelItem(i); 
	  i--;
	}
      }
      QApplication::processEvents(); //make sure the scrollbar is up to date after removals
      scrollpercent = treeWidget->verticalScrollBar()->value()/( (double) treeWidget->verticalScrollBar()->maximum());
    }else{
      for(int i=0; i<listWidget->count(); i++){
        if( !newfiles.contains(listWidget->item(i)->text()) ){
	  delete listWidget->takeItem(i); 
	  i--;
	}
      }
      QApplication::processEvents(); //make sure the scrollbar is up to date after removals
      scrollpercent = listWidget->horizontalScrollBar()->value()/( (double) listWidget->horizontalScrollBar()->maximum());
    }
  } //end check for CDIR reload
  //Now fill the display widget
  bool hasimages, hasmultimedia;
  hasimages = hasmultimedia = false;
  int numdirs = 0;
  qint64 filebytes = 0;
  if(DEBUG){ qDebug() << "Start Loop over items:" << time.elapsed(); }
  for(int i=0; i<list.length(); i++){
    if(stopload){ ui->actionStopLoad->setVisible(false); return; } //stop right now
    if(!hasimages && list[i].isImage()){ hasimages = true;  ui->tool_goToImages->setVisible(true); }
    else if(!hasmultimedia && list[i].isAVFile()){ hasmultimedia = true;  ui->tool_goToPlayer->setVisible(true); }
    //Update statistics
    if(list[i].isDir()){ numdirs++; }
    else{ filebytes += list[i].size(); }
    watcher->addPath(list[i].absoluteFilePath());
    if(showDetails){
      //Now create all the individual items for the details tree
      QTreeWidgetItem *it;
      bool addnew = false;
	//See if an item already exists for this file
	QList<QTreeWidgetItem*> items = treeWidget->findItems(list[i].fileName(),Qt::MatchExactly,0); //NOTE: This requires column 0 to be the name
	if(items.isEmpty()){
	    it = new QTreeWidgetItem();
	    addnew = true;
	}else{
	    it = items.first();
	}
	//Now update the entry contents
	it->setWhatsThis(0, QString(canmodify ? "cut": "copy")+"::::"+list[i].absoluteFilePath());
      for(int t=0; t<listDetails.length(); t++){
        switch(listDetails[t]){
	  case NAME:
	    it->setText(t,list[i].fileName());
	    it->setStatusTip(t, list[i].fileName());
	      //Since the icon/image is based on the filename - only update this for a new item
	      // (This is the slowest part of the routine)
	      if(list[i].isImage()){
	        if(showThumbs){ it->setIcon(t, QIcon( QPixmap(list[i].absoluteFilePath()).scaled(treeWidget->iconSize(),Qt::IgnoreAspectRatio, Qt::FastTransformation) ) ); }
	        else{ it->setIcon(t, LXDG::findIcon(list[i].iconfile(),"image-x-generic") ); }
	      }else if(addnew){
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
      if(addnew){ treeWidget->addTopLevelItem(it); }
      if(lastdir == CDIR+"/"+list[i].fileName()){ 
	treeWidget->setCurrentItem(it);
	treeWidget->scrollToItem(it);
      }
    }else{
	//Create all the individual items for the basic list
	QListWidgetItem *it;
	  //See if there is an existing item to re-use
	  bool addnew = false;
	  QList<QListWidgetItem*> items = listWidget->findItems(list[i].fileName(), Qt::MatchExactly);
	  if(items.isEmpty()){
	    it = new QListWidgetItem();
	    addnew = true;
	  }else{ it = items.first(); }
	    it->setWhatsThis( QString(canmodify ? "cut": "copy")+"::::"+list[i].absoluteFilePath()); //used for drag and drop
	    it->setText(list[i].fileName());
	    it->setStatusTip(list[i].fileName());
	    //Since the icon/image is based on the filename - only update this for a new items (non-thumbnail)
	    // (This is the slowest part of the routine)
	    if(list[i].isImage()){
	      if(showThumbs){ it->setIcon(QIcon( QPixmap(list[i].absoluteFilePath()).scaled(listWidget->iconSize(),Qt::IgnoreAspectRatio, Qt::FastTransformation) ) ); }
	      else{ it->setIcon(LXDG::findIcon(list[i].iconfile(),"image-x-generic") ); }
	    }else if(addnew){
	      it->setIcon(LXDG::findIcon(list[i].iconfile(),"unknown") );
	    }
	listWidget->addItem(it);
	if(lastdir == CDIR+"/"+it->whatsThis()){ 
	  listWidget->setCurrentItem(it);
	  listWidget->scrollToItem(it);
	}
    }
    if(i%15==0){ QApplication::processEvents(); }//keep the UI snappy while loading a directory
    if(DEBUG){ qDebug() << " - item finished:" << i << time.elapsed(); }
  }
  if(DEBUG){ qDebug() << "Done with item loop:" << time.elapsed() << list.length(); }
  ui->actionStopLoad->setVisible(false);
  //Another check to ensure the current item is visible (or return to the same scroll position)
  if(stopload){ return; } //stop right now
  if(scrollpercent<0){
    if(showDetails){
      for(int t=0; t<treeWidget->columnCount(); t++){treeWidget->resizeColumnToContents(t); }
      if(treeWidget->currentItem()!=0){ treeWidget->scrollToItem(treeWidget->currentItem()); }
    }else{
      if(listWidget->currentItem()!=0){ listWidget->scrollToItem(listWidget->currentItem()); }
    }
  }else{
    if(showDetails){
      treeWidget->verticalScrollBar()->setValue( qRound(treeWidget->verticalScrollBar()->maximum()*scrollpercent) );
    }else{
      listWidget->horizontalScrollBar()->setValue( qRound(listWidget->horizontalScrollBar()->maximum()*scrollpercent) );
    }
  }

  
  if(stopload){ return; } //stop right now
  if(DEBUG){ qDebug() << "Assemble Status Message:" << time.elapsed(); }
  //Assemble any status message
  QString stats = QString(tr("Capacity: %1")).arg(LOS::FileSystemCapacity(CDIR));
  if(list.length()>0){
    stats.prepend("\t");
    if(numdirs < list.length()){
      //Has Files
      stats.prepend( QString(tr("Files: %1 (%2)")).arg(QString::number(list.length()-numdirs), LUtils::BytesToDisplaySize(filebytes)) );
    }
    if(numdirs > 0){
      //Has Dirs
      if(numdirs<list.length()){ stats.prepend(" / "); }//has files output already
      stats.prepend( QString(tr("Dirs: %1")).arg(QString::number(numdirs)) );
    }
    
  }
  if(stopload){ return; } //stop right now  
  ui->label_status->setText( QString(ui->label_status->text()+stats).simplified() );
  if(DEBUG){ qDebug() << "DONE:" << time.elapsed(); }
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
  ui->tool_new_file->setIcon( LXDG::findIcon("document-new","") );
  ui->tool_new_dir->setIcon( LXDG::findIcon("folder-new","") );
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
  connect(treeWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OpenContextMenu()) );
  connect(listWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OpenContextMenu()) );
  connect(treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(SelectionChanged()) );
  connect(listWidget, SIGNAL(itemSelectionChanged()), this, SLOT(SelectionChanged()) );
  
  //Activation routines	
  connect(treeWidget, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(on_tool_act_run_clicked()) );
  connect(treeWidget, SIGNAL(DataDropped(QString, QStringList)), this, SIGNAL(PasteFiles(QString, QStringList)) );
  connect(listWidget, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(on_tool_act_run_clicked()) );
  connect(listWidget, SIGNAL(DataDropped(QString, QStringList)), this, SIGNAL(PasteFiles(QString, QStringList)) );
  connect(line_dir, SIGNAL(returnPressed()), this, SLOT(dir_changed()) );

  //Keyboard Shortcuts
  connect(copyFilesShort, SIGNAL(activated()), this, SLOT( on_tool_act_copy_clicked() ) );
  connect(cutFilesShort, SIGNAL(activated()), this, SLOT( on_tool_act_cut_clicked() ) );
  connect(pasteFilesShort, SIGNAL(activated()), this, SLOT( on_tool_act_paste_clicked() ) );
  connect(deleteFilesShort, SIGNAL(activated()), this, SLOT( on_tool_act_rm_clicked() ) );
  connect(refreshShort, SIGNAL(activated()), this, SLOT( refresh()) );
  //Filesystem Watcher
  connect(watcher, SIGNAL(directoryChanged(const QString&)), this, SLOT(startSync()) );
  connect(watcher, SIGNAL(fileChanged(const QString&)), this, SLOT(startSync()) ); //just in case
  connect(synctimer, SIGNAL(timeout()), this, SLOT(refresh()) );
}

QStringList DirWidget::currentSelection(){
  QStringList out;
  if(showDetails){
    QList<QTreeWidgetItem*> sel = treeWidget->selectedItems();
    for(int i=0; i<sel.length(); i++){
      out << sel[i]->whatsThis(0).section("::::",1,100); //absolute file path
    }
  }else{
    QList<QListWidgetItem*> sel = listWidget->selectedItems();
    for(int i=0; i<sel.length(); i++){
      out << sel[i]->whatsThis().section("::::",1,100); //absolute file path
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
  emit CutFiles(sel);
}

void DirWidget::on_tool_act_copy_clicked(){
  QStringList sel = currentSelection();
  if(sel.isEmpty()){ return; }
  emit CopyFiles(sel);	
}

void DirWidget::on_tool_act_fav_clicked(){
  QStringList sel = currentSelection();
  if(sel.isEmpty()){ return; }
  emit FavoriteFiles(sel);	
}

void DirWidget::on_tool_act_paste_clicked(){
  emit PasteFiles(CDIR, QStringList()); //use the clipboard for pasting
}

void DirWidget::on_tool_act_rename_clicked(){
  QStringList sel = currentSelection();
  if(sel.isEmpty()){ return; }
  emit RenameFiles(sel);		
}

void DirWidget::on_tool_act_rm_clicked(){
  QStringList sel = currentSelection();
  if(sel.isEmpty()){ return; }
  emit RemoveFiles(sel);	
}

void DirWidget::on_tool_act_run_clicked(){
  QStringList sel = currentSelection();
  if(sel.isEmpty()){ return; }
  QStringList dirs;
  for(int i=0; i<sel.length(); i++){ 
    if(QFileInfo(sel[i]).isDir()){
      dirs << sel[i];
    }else{
      QProcess::startDetached("lumina-open \""+sel[i]+"\"");
    }
  }	  
  if(!dirs.isEmpty()){
    stopload = true; //just in case it is still loading
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
    QProcess::startDetached("lumina-open -select \""+sel[i]+"\""); //use absolute paths
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
      if(CLIST[i].isImage() && sel.contains(CLIST[i].absoluteFilePath()) ){
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
      if(CLIST[i].isAVFile() && sel.contains(CLIST[i].absoluteFilePath()) ){
        list << CLIST[i]; //add to the list
      }
    }
    if(!list.isEmpty()){ emit PlayFiles(list); }
  }
}

void DirWidget::on_tool_new_file_clicked(){
  if(!canmodify){ return; } //cannot create anything here
  //Prompt for the new filename
  bool ok = false;
  QString newdocument = QInputDialog::getText(this, tr("New Document"), tr("Name:"), QLineEdit::Normal, "", \
        &ok, 0, Qt::ImhFormattedNumbersOnly | Qt::ImhUppercaseOnly | Qt::ImhLowercaseOnly);
  if(!ok || newdocument.isEmpty()){ return; }	
  //Create the empty file
  QString full = CDIR;
  if(!full.endsWith("/")){ full.append("/"); }
  QFile file(full+newdocument);
  if(file.open(QIODevice::ReadWrite)){
    //If successfully opened, it has created a blank file
    file.close();
  }else{
    QMessageBox::warning(this, tr("Error Creating Document"), tr("The document could not be created. Please ensure that you have the proper permissions."));	  
  }
}

void DirWidget::on_tool_new_dir_clicked(){
  if(!canmodify){ return; } //cannot create anything here
  //Prompt for the new dir name
  bool ok = false;
  QString newdir = QInputDialog::getText(this, tr("New Directory"), tr("Name:"), QLineEdit::Normal, "", \
		&ok, 0, Qt::ImhFormattedNumbersOnly | Qt::ImhUppercaseOnly | Qt::ImhLowercaseOnly);
  if(!ok || newdir.isEmpty()){ return; }
  //Now create the new dir
  QString full = CDIR;
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
    //if(snaprelpath.isEmpty()){
      //Need to figure out the relative path within the snapshot
      snaprelpath = CDIR.section(snapbasedir.section(ZSNAPDIR,0,0), 1,1000);
      //qDebug() << " - new snapshot-relative path:" << snaprelpath;
    //}
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
  stopload = true; //just in case it is still loading
  emit LoadDirectory(ID, dir);
}

//Top Toolbar buttons
void DirWidget::on_actionBack_triggered(){
  if(history.isEmpty()){ return; } //cannot do anything
  QString dir = history.takeLast();
  //qDebug() << "Go Back:" << dir << normalbasedir << history.last();
  if(dir == normalbasedir){
    dir = history.takeLast();
  }
  stopload = true; //just in case it is still loading
  emit LoadDirectory(ID, dir);
}

void DirWidget::on_actionUp_triggered(){
  QString dir = CDIR.section("/",0,-2);
  if(dir.isEmpty())
      dir = "/";
  //Quick check to ensure the directory exists
  while(!QFile::exists(dir) && !dir.isEmpty()){
    dir = dir.section("/",0,-2); //back up one additional dir
  }
  stopload = true; //just in case it is still loading
  emit LoadDirectory(ID, dir);
}

void DirWidget::on_actionHome_triggered(){
  stopload = true; //just in case it is still loading
  emit LoadDirectory(ID, QDir::homePath());
}

void DirWidget::on_actionStopLoad_triggered(){
  stopload = true;
  ui->actionStopLoad->setVisible(false);
}

void DirWidget::dir_changed(){
  QString dir = line_dir->text().simplified();
  //Run the dir through the user-input checks
  dir = LUtils::PathToAbsolute(dir);
  qDebug() << "Dir:" << dir;
  //Quick check to ensure the directory exists
  while(!QFile::exists(dir) && !dir.isEmpty()){
    dir = dir.section("/",0,-2); //back up one additional dir
  }
  qDebug() << " - Now Dir:" << dir;
  stopload = true; //just in case it is still loading
  emit LoadDirectory(ID, dir);
}

void DirWidget::on_actionClose_Browser_triggered(){
  emit CloseBrowser(ID);
}

// - Other Actions without a specific button on the side
void DirWidget::fileCheckSums(){
  QStringList files = currentSelection();
  if(files.isEmpty()){ return; }
  qDebug() << "Run Checksums:" << files;
  QStringList info = LOS::Checksums(files);
  qDebug() << " - Info:" << info;
  if(info.isEmpty() || (info.length() != files.length()) ){ return; }
  for(int i=0; i<info.length(); i++){
    info[i] = QString("%2\n\t(%1)").arg(files[i].section("/",-1), info[i]);
  }
  ScrollDialog dlg(this);
    dlg.setWindowTitle( tr("File Checksums:") );
    dlg.setWindowIcon( LXDG::findIcon("document-encrypted","") );
    dlg.setText(info.join("\n"));
  dlg.exec();
  /*QMessageBox dlg(this);
    dlg.setWindowFlags( Qt::Dialog );
    dlg.setWindowTitle( tr("File Checksums") );
    dlg.setDetailedText(info.join("\n"));
  dlg.exec();*/
  //QMessageBox::information(this, tr("File Checksums"), info.join("\n") );
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
    QProcess::startDetached("lumina-fileinfo \""+sel[i]+"\""); //use absolute paths
  }
}

void DirWidget::openTerminal(){
  emit LaunchTerminal(CDIR);
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

void DirWidget::startSync(){
  if(synctimer->isActive()){ synctimer->stop(); }
  synctimer->start();
}

//====================
//         PROTECTED
//====================
void DirWidget::mouseReleaseEvent(QMouseEvent *ev){
  static Qt::MouseButtons backmap = Qt::BackButton | Qt::ExtraButton5;
  //qDebug() << "Mouse Click:" << ev->button();
  if(backmap.testFlag(ev->button())){
    ev->accept();
    on_actionBack_triggered();
  //}else if(ev->button()==Qt::ForwardButton()){
    //ev->accept();
  }else{
    ev->ignore(); //not handled here
  }
}
