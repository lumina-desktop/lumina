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
  connect(BACKEND, SIGNAL(ProcessFinished()), this, SLOT(ProcFinished()) );
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
  connect(ui->actionAdd_Dirs, SIGNAL(triggered()), this, SLOT(addDirs()) );
  ui->progressBar->setVisible(false);
  ui->label_progress->setVisible(false);
  ui->actionAdd_File->setEnabled(false);
  ui->actionRemove_File->setEnabled(false);
  ui->actionExtract_All->setEnabled(false);
  ui->actionAdd_Dirs->setEnabled(false);
  loadIcons();
  ui->tree_contents->setHeaderLabels( QStringList() << tr("File") << tr("MimeType") << tr("Size") << tr("Compressed")+" " );
}

MainUI::~MainUI(){

}

void MainUI::LoadArguments(QStringList args){
  for(int i=0; i<args.length(); i++){
    if(QFile::exists(args[i])){ BACKEND->loadFile(args[i]);  break;}
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
}

//===================
//     PRIVATE
//===================
QTreeWidgetItem* MainUI::findItem(QString path, QTreeWidgetItem *start){
  if(start==0){
    for(int i=0; i<ui->tree_contents->topLevelItemCount(); i++){
      QString itpath = ui->tree_contents->topLevelItem(i)->whatsThis(0);
      if(itpath == path){ return ui->tree_contents->topLevelItem(i); }
      else if(path.startsWith(itpath+"/")){ return findItem(path, ui->tree_contents->topLevelItem(i)); }
    }
  }else{
    for(int i=0; i<start->childCount(); i++){
      QString itpath = start->child(i)->whatsThis(0);
      if(itpath == path){ return start->child(i); }
      else if(path.startsWith(itpath+"/")){ return findItem(path, start->child(i)); }      
    }
  }
  return 0; //nothing found
}

void MainUI::cleanItems(QStringList list, QTreeWidgetItem *start){
  if(start==0){
    for(int i=0; i<ui->tree_contents->topLevelItemCount(); i++){
      cleanItems(list, ui->tree_contents->topLevelItem(i));
      if(!list.contains(ui->tree_contents->topLevelItem(i)->whatsThis(0)) ){
        delete ui->tree_contents->topLevelItem(i);
      }
    }
  }else{
    for(int i=0; i<start->childCount(); i++){
      cleanItems(list, start->child(i));
      if(!list.contains(start->child(i)->whatsThis(0)) ){
        delete start->child(i);
      }
    }
  }
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
  BACKEND->loadFile(file);
}

void MainUI::OpenArchive(){
  QString file = QFileDialog::getOpenFileName(this, tr("Open Archive"), QDir::homePath(), OpenFileTypes() );
  if(file.isEmpty()){ return; }
  BACKEND->loadFile(file);
}

void MainUI::addFiles(){
  QStringList files = QFileDialog::getOpenFileNames(this, tr("Add to Archive"), QDir::homePath() );
  if(files.isEmpty()){ return; }
  BACKEND->startAdd(files);
}

void MainUI::addDirs(){
  QString dirs = QFileDialog::getExistingDirectory(this, tr("Add to Archive"), QDir::homePath() );
  if(dirs.isEmpty()){ return; }
  BACKEND->startAdd(QStringList() << dirs);
}

void MainUI::remFiles(){
  QList<QTreeWidgetItem*> sel = ui->tree_contents->selectedItems();
  QStringList items;
  for(int i=0; i<sel.length(); i++){
    items << sel[i]->whatsThis(0);
  }
  items.removeDuplicates();
  BACKEND->startRemove(items);
}

void MainUI::extractFiles(){
  QString dir = QFileDialog::getExistingDirectory(this, tr("Extract Into Directory"), QDir::homePath() );
  if(dir.isEmpty()){ return; }
  BACKEND->startExtract(dir, true);
}

void MainUI::UpdateTree(){
  ui->tree_contents->setHeaderLabels( QStringList() << tr("File") << tr("MimeType") << tr("Size") << tr("Compressed")+" " );
  QStringList files = BACKEND->heirarchy();
  files.sort();
  //Remove any entries for file no longer in the archive
  cleanItems(files);
  qDebug() << "Found Files:" << files;
  for(int i=0; i<files.length(); i++){
    if(0 != findItem(files[i]) ){ continue; } //already in the tree widget
    QString mime = LXDG::findAppMimeForFile(files[i].section("/",-1), false); //first match only
    QTreeWidgetItem *it = new QTreeWidgetItem( QStringList() << files[i].section("/",-1) << mime << LUtils::BytesToDisplaySize( BACKEND->size(files[i])) << LUtils::BytesToDisplaySize(BACKEND->csize(files[i])) );
    it->setWhatsThis(0, files[i]);
    if(BACKEND->isDir(files[i])){
      it->setIcon(0, LXDG::findIcon("folder",""));
      it->setText(1,""); //clear the mimetype
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
    }
  }
  int wid = 0;
  for(int i=3; i>0; i--){ui->tree_contents->resizeColumnToContents(i); wid+= ui->tree_contents->columnWidth(i); }
  ui->tree_contents->setColumnWidth(0, ui->tree_contents->viewport()->width()-wid);
}

//Backend Handling
void MainUI::ProcStarting(){
  ui->progressBar->setRange(0,0);
  ui->progressBar->setValue(0);
  ui->progressBar->setVisible(true);
  ui->label_progress->setVisible(true);
  ui->tree_contents->setEnabled(false);
  ui->label_archive->setText(BACKEND->currentFile());
}

void MainUI::ProcFinished(){
  ui->progressBar->setRange(0,0);
  ui->progressBar->setValue(0);
  ui->progressBar->setVisible(false);
  ui->label_progress->setVisible(false);
  ui->tree_contents->setEnabled(true);
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
  UpdateTree();
}

void MainUI::ProcUpdate(int percent, QString txt){
  ui->progressBar->setMaximum( percent<0 ? 0 : 100 );
  ui->progressBar->setValue(percent);
  if(!txt.isEmpty()){ ui->label_progress->setText(txt); }
}
