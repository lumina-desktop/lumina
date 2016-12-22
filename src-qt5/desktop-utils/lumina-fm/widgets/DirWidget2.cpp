//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "DirWidget2.h"
#include "ui_DirWidget2.h"

#include <QMessageBox>
#include <QCursor>
#include <QClipboard>
#include <QMimeData>
#include <QTimer>
#include <QInputDialog>
#include <QScrollBar>
#include <QSettings>
#include <QtConcurrent/QtConcurrentRun>

#include <LuminaOS.h>
#include <LuminaXDG.h>
#include <LUtils.h>

#include "../ScrollDialog.h"

#define DEBUG 0

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
    connect(line_dir, SIGNAL(returnPressed()), this, SLOT(dir_changed()) );
  toolbar->addAction(ui->actionSingleColumn);
  ui->actionSingleColumn->setChecked(true);
  toolbar->addAction(ui->actionDualColumn);
  toolbar->addAction(ui->actionMenu);
  //Add the browser widgets
  RCBW = 0; //right column browser is unavailable initially
  BW = new BrowserWidget("", this);
  ui->browser_layout->addWidget(BW);
  connect(BW, SIGNAL(dirChange(QString)), this, SLOT(currentDirectoryChanged()) );
  connect(BW, SIGNAL(itemsActivated()), this, SLOT(runFiles()) );
  connect(BW, SIGNAL(DataDropped(QString, QStringList)), this, SIGNAL(PasteFiles(QString, QStringList)) );
  connect(BW, SIGNAL(contextMenuRequested()), this, SLOT(OpenContextMenu()) );
  connect(BW, SIGNAL(updateDirectoryStatus(QString)), this, SLOT(dirStatusChanged(QString)) );
  connect(BW, SIGNAL(hasFocus(QString)), this, SLOT(setCurrentBrowser(QString)) );
  //Now update the rest of the UI
  canmodify = false; //initial value
  contextMenu = new QMenu(this);
  cNewMenu = cOpenMenu = cFModMenu = cFViewMenu = 0; //not created yet
  connect(contextMenu, SIGNAL(aboutToShow()), this, SLOT(UpdateContextMenu()) );

  UpdateIcons();
  UpdateText();	
  createShortcuts();
  createMenus();
}

DirWidget::~DirWidget(){
  //stopload = true; //just in case another thread is still loading/running
}

void DirWidget::setFocusLineDir() {
  line_dir->setFocus();
  line_dir->selectAll();
}

void DirWidget::cleanup(){
  //stopload = true; //just in case another thread is still loading/running
  //if(thumbThread.isRunning()){ thumbThread.waitForFinished(); } //this will stop really quickly with the flag set
}

void DirWidget::ChangeDir(QString dirpath){
  //stopload = true; //just in case it is still loading
  //emit LoadDirectory(ID, dirpath);
  //qDebug() << "ChangeDir:" << dirpath;
  currentBrowser()->changeDirectory(dirpath);
}

void DirWidget::setDirCompleter(QCompleter *comp){
  //line_dir->setCompleter(comp);
}

QString DirWidget::id(){
  return ID;
}

QString DirWidget::currentDir(){
  return currentBrowser()->currentDirectory();
}

void DirWidget::setShowDetails(bool show){
  BW->showDetails(show);
  if(RCBW!=0){ RCBW->showDetails(show); }
}

void DirWidget::showHidden(bool show){
  BW->showHiddenFiles(show);
  if(RCBW!=0){ RCBW->showHiddenFiles(show); }
}

void DirWidget::setThumbnailSize(int px){
  BW->setThumbnailSize(px);
  if(RCBW!=0){ RCBW->setThumbnailSize(px); }
  ui->tool_zoom_in->setEnabled(px < 256); //upper limit on image sizes
  ui->tool_zoom_out->setEnabled(px >16); //lower limit on image sizes
}

// ================
//    PUBLIC SLOTS
// ================

void DirWidget::LoadSnaps(QString basedir, QStringList snaps){
  //Save these value internally for use later
  //qDebug() << "ZFS Snapshots available:" << basedir << snaps;
  snapbasedir = basedir;
  snapshots = snaps;
  //if(!snapbasedir.isEmpty()){ watcher->addPath(snapbasedir); } //add this to the watcher in case snapshots get created/removed
  //Now update the UI as necessary
  if(ui->tool_snap->menu()==0){ 
    ui->tool_snap->setMenu(new QMenu(this)); 
    connect(ui->tool_snap->menu(), SIGNAL(triggered(QAction*)), this, SLOT(direct_snap_selected(QAction*)) );
  }
  ui->tool_snap->menu()->clear();
  for(int i=0; i<snapshots.length(); i++){
    QAction *tmp = ui->tool_snap->menu()->addAction(snapshots[i]);
      tmp->setWhatsThis(snapshots[i]);
  }
  ui->slider_snap->setRange(0, snaps.length());
  if(currentBrowser()->currentDirectory().contains(ZSNAPDIR)){
    //The user was already within a snapshot - figure out which one and set the slider appropriately
    int index = snaps.indexOf( currentBrowser()->currentDirectory().section(ZSNAPDIR,1,1).section("/",0,0) );
    if(index < 0){ index = snaps.length(); } //unknown - load the system (should never happen)
    ui->slider_snap->setValue(index);
  }else{
    ui->slider_snap->setValue(snaps.length()); //last item (normal system)
  }
  on_slider_snap_valueChanged();
  QApplication::processEvents(); //let the slider changed signal get thrown away before we re-enable the widget
  ui->group_snaps->setEnabled(!snaps.isEmpty());
  ui->group_snaps->setVisible(!snaps.isEmpty());
  ui->tool_snap_newer->setEnabled(ui->slider_snap->value() < ui->slider_snap->maximum());
  ui->tool_snap_older->setEnabled(ui->slider_snap->value() > ui->slider_snap->minimum());
}

void DirWidget::refresh(){
  currentBrowser()->changeDirectory(""); //refresh current dir
}

//Theme change functions
void DirWidget::UpdateIcons(){
  //Snapshot buttons
  ui->tool_snap_newer->setIcon(LXDG::findIcon("go-next-view","") );
  ui->tool_snap_older->setIcon(LXDG::findIcon("go-previous-view","") );

  //ToolBar Buttons
  ui->actionBack->setIcon( LXDG::findIcon("go-previous","") );
  ui->actionUp->setIcon( LXDG::findIcon("go-up","") );
  ui->actionHome->setIcon( LXDG::findIcon("go-home","") );
  ui->actionMenu->setIcon( LXDG::findIcon("format-justify-fill","format-list-unordered") );
  ui->actionSingleColumn->setIcon(LXDG::findIcon("view-right-close","view-close") );
  ui->actionDualColumn->setIcon(LXDG::findIcon("view-right-new","view-split-left-right") );
  
  ui->tool_zoom_in->setIcon(LXDG::findIcon("zoom-in",""));
  ui->tool_zoom_out->setIcon(LXDG::findIcon("zoom-out",""));

}

void DirWidget::UpdateText(){
  ui->retranslateUi(this);
  BW->retranslate();
  if(RCBW!=0){ RCBW->retranslate(); }
}

// =================
//       PRIVATE
// =================
void DirWidget::createShortcuts(){
kZoomIn= new QShortcut(QKeySequence(QKeySequence::ZoomIn),this);
kZoomOut= new QShortcut(QKeySequence(QKeySequence::ZoomOut),this);
kNewFile= new QShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_F),this);
kNewDir= new QShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_N),this);
kNewXDG= new QShortcut(QKeySequence(Qt::CTRL+Qt::Key_G),this);
kCut= new QShortcut(QKeySequence(QKeySequence::Cut),this);
kCopy= new QShortcut(QKeySequence(QKeySequence::Copy),this);
kPaste= new QShortcut(QKeySequence(QKeySequence::Paste),this);
kRename= new QShortcut(QKeySequence(Qt::Key_F2),this);
kFav= new QShortcut(QKeySequence(Qt::Key_F3),this);
kDel= new QShortcut(QKeySequence(QKeySequence::Delete),this);
kOpSS= new QShortcut(QKeySequence(Qt::Key_F6),this);
kOpMM= new QShortcut(QKeySequence(Qt::Key_F7),this);
kOpTerm = new QShortcut(QKeySequence(Qt::Key_F1),this);

connect(kZoomIn, SIGNAL(activated()), this, SLOT(on_tool_zoom_in_clicked()) );
connect(kZoomOut, SIGNAL(activated()), this, SLOT(on_tool_zoom_out_clicked()) );
connect(kNewFile, SIGNAL(activated()), this, SLOT(createNewFile()) );
connect(kNewDir, SIGNAL(activated()), this, SLOT(createNewDir()) );
connect(kNewXDG, SIGNAL(activated()), this, SLOT(createNewXDGEntry()) );
connect(kCut, SIGNAL(activated()), this, SLOT(cutFiles()) );
connect(kCopy, SIGNAL(activated()), this, SLOT(copyFiles()) );
connect(kPaste, SIGNAL(activated()), this, SLOT(pasteFiles()) );
connect(kRename, SIGNAL(activated()), this, SLOT(renameFiles()) );
connect(kFav, SIGNAL(activated()), this, SLOT(favoriteFiles()) );
connect(kDel, SIGNAL(activated()), this, SLOT(removeFiles()) );
connect(kOpSS, SIGNAL(activated()), this, SLOT(openInSlideshow()) );
connect(kOpMM, SIGNAL(activated()), this, SLOT(openMultimedia()) );
connect(kOpTerm, SIGNAL(activated()), this, SLOT(openTerminal()) );

}

void DirWidget::createMenus(){
  //Note: contextMenu already created - this is just for the sub-items
  if(cNewMenu==0){ cNewMenu = new QMenu(this); }
  else{ cNewMenu->clear(); }
  cNewMenu->setTitle(tr("Create...") );
  cNewMenu->setIcon( LXDG::findIcon("list-add","") );
  cNewMenu->addAction(LXDG::findIcon("document-new",""), tr("File"), this, SLOT(createNewFile()), kNewFile->key() );
  cNewMenu->addAction(LXDG::findIcon("folder-new",""), tr("Directory"), this, SLOT(createNewDir()), kNewDir->key() );
  if(LUtils::isValidBinary("lumina-fileinfo")){ cNewMenu->addAction(LXDG::findIcon("system-run",""), tr("Application Launcher"), this, SLOT(createNewXDGEntry()), kNewXDG->key() ); }

  if(cOpenMenu==0){ cOpenMenu = new QMenu(this); }
  else{ cOpenMenu->clear(); }
  cOpenMenu->setTitle(tr("Launch..."));
  cOpenMenu->setIcon( LXDG::findIcon("quickopen","") );
  cOpenMenu->addAction(LXDG::findIcon("utilities-terminal",""), tr("Terminal"), this, SLOT(openTerminal()), kOpTerm->key());
  cOpenMenu->addAction(LXDG::findIcon("view-preview",""), tr("SlideShow"), this, SLOT(openInSlideshow()), kOpSS->key());
  cOpenMenu->addAction(LXDG::findIcon("view-media-lyrics","media-playback-start"), tr("Multimedia Player"), this, SLOT(openMultimedia()), kOpMM->key());
/*
  if(cFModMenu==0){ cFModMenu = new QMenu(this); }
  else{ cFModMenu->clear(); }
  cFModMenu->setTitle(tr("Modify Files..."));
  cFModMenu->setIcon( LXDG::findIcon("document-edit","") );
  cFModMenu->addAction(LXDG::findIcon("edit-cut",""), tr("Cut Selection"), this, SLOT(cutFiles()), kCut->key() );
  cFModMenu->addAction(LXDG::findIcon("edit-copy",""), tr("Copy Selection"), this, SLOT(copyFiles()), kCopy->key() );
  cFModMenu->addSeparator();
  cFModMenu->addAction(LXDG::findIcon("edit-rename",""), tr("Rename..."), this, SLOT(renameFiles()), kRename->key() );
  cFModMenu->addSeparator();
  cFModMenu->addAction(LXDG::findIcon("edit-delete",""), tr("Delete Selection"), this, SLOT(removeFiles()), kDel->key() );
*/
  if(cFViewMenu==0){ cFViewMenu = new QMenu(this); }
  else{ cFViewMenu->clear(); }
  cFViewMenu->setTitle(tr("View Files..."));
  cFViewMenu->setIcon( LXDG::findIcon("document-preview","") );
  cFViewMenu->addAction(LXDG::findIcon("document-encrypted",""), tr("Checksums"), this, SLOT(fileCheckSums()) );
  if(LUtils::isValidBinary("lumina-fileinfo")){
    cFViewMenu->addAction(LXDG::findIcon("edit-find-replace",""), tr("Properties"), this, SLOT(fileProperties()) );
  }

}

BrowserWidget* DirWidget::currentBrowser(){
  if(cBID.isEmpty() || RCBW==0){ return BW; }
  else{ return RCBW; }
}

QStringList DirWidget::currentDirFiles(){
  return currentBrowser()->currentItems(-1);  //files only
}
// =================
//    PRIVATE SLOTS
// =================

//UI BUTTONS
void DirWidget::on_tool_zoom_in_clicked(){
  int size = BW->thumbnailSize();
  size += 16;
  setThumbnailSize(size);
  //Now Save the size value as the default for next time
  QSettings SET("lumina-desktop","lumina-fm");
  SET.setValue("iconsize", size);
}

void DirWidget::on_tool_zoom_out_clicked(){
  int size = BW->thumbnailSize();
  if(size <= 16){ return; }
  size -= 16;
  setThumbnailSize(size);
  //Now Save the size value as the default for next time
  QSettings SET("lumina-desktop","lumina-fm");
  SET.setValue("iconsize", size);
}

// -- Top Snapshot Buttons
void DirWidget::on_tool_snap_newer_clicked(){
  ui->slider_snap->setValue( ui->slider_snap->value()+1 );
}

void DirWidget::on_tool_snap_older_clicked(){
  ui->slider_snap->setValue( ui->slider_snap->value()-1 );
}

void DirWidget::on_slider_snap_valueChanged(int val){
  bool labelsonly = false;
  if(val==-1){ val = ui->slider_snap->value(); labelsonly=true; }
  //Update the snapshot interface
  ui->tool_snap_newer->setEnabled(val < ui->slider_snap->maximum());
  ui->tool_snap_older->setEnabled(val > ui->slider_snap->minimum());
  if(val >= snapshots.length() || val < 0){ 
    ui->tool_snap->setText(tr("Current"));
  }else if(QFile::exists(snapbasedir+snapshots[val])){
    ui->tool_snap->setText( QFileInfo(snapbasedir+snapshots[val]).lastModified().toString(Qt::DefaultLocaleShortDate) );
  }
  //Exit if a non-interactive snapshot change
  if(!ui->group_snaps->isEnabled() || labelsonly){ return; } //internal change - do not try to change the actual info
  //Determine which snapshot is now selected
  QString dir;
  if(DEBUG){ qDebug() << "Changing snapshot:" << currentBrowser()->currentDirectory() << val << snapbasedir; }
  //stopload = true; //stop any currently-loading procedures
  if(val >= snapshots.length() || val < 0){ //active system selected
    if(DEBUG){ qDebug() << " - Load Active system:" << normalbasedir; }
    dir = normalbasedir;
  }else{
    dir = snapbasedir+snapshots[val]+"/";
    if(!QFile::exists(dir)){
      //This snapshot must have been removed in the background by pruning tools
      //    move to a newer snapshot or the current base dir as necessary
      qDebug() << "Snapshot no longer available:" << dir;
      qDebug() << " - Reloading available snapshots";
      emit findSnaps(ID, normalbasedir);
      return;
    }
    //Need to figure out the relative path within the snapshot
      snaprelpath = normalbasedir.section(snapbasedir.section(ZSNAPDIR,0,0), 1,1000);
      if(DEBUG){ qDebug() << " - new snapshot-relative path:" << snaprelpath; }
    dir.append(snaprelpath);
    dir.replace("//","/"); //just in case any duplicate slashes from all the split/combining
    if(DEBUG){ qDebug() << " - Load Snapshot:" << dir; }
  }
  //Make sure this directory exists, and back up as necessary
  if(dir.isEmpty()){ return; }
  //Load the newly selected snapshot
  currentBrowser()->changeDirectory(dir);
}

void DirWidget::direct_snap_selected(QAction *act){
  QString snap = act->whatsThis();
  int val = snapshots.indexOf(snap);
  if(val<0){ return; }
  else{ ui->slider_snap->setValue(val); }
}

//Top Toolbar buttons
void DirWidget::on_actionBack_triggered(){
  QStringList history = currentBrowser()->history();
  if(history.length()<2){ return; } //cannot do anything
  QString dir = history.takeLast();
  //qDebug() << "Go Back:" << dir << normalbasedir << history;
  if(dir == normalbasedir){
    dir = history.takeLast();
  }
  history << dir; //make sure the current dir is always last in the history
  currentBrowser()->changeDirectory(dir);
  currentBrowser()->setHistory(history); //re-write the history to account for going backwards
  ui->actionBack->setEnabled(history.length()>1);
}

void DirWidget::on_actionUp_triggered(){
  QString dir = currentBrowser()->currentDirectory().section("/",0,-2);
  if(dir.isEmpty())
      dir = "/";
  //Quick check to ensure the directory exists
  while(!QFile::exists(dir) && !dir.isEmpty()){
    dir = dir.section("/",0,-2); //back up one additional dir
  }
  currentBrowser()->changeDirectory(dir);
}

void DirWidget::on_actionHome_triggered(){
  currentBrowser()->changeDirectory(QDir::homePath());
}

void DirWidget::dir_changed(){
  QString dir = line_dir->text().simplified();
  //Run the dir through the user-input checks
  dir = LUtils::PathToAbsolute(dir);
  //qDebug() << "Dir Changed:" << dir;
  //Quick check to ensure the directory exists
  while(!QFile::exists(dir) && !dir.isEmpty()){
    dir = dir.section("/",0,-2); //back up one additional dir
  }
  //qDebug() << " - Now Dir:" << dir;
  currentBrowser()->changeDirectory(dir);
  //emit LoadDirectory(ID, dir);
}


void DirWidget::on_actionSingleColumn_triggered(bool checked){
  if(!checked){ return; }
  ui->actionDualColumn->setChecked(false);
  if(RCBW==0){ return; } //nothing to do
  ui->browser_layout->removeWidget(RCBW);
  RCBW->deleteLater();
  RCBW = 0;
  setCurrentBrowser(""); //reset back to the remaining browser
}

void DirWidget::on_actionDualColumn_triggered(bool checked){
   if(!checked){ return; }
  ui->actionSingleColumn->setChecked(false);
  if(RCBW!=0){ return; } //nothing to do
  RCBW = new BrowserWidget("rc", this);
  ui->browser_layout->addWidget(RCBW);
  connect(RCBW, SIGNAL(dirChange(QString)), this, SLOT(currentDirectoryChanged()) );
  connect(RCBW, SIGNAL(itemsActivated()), this, SLOT(runFiles()) );
  connect(RCBW, SIGNAL(DataDropped(QString, QStringList)), this, SIGNAL(PasteFiles(QString, QStringList)) );
  connect(RCBW, SIGNAL(contextMenuRequested()), this, SLOT(OpenContextMenu()) );
  connect(RCBW, SIGNAL(updateDirectoryStatus(QString)), this, SLOT(dirStatusChanged(QString)) );
  connect(RCBW, SIGNAL(hasFocus(QString)), this, SLOT(setCurrentBrowser(QString)) );
  //Now make sure it has all the same settings as the main browser
  setCurrentBrowser("rc");
  RCBW->showDetails(BW->hasDetails());
  RCBW->showHiddenFiles( BW->hasHiddenFiles());
  RCBW->setThumbnailSize( BW->thumbnailSize());
  RCBW->changeDirectory( BW->currentDirectory());
}

void DirWidget::on_actionMenu_triggered(){
  OpenContextMenu();
}


// - Other Actions without a specific button on the side
void DirWidget::fileCheckSums(){
  QStringList files = currentBrowser()->currentSelection();
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
}

void DirWidget::fileProperties(){
  QStringList sel = currentBrowser()->currentSelection();
  //qDebug() << "Open File properties:" << sel;
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
  emit LaunchTerminal(currentBrowser()->currentDirectory());
}

//Browser Functions
void DirWidget::OpenContextMenu(){
  //Now open the menu at the current cursor location
  contextMenu->popup(QCursor::pos());
}

void DirWidget::UpdateContextMenu(){
  //First generate the context menu based on the selection
  //qDebug() << "Update context menu";
  QStringList sel = currentBrowser()->currentSelection();
  contextMenu->clear();

  if(!sel.isEmpty()){  
    contextMenu->addAction(LXDG::findIcon("run-build-file",""), tr("Open"), this, SLOT(runFiles()) );
    contextMenu->addAction(LXDG::findIcon("run-build-configure",""), tr("Open With..."), this, SLOT(runWithFiles()) );
  }
  contextMenu->addSection(LXDG::findIcon("unknown",""), tr("File Operations"));
 // contextMenu->addMenu(cFModMenu);
 //   cFModMenu->setEnabled(!sel.isEmpty() && canmodify);
  contextMenu->addMenu(cFViewMenu);
    contextMenu->setEnabled(!sel.isEmpty());
    contextMenu->addAction(LXDG::findIcon("edit-rename",""), tr("Rename..."), this, SLOT(renameFiles()), kRename->key() );
    contextMenu->addAction(LXDG::findIcon("edit-cut",""), tr("Cut Selection"), this, SLOT(cutFiles()), kCut->key() );
    contextMenu->addAction(LXDG::findIcon("edit-copy",""), tr("Copy Selection"), this, SLOT(copyFiles()), kCopy->key() );
    contextMenu->addAction(LXDG::findIcon("edit-paste",""), tr("Paste"), this, SLOT(pasteFiles()), QKeySequence(Qt::CTRL+Qt::Key_V) )->setEnabled(QApplication::clipboard()->mimeData()->hasFormat("x-special/lumina-copied-files") && canmodify);
    contextMenu->addSeparator();
    contextMenu->addAction(LXDG::findIcon("edit-delete",""), tr("Delete Selection"), this, SLOT(removeFiles()), kDel->key() );
    contextMenu->addSeparator();

  //Now add the general selection options
  contextMenu->addSection(LXDG::findIcon("folder","inode/directory"), tr("Directory Operations"));
  if(canmodify){
    contextMenu->addMenu(cNewMenu);
  }
  contextMenu->addMenu(cOpenMenu);
}

void DirWidget::currentDirectoryChanged(bool widgetonly){
  QString cur = currentBrowser()->currentDirectory();
  QFileInfo info(cur);
  canmodify = info.isWritable();
  if(widgetonly){ ui->label_status->setText(currentBrowser()->status()); }
  else if( !currentBrowser()->isEnabled() ){ ui->label_status->setText(tr("Loading...")); }
  //qDebug() << "Start search for snapshots";
  if(!cur.contains("/.zfs/snapshot") ){ 
    normalbasedir = cur;
    ui->group_snaps->setVisible(false);
    emit findSnaps(ID, cur); 
    qDebug() << "Changed to directory:" << cur;
  }else{
    //Re-assemble the normalbasedir variable (in case moving around within a snapshot)
    normalbasedir = cur;
    normalbasedir.replace( QRegExp("\\/\\.zfs\\/snapshot/([^/]+)\\/"), "/" );
    qDebug() << "Changed to snapshot:" << cur << normalbasedir;
  }
  ui->actionBack->setEnabled( currentBrowser()->history().length()>1 );
  line_dir->setText(normalbasedir);
  emit TabNameChanged(ID, normalbasedir.section("/",-1));
}

void DirWidget::dirStatusChanged(QString stat){
  if(!canmodify){ stat.prepend(tr("(Limited Access) ")); }
  ui->label_status->setText(stat);
}

void DirWidget::setCurrentBrowser(QString id){
  //qDebug() << "Set Current Browser:" << id;
  if(id==cBID){ return; } //no change
  cBID = id;
  currentDirectoryChanged(true); //update all the averarching widget elements (widget only)
  //Now adjust the frame/highlighting around the "active" browser
  if(RCBW==0){ BW->setShowActive(true); }
  else{
    BW->setShowActive( cBID.isEmpty() );
    RCBW->setShowActive( !cBID.isEmpty() );
  }
}

//Context Menu Functions
void DirWidget::createNewFile(){
 if(!canmodify){ return; } //cannot create anything here
  //Prompt for the new filename
  bool ok = false;
  QString newdocument = QInputDialog::getText(this, tr("New Document"), tr("Name:"), QLineEdit::Normal, "", \
        &ok, 0, Qt::ImhFormattedNumbersOnly | Qt::ImhUppercaseOnly | Qt::ImhLowercaseOnly);
  if(!ok || newdocument.isEmpty()){ return; }	
  //Create the empty file
  QString full = currentBrowser()->currentDirectory();
  if(!full.endsWith("/")){ full.append("/"); }
  //verify the new file does not already exist
  if(QFile::exists(full+newdocument)){
    QMessageBox::warning(this, tr("Invalid Name"), tr("A file or directory with that name already exists! Please pick a different name."));
    QTimer::singleShot(0,this, SLOT(createNewFile()) ); //repeat this function
    return;
  }
  QFile file(full+newdocument);
  if(file.open(QIODevice::ReadWrite)){
    //If successfully opened, it has created a blank file
    file.close();
  }else{
    QMessageBox::warning(this, tr("Error Creating Document"), tr("The document could not be created. Please ensure that you have the proper permissions."));	  
  }
}

void DirWidget::createNewDir(){
  if(!canmodify){ return; } //cannot create anything here
  //Prompt for the new dir name
  bool ok = false;
  QString newdir = QInputDialog::getText(this, tr("New Directory"), tr("Name:"), QLineEdit::Normal, "", \
		&ok, 0, Qt::ImhFormattedNumbersOnly | Qt::ImhUppercaseOnly | Qt::ImhLowercaseOnly);
  if(!ok || newdir.isEmpty()){ return; }
  //Now create the new dir
  QString full = currentBrowser()->currentDirectory();
  if(!full.endsWith("/")){ full.append("/"); }
  QDir dir(full); //open the current dir
  full.append(newdir); //append the new name to the current dir
  //Verify that the new dir does not already exist
  if(dir.exists(full)){
    QMessageBox::warning(this, tr("Invalid Name"), tr("A file or directory with that name already exists! Please pick a different name."));
    QTimer::singleShot(0,this, SLOT(createNewDir()) ); //repeat this function
  }else{
    if(!dir.mkdir(newdir) ){
      QMessageBox::warning(this, tr("Error Creating Directory"), tr("The directory could not be created. Please ensure that you have the proper permissions to modify the current directory."));
    }
  }
}

void DirWidget::createNewXDGEntry(){
  if(!canmodify){ return; } //cannot create anything here
  //Prompt for the new filename
  bool ok = false;
  QString newdocument = QInputDialog::getText(this, tr("New Document"), tr("Name:"), QLineEdit::Normal, "", \
        &ok, 0, Qt::ImhFormattedNumbersOnly | Qt::ImhUppercaseOnly | Qt::ImhLowercaseOnly);
  if(!ok || newdocument.isEmpty()){ return; }	
  if(!newdocument.endsWith(".desktop")){ newdocument.append(".desktop"); }
  //Create the empty file
  QString full = currentBrowser()->currentDirectory();
  if(!full.endsWith("/")){ full.append("/"); }
  //Verify the file does not already exist
  if(QFile::exists(full+newdocument)){
    QMessageBox::warning(this, tr("Invalid Name"), tr("A file or directory with that name already exists! Please pick a different name."));
    QTimer::singleShot(0,this, SLOT(createNewFile()) ); //repeat this function
    return;
  }
  QProcess::startDetached("lumina-fileinfo -application \""+full+newdocument+"\"");
}

/*void DirWidget::createNewSymlink{

}*/

// - Selected FILE operations
void DirWidget::cutFiles(){
  QStringList sel = currentBrowser()->currentSelection();
  if(sel.isEmpty() || !canmodify){ return; }
  emit CutFiles(sel);	
}

void DirWidget::copyFiles(){
  QStringList sel = currentBrowser()->currentSelection();
  if(sel.isEmpty()){ return; }
  emit CopyFiles(sel);	
}

void DirWidget::pasteFiles(){
  if( !canmodify ){ return; }
  emit PasteFiles(currentBrowser()->currentDirectory(), QStringList() );	
}

void DirWidget::renameFiles(){
  QStringList sel = currentBrowser()->currentSelection();
  if(sel.isEmpty() || !canmodify){ return; }
  qDebug() << "Deleting selected Items:" << sel;
  emit RenameFiles(sel);	
}

void DirWidget::favoriteFiles(){
  QStringList sel = currentBrowser()->currentSelection();
  if(sel.isEmpty()){ return; }
  emit FavoriteFiles(sel);	
}

void DirWidget::removeFiles(){
  QStringList sel = currentBrowser()->currentSelection();
  if(sel.isEmpty() || !canmodify){ return; }
  qDebug() << "Deleting selected Items:" << sel;
  emit RemoveFiles(sel);	
}

void DirWidget::runFiles(){
  QStringList sel = currentBrowser()->currentSelection();
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
    currentBrowser()->changeDirectory( dirs.takeFirst()); //load the first directory in this widget
  }
  if(!dirs.isEmpty()){
    emit OpenDirectories(dirs); //open the rest of the directories in other tabs
  }
}

void DirWidget::runWithFiles(){
  QStringList sel = currentBrowser()->currentSelection();
  if(sel.isEmpty()){ return; }
  QStringList dirs;
  for(int i=0; i<sel.length(); i++){ 
    if(QFileInfo(sel[i]).isDir()){
      dirs << sel[i];
    }else{
      QProcess::startDetached("lumina-open -select \""+sel[i]+"\"");
    }
  }	  
  if(!dirs.isEmpty()){
    emit OpenDirectories(dirs); //open the rest of the directories in other tabs
  }
}

/*void DirWidget::attachToNewEmail(){

}*/	

// - Context-specific operations
void DirWidget::openInSlideshow(){
  QStringList sel = currentBrowser()->currentSelection();
  if(sel.isEmpty()){ sel = currentDirFiles(); }
  //Now turn them all into a list and emit them
    LFileInfoList list;
    for(int i=0; i<sel.length(); i++){
      if(sel.endsWith(".desktop")){ continue; } //simplification to make sure we don't read any files which are not needed
      LFileInfo info(sel[i]);
      if( info.isImage() ){  list << info; } //add to the list
    }
  if(!list.isEmpty()){ emit ViewFiles(list); }
}

void DirWidget::openMultimedia(){
  QStringList sel = currentBrowser()->currentSelection();
  if(sel.isEmpty()){ sel = currentDirFiles(); }
  //Now turn them all into a list and emit them
    LFileInfoList list;
    for(int i=0; i<sel.length(); i++){
      if(sel.endsWith(".desktop")){ continue; } //simplification to make sure we don't read any files which are not needed
      LFileInfo info(sel[i]);
      if( info.isAVFile() ){  list << info; } //add to the list
    }
  if(!list.isEmpty()){ emit PlayFiles(list); }
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
