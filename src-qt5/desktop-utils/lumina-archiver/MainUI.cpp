//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "MainUI.h"
#include "ui_MainUI.h"

#include <QMessageBox>
#include <QFileDialog>
#include <LuminaXDG.h>
#include <LUtils.h>

#define TAREXT (
MainUI::MainUI() : QMainWindow(), ui(new Ui::MainUI){
  ui->setupUi(this);
  this->setWindowTitle(tr("Archive Manager"));
  BACKEND = new Backend(this);
  connect(BACKEND, SIGNAL(ProcessStarting()), this, SLOT(ProcStarting()) );
  connect(BACKEND, SIGNAL(ProcessFinished(bool, QString)), this, SLOT(ProcFinished(bool, QString)) );
  connect(BACKEND, SIGNAL(ProgressUpdate(int, QString)), this, SLOT(ProcUpdate(int, QString)) );

  //Add a spacer between toolbar items
  QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  ui->toolBar->insertWidget(ui->actionAdd_File, spacer);

  //Connect the UI signals/slots
  connect(ui->action_New, SIGNAL(triggered()), this, SLOT(NewArchive()) );
  connect(ui->action_Open, SIGNAL(triggered()), this, SLOT(OpenArchive()) );
  connect(ui->action_Quit, SIGNAL(triggered()), this, SLOT(close()) );
  connect(ui->actionAdd_File, SIGNAL(triggered()), this, SLOT(addFiles()) );
  connect(ui->actionRemove_File, SIGNAL(triggered()), this, SLOT(remFiles()) );
  connect(ui->actionExtract_All, SIGNAL(triggered()), this, SLOT(extractFiles()) );
  connect(ui->actionExtract_Sel, SIGNAL(triggered()), this, SLOT(extractSelection()) );
  connect(ui->actionAdd_Dirs, SIGNAL(triggered()), this, SLOT(addDirs()) );
  connect(ui->tree_contents, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(ViewFile(QTreeWidgetItem*)) );
  ui->progressBar->setVisible(false);
  ui->label_progress->setVisible(false);
  ui->label_progress_icon->setVisible(false);
  ui->actionAdd_File->setEnabled(false);
  ui->actionRemove_File->setEnabled(false);
  ui->actionExtract_All->setEnabled(false);
  ui->actionAdd_Dirs->setEnabled(false);
  loadIcons();
  ui->tree_contents->setHeaderLabels( QStringList() << tr("File") << tr("MimeType") << tr("Size")+" " );
}

MainUI::~MainUI(){

}

void MainUI::LoadArguments(QStringList args){
  for(int i=0; i<args.length(); i++){
    if(QFile::exists(args[i])){ 
      ui->label_progress->setText(tr("Opening Archive..."));
      BACKEND->loadFile(args[i]);  
      break;
    }
  }
}

void MainUI::loadIcons(){
  this->setWindowIcon( LXDG::findIcon("utilities-file-archiver","") );
  ui->action_New->setIcon( LXDG::findIcon("file-new", "document-new") );
  ui->action_Open->setIcon( LXDG::findIcon("file-open","document-open") );
  ui->action_Quit->setIcon( LXDG::findIcon("application-exit", "dialog-close") );
  ui->actionAdd_File->setIcon( LXDG::findIcon("archive-insert", "") );
  ui->actionAdd_Dirs->setIcon( LXDG::findIcon("archive-insert-directory","") );
  ui->actionRemove_File->setIcon( LXDG::findIcon("archive-remove","") );
  ui->actionExtract_All->setIcon( LXDG::findIcon("archive-extract","") );
  ui->actionExtract_Sel->setIcon( LXDG::findIcon("archive-extract","") );
}

//===================
//     PRIVATE
//===================
QTreeWidgetItem* MainUI::findItem(QString path, QTreeWidgetItem *start){
  if(start==0){
    for(int i=0; i<ui->tree_contents->topLevelItemCount(); i++){
      if(ui->tree_contents->topLevelItem(i)->whatsThis(0) == path){ return ui->tree_contents->topLevelItem(i); }
      else if(path.startsWith(ui->tree_contents->topLevelItem(i)->whatsThis(0)+"/")){ return findItem(path, ui->tree_contents->topLevelItem(i)); }
    }
  }else{
    for(int i=0; i<start->childCount(); i++){
      if(start->child(i)->whatsThis(0) == path){ return start->child(i); }
      else if(path.startsWith(start->child(i)->whatsThis(0)+"/")){ return findItem(path, start->child(i)); }      
    }
  }
  //qDebug() << "Could not find item:" << path;
  return 0; //nothing found
}

bool MainUI::cleanItems(QStringList list, QTreeWidgetItem *start){
  bool changed = false;
  if(start==0){
    for(int i=0; i<ui->tree_contents->topLevelItemCount(); i++){
      changed = changed || cleanItems(list, ui->tree_contents->topLevelItem(i));
      if(!list.contains(ui->tree_contents->topLevelItem(i)->whatsThis(0)) ){
        delete ui->tree_contents->topLevelItem(i);
        changed = true;
      }
    }
  }else{
    for(int i=0; i<start->childCount(); i++){
      changed = changed || cleanItems(list, start->child(i));
      if(!list.contains(start->child(i)->whatsThis(0)) ){
        delete start->child(i);
        changed = true;
      }
    }
  }
  return changed;
}

//Functions for setting the valid file extensions ("tar" limitations)
QString MainUI::CreateFileTypes(){
  QStringList types;
  types << QString(tr("All Types %1")).arg("(*.tar.gz *.tar.xz *.tar.bz *.tar.bz2 *.tar.lzma *.tar *.zip *.tgz *.txz *.tbz *.tbz2 *.tlz *.cpio *.pax *.ar *.shar *.7z)");
  types << tr("Uncompressed Archive (*.tar)");
  types << tr("GZip Compressed Archive (*.tar.gz *.tgz)");
  types << tr("BZip Compressed Archive (*.tar.bz *.tbz)");
  types << tr("BZip2 Compressed Archive (*.tar.bz2 *.tbz2)");
  types << tr("LMZA Compressed Archive (*.tar.lzma *.tlz)");
  types << tr("XZ Compressed Archive (*.tar.xz *.txz)");
  types << tr("CPIO Archive (*.cpio)");
  types << tr("PAX Archive (*.pax)");
  types << tr("AR Archive (*.ar)");
  types << tr("SHAR Archive (*.shar)");
  types << tr("Zip Archive (*.zip)");
  types << tr("7-Zip Archive (*.7z)");
  return types.join(";;");
}

QString MainUI::OpenFileTypes(){
  QStringList types;
  types << QString(tr("All Types %1")).arg("(*.tar.gz *.tar.xz *.tar.bz *.tar.bz2 *.tar.lzma *.tar *.zip *.tgz *.txz *.tbz *.tbz2 *.tlz *.cpio *.pax *.ar *.shar *.7z *.iso *.xar *.jar *.rpm)");
  types << tr("Uncompressed Archive (*.tar)");
  types << tr("GZip Compressed Archive (*.tar.gz *.tgz)");
  types << tr("BZip Compressed Archive (*.tar.bz *.tbz)");
  types << tr("BZip2 Compressed Archive (*.tar.bz2 *.tbz2)");
  types << tr("XZ Compressed Archive (*.tar.xz *.txz)");
  types << tr("LMZA Compressed Archive (*.tar.lzma *.tlz)");
  types << tr("CPIO Archive (*.cpio)");
  types << tr("PAX Archive (*.pax)");
  types << tr("AR Archive (*.ar)");
  types << tr("SHAR Archive (*.shar)");
  types << tr("Zip Archive (*.zip)");
  types << tr("7-Zip Archive (*.7z)");
  types << tr("READ-ONLY: ISO image (*.iso)");
  types << tr("READ-ONLY: XAR archive (*.xar)");
  types << tr("READ-ONLY: Java archive (*.jar)");
  types << tr("READ-ONLY: RedHat Package (*.rpm)");
  return types.join(";;");
}

//===================
//     PRIVATE SLOTS
//===================
void MainUI::NewArchive(){
  QString file = QFileDialog::getSaveFileName(this, tr("Create Archive"), QDir::homePath(), CreateFileTypes() );
  if(file.isEmpty()){ return; }
  if(QFile::exists(file)){ 
    if( !QFile::remove(file) ){ QMessageBox::warning(this, tr("Error"), QString(tr("Could not overwrite file:"))+"\n"+file); } 
  }
  ui->label_progress->setText(""); //just clear it (this action is instant)
  BACKEND->loadFile(file);
}

void MainUI::OpenArchive(){
  QString file = QFileDialog::getOpenFileName(this, tr("Open Archive"), QDir::homePath(), OpenFileTypes() );
  if(file.isEmpty()){ return; }
  ui->label_progress->setText(tr("Opening Archive..."));
  BACKEND->loadFile(file);
}

void MainUI::addFiles(){
  QStringList files = QFileDialog::getOpenFileNames(this, tr("Add to Archive"), QDir::homePath() );
  if(files.isEmpty()){ return; }
  ui->label_progress->setText(tr("Adding Items..."));
  BACKEND->startAdd(files);
}

void MainUI::addDirs(){
  QString dirs = QFileDialog::getExistingDirectory(this, tr("Add to Archive"), QDir::homePath() );
  if(dirs.isEmpty()){ return; }
  ui->label_progress->setText(tr("Adding Items..."));
  BACKEND->startAdd(QStringList() << dirs);
}

void MainUI::remFiles(){
  QList<QTreeWidgetItem*> sel = ui->tree_contents->selectedItems();
  QStringList items;
  for(int i=0; i<sel.length(); i++){
    items << sel[i]->whatsThis(0);
  }
  items.removeDuplicates();
  ui->label_progress->setText(tr("Removing Items..."));
  BACKEND->startRemove(items);
}

void MainUI::extractFiles(){
  QString dir = QFileDialog::getExistingDirectory(this, tr("Extract Into Directory"), QDir::homePath() );
  if(dir.isEmpty()){ return; }
  ui->label_progress->setText(tr("Extracting..."));
  BACKEND->startExtract(dir, true);
}

void MainUI::extractSelection(){
  if(ui->tree_contents->currentItem()==0){ return; } //nothing selected
  QList<QTreeWidgetItem*> sel = ui->tree_contents->selectedItems();
  if(sel.isEmpty()){ sel << ui->tree_contents->currentItem(); }
  QStringList selList;
  for(int i=0; i<sel.length(); i++){ selList << sel[i]->whatsThis(0); }
  selList.removeDuplicates();
  QString dir = QFileDialog::getExistingDirectory(this, tr("Extract Into Directory"), QDir::homePath() );
  if(dir.isEmpty()){ return; }
  ui->label_progress->setText(tr("Extracting..."));
  BACKEND->startExtract(dir, true, selList);
}

void MainUI::ViewFile(QTreeWidgetItem *it){
  if(it->childCount()>0){ return; } //directory - not viewable
  ui->label_progress->setText(tr("Extracting..."));
  BACKEND->startViewFile(it->whatsThis(0));
}

void MainUI::UpdateTree(){
  this->setEnabled(false);
  ui->tree_contents->setHeaderLabels( QStringList() << tr("File") << tr("MimeType") << tr("Size")+" " );
  QStringList files = BACKEND->heirarchy();
  files.sort();
  //Remove any entries for file no longer in the archive
  bool changed = cleanItems(files);
  //qDebug() << "Found Files:" << files;
  for(int i=0; i<files.length(); i++){
    if(0 != findItem(files[i]) ){ continue; } //already in the tree widget
    QString mime = LXDG::findAppMimeForFile(files[i].section("/",-1), false); //first match only
    QTreeWidgetItem *it = new QTreeWidgetItem();
      it->setText(0, files[i].section("/",-1) );
      if(!BACKEND->isLink(files[i])){
        it->setText(1, LXDG::findAppMimeForFile(files[i].section("/",-1), false) );
        it->setText(2, LUtils::BytesToDisplaySize( BACKEND->size(files[i])) );
      }else{
        it->setText(1, QString(tr("Link To: %1")).arg(BACKEND->linkTo(files[i]) ) );
      }
    it->setWhatsThis(0, files[i]);
    if(BACKEND->isDir(files[i])){
      it->setIcon(0, LXDG::findIcon("folder",""));
      it->setText(1,""); //clear the mimetype
    }else if(BACKEND->isLink(files[i])){
      it->setIcon(0, LXDG::findIcon("emblem-symbolic-link","") );
    }else{
      it->setIcon(0, LXDG::findMimeIcon(files[i].section("/",-1)) );
    }
     //Now find which item to add this too
    if(files[i].contains("/")){
      QTreeWidgetItem *parent = findItem(files[i].section("/",0,-2));
      QList<QTreeWidgetItem*> list = ui->tree_contents->findItems(files[i].section("/",-3,-2), Qt::MatchExactly, 0);
      if(parent==0){ ui->tree_contents->addTopLevelItem(it); }
      else{ parent->addChild(it); }
    }else{
      ui->tree_contents->addTopLevelItem(it);
      QApplication::processEvents();
    }
    changed = true;
  }

  if(changed){
    int wid = ui->tree_contents->fontMetrics().width("W")*5;
    ui->tree_contents->setColumnWidth(2, wid);
    for(int i=1; i<2; i++){ui->tree_contents->resizeColumnToContents(i); QApplication::processEvents(); wid+= ui->tree_contents->columnWidth(i); }
    //qDebug() << "Set column 0 width:" << wid << ui->tree_contents->viewport()->width();
    ui->tree_contents->setColumnWidth(0, ui->tree_contents->viewport()->width()-wid);
  }
  ui->tree_contents->sortItems(0, Qt::AscendingOrder); //sort by name
  ui->tree_contents->sortItems(1,Qt::AscendingOrder); //sort by mimetype (put dirs first - still organized by name)

  this->setEnabled(true);
  ui->tree_contents->setEnabled(true);
}

//Backend Handling
void MainUI::ProcStarting(){
  ui->progressBar->setRange(0,0);
  ui->progressBar->setValue(0);
  ui->progressBar->setVisible(true);
  ui->label_progress->setVisible(!ui->label_progress->text().isEmpty());
  ui->label_progress_icon->setVisible(false);
  ui->tree_contents->setEnabled(false);
  ui->label_archive->setText(BACKEND->currentFile());
}

void MainUI::ProcFinished(bool success, QString msg){
  UpdateTree();
  ui->progressBar->setRange(0,0);
  ui->progressBar->setValue(0);
  ui->progressBar->setVisible(false);
  ui->label_progress->setText(msg);
  ui->label_progress->setVisible(!msg.isEmpty());
  ui->label_progress_icon->setVisible(!msg.isEmpty());
  if(success){ ui->label_progress_icon->setPixmap( LXDG::findIcon("task-complete","").pixmap(32,32) );}
  else{ ui->label_progress_icon->setPixmap( LXDG::findIcon("task-attention","").pixmap(32,32) );}
  if(ui->label_archive->text()!=BACKEND->currentFile()){
    ui->label_archive->setText(BACKEND->currentFile());
    this->setWindowTitle(BACKEND->currentFile().section("/",-1));
    ui->tree_contents->clear();
  }
  QFileInfo info(BACKEND->currentFile());
    bool canmodify = info.isWritable();
    if(!info.exists()){ canmodify = QFileInfo(BACKEND->currentFile().section("/",0,-2)).isWritable(); }
    canmodify = canmodify && BACKEND->canModify(); //also include the file type limitations
    ui->actionAdd_File->setEnabled(canmodify);
    ui->actionRemove_File->setEnabled(canmodify && info.exists());
    ui->actionExtract_All->setEnabled(info.exists());
    ui->actionAdd_Dirs->setEnabled(canmodify);
}

void MainUI::ProcUpdate(int percent, QString txt){
  ui->progressBar->setMaximum( percent<0 ? 0 : 100 );
  ui->progressBar->setValue(percent);
  if(!txt.isEmpty()){ ui->label_progress->setText(txt); }
}
