//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include <QtConcurrent/QtConcurrentRun>
#include "MainUI.h"
#include "ui_MainUI.h"

#include <QVideoFrame>
#include <QFileDialog>
#include <QMessageBox>

#include <LUtils.h>
#include <LuminaOS.h>

MainUI::MainUI() : QMainWindow(), ui(new Ui::MainUI){
  ui->setupUi(this); //load the designer form
  canwrite = false;
  terminate_thread = false;
  INFO = new LFileInfo();
  UpdateIcons(); //Set all the icons in the dialog
  SetupConnections();

  //Disable buttons that are not working yet
  ui->actionOpen_File->setVisible(false);
  ui->actionOpen_Directory->setVisible(false);
  ui->menuSave_As->setEnabled(false);
}

MainUI::~MainUI(){
  terminate_thread = true;
  this->close();
}

//=============
//      PUBLIC
//=============
void MainUI::LoadFile(QString path, QString type){

  //Do the first file information tab
  qDebug() << "Load File:" << path << type;
  INFO = new LFileInfo(path);
  //First load the general file information
  if(!INFO->filePath().isEmpty()){
    SyncFileInfo();
  }else{
    SetupNewFile();
  }
}

void MainUI::UpdateIcons(){

}

//==============
//     PRIVATE
//==============
void MainUI::ReloadAppIcon(){
  //qDebug() << "Reload App Icon:";
  ui->label_xdg_icon->setPixmap( LXDG::findIcon(ui->line_xdg_icon->text(),"").pixmap(64,64) );
  //qDebug() << "Check Desktop File entry";
  if(INFO->iconfile()!=ui->line_xdg_icon->text()){
    xdgvaluechanged();
  }
  //qDebug() << "Done with app icon";
}

void MainUI::GetDirSize(const QString dirname) const {
  const quint16 update_frequency = 2000; //For reducing the number of folder_size_changed calls
  quint64 filesize = 0;
  quint64 file_number = 0;
  quint64 dir_number = 1;
  QDir folder(dirname);
  QFileInfoList file_list;
  QString dir_name;
  QList<QString> head;
  folder.setFilter(QDir::Hidden|QDir::AllEntries|QDir::NoDotAndDotDot);
  file_list = folder.entryInfoList();
  for(int i=0; i<file_list.size(); ++i) {
    if(terminate_thread)
        break;
    if(file_list[i].isDir() && !file_list[i].isSymLink()) {
      ++dir_number;
      head.prepend(file_list[i].absoluteFilePath());
    }
    else
      ++file_number;
    if(!file_list[i].isSymLink())
      filesize += file_list[i].size();
  }
  while(!head.isEmpty()) {
    if(terminate_thread)
      break;
    dir_name = head.takeFirst();
    if(!folder.cd(dir_name)) {
      qDebug() << "The folder " << dir_name << " doesn't exist";
      continue;
    }
    file_list = folder.entryInfoList();
    for(int i=0; i<file_list.size(); ++i) {
      if(file_list[i].isDir() && !file_list[i].isSymLink()) {
        ++dir_number;
        head.prepend(file_list[i].absoluteFilePath());
      }
      else
        ++file_number;
      if(!file_list[i].isSymLink())
        filesize += file_list[i].size();
      if(i%update_frequency == 0)
        emit folder_size_changed(filesize, file_number, dir_number, false);
    }
  }
  emit folder_size_changed(filesize, file_number, dir_number, true);
}

void MainUI::SyncFileInfo(){
  qDebug() << "Sync File Info";
  if(INFO->filePath().isEmpty()){ return; }
  if(INFO->exists()){ canwrite = INFO->isWritable(); }
  else{
    //See if the containing directory can be written
    QFileInfo chk(INFO->absolutePath());
    canwrite = (chk.isDir() && chk.isWritable());
  }
    ui->label_file_name->setText( INFO->fileName() );
    ui->label_file_mimetype->setText( INFO->mimetype() );
    if(!INFO->isDir()){ ui->label_file_size->setText( LUtils::BytesToDisplaySize( INFO->size() ) ); }
    else {
      ui->label_file_size->setText(tr("---Calculating---"));
      QtConcurrent::run(this, &MainUI::GetDirSize, INFO->absoluteFilePath());
    }
    ui->label_file_owner->setText(INFO->owner());
    ui->label_file_group->setText(INFO->group());
    ui->label_file_created->setText( INFO->created().toString(Qt::SystemLocaleLongDate) );
    ui->label_file_modified->setText( INFO->lastModified().toString(Qt::SystemLocaleLongDate) );
    //Get the file permissions
    QString perms;
    if(INFO->isReadable() && INFO->isWritable()){ perms = tr("Read/Write"); }
    else if(INFO->isReadable()){ perms = tr("Read Only"); }
    else if(INFO->isWritable()){ perms = tr("Write Only"); }
    else{ perms = tr("No Access"); }
    ui->label_file_perms->setText(perms);
    //Now the special "type" for the file
    QString ftype;
    if(INFO->suffix().toLower()=="desktop"){ ftype = tr("XDG Shortcut"); }
    else if(INFO->isDir()){ ftype = tr("Directory"); }
    else if(INFO->isExecutable()){ ftype = tr("Binary"); }
    else{ ftype = INFO->suffix().toUpper(); }
    if(INFO->isHidden()){ ftype = QString(tr("Hidden %1")).arg(ftype); }
    ui->label_file_type->setText(ftype);

    //Now load the icon for the file
    if(INFO->isImage()){
      QPixmap pix(INFO->absoluteFilePath());
      ui->label_file_icon->setPixmap(pix.scaledToHeight(64));
      ui->label_file_size->setText( ui->label_file_size->text()+" ("+QString::number(pix.width())+" x "+QString::number(pix.height())+" px)" );
    }else if(INFO->isVideo()){
      ui->label_file_icon->hide();
      LVideoLabel *mediaLabel = new LVideoLabel(INFO->absoluteFilePath(), true, ui->tab_file);
      mediaLabel->setFixedSize(64,64);
      ui->formLayout->replaceWidget(ui->label_file_icon, mediaLabel);
    }else{
      ui->label_file_icon->setPixmap( LXDG::findIcon( INFO->iconfile(), "unknown").pixmap(QSize(64,64)) );
    }

  //qDebug() << "Check XDG Info:"
  //qDebug() << INFO->isDesktopFile() << type;
  syncXdgStruct(INFO->XDG());
  //Make sure the right tabs are available
  if(ui->tabWidget->indexOf(ui->tab_file)<0){
    //qDebug() << "Add File Info Tab";
    ui->tabWidget->addTab(ui->tab_file, tr("File Information"));
  }
  if(!INFO->isDesktopFile()){
    if(ui->tabWidget->indexOf(ui->tab_deskedit)>=0){
      ui->tabWidget->removeTab( ui->tabWidget->indexOf(ui->tab_deskedit) );
    }
  }
  ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->tab_file) );
}

void MainUI::SetupNewFile(){
  //qDebug() << "Setup New File";
  if(!INFO->filePath().isEmpty()){
    INFO = new LFileInfo();
  }
  terminate_thread = true; //just in case
  canwrite = true; //can always write a new file
  syncXdgStruct(INFO->XDG());
  //Make sure the right tabs are enabled
  if(ui->tabWidget->indexOf(ui->tab_file)>=0){
    ui->tabWidget->removeTab( ui->tabWidget->indexOf(ui->tab_file) );
  }
  if(ui->tabWidget->indexOf(ui->tab_deskedit)<0){
    //qDebug() << "Adding the deskedit tab";
    ui->tabWidget->addTab(ui->tab_deskedit, tr("XDG Shortcut"));
  }
  ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->tab_deskedit) );
}

void MainUI::syncXdgStruct(XDGDesktop *XDG){
  bool cleanup = false;
  if(XDG==0){ XDG = new XDGDesktop(); cleanup = true;} //make sure nothing crashes
  if(XDG->type == XDGDesktop::APP){
      ui->line_xdg_command->setText(XDG->exec);
      ui->line_xdg_wdir->setText(XDG->path);
      ui->check_xdg_useTerminal->setChecked( XDG->useTerminal );
      ui->check_xdg_startupNotify->setChecked( XDG->startupNotify );
    }else if(XDG->type==XDGDesktop::LINK){
      //Hide the options that are unavailable for links
      //Command  line (exec)
      ui->line_xdg_command->setVisible(false);
      ui->tool_xdg_getCommand->setVisible(false);
      ui->lblCommand->setVisible(false);
      //Options
      ui->lblOptions->setVisible(false);
      ui->check_xdg_useTerminal->setVisible(false);
      ui->check_xdg_startupNotify->setVisible(false);
      //Now load the variables for this type of shortcut
      ui->lblWorkingDir->setText(tr("URL:"));
      ui->line_xdg_wdir->setText( XDG->url );
      ui->tool_xdg_getDir->setVisible(false); //the dir selection button
    }
    ui->line_xdg_name->setText(XDG->name);
    ui->line_xdg_comment->setText(XDG->comment);
    ui->line_xdg_icon->setText( XDG->icon );
    ReloadAppIcon();
    ui->actionSave_Shortcut->setVisible(true);
    ui->actionSave_Shortcut->setEnabled(false);
  if(cleanup){ delete XDG; }
}

bool MainUI::saveFile(QString path){
  //qDebug() << "Request save file:" << path;
  XDGDesktop *XDG = INFO->XDG();
  if(XDG==0){ XDG = new XDGDesktop(); }
  if(XDG->type == XDGDesktop::BAD){ XDG->type = XDGDesktop::APP; }
  //Update the file path in the data structure
  XDG->filePath = path;
  //Now change the structure
  XDG->name = ui->line_xdg_name->text();
  XDG->genericName = ui->line_xdg_name->text().toLower();
  XDG->comment = ui->line_xdg_comment->text();
  XDG->icon = ui->line_xdg_icon->text();
  //Now do the type-specific fields
  if(XDG->type == XDGDesktop::APP){
    XDG->exec = ui->line_xdg_command->text();
    XDG->tryexec = ui->line_xdg_command->text().section(" ",0,0); //use the first word/binary for the existance check
    XDG->path = ui->line_xdg_wdir->text(); //working dir/path
    XDG->useTerminal = ui->check_xdg_useTerminal->isChecked();
    XDG->startupNotify = ui->check_xdg_startupNotify->isChecked();
  }else if(XDG->type==XDGDesktop::LINK){
    XDG->url = ui->line_xdg_wdir->text(); //we re-used this field
  }
  //Clear any info which this utility does not support at the moment
  XDG->actionList.clear();
  XDG->actions.clear();
  //Now save the structure to file
  //qDebug() << "Saving File:" << XDG->filePath;
  return XDG->saveDesktopFile(true); //Try to merge the file/structure as necessary
}

// Initialization procedures
void MainUI::SetupConnections(){
  connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(closeApplication()) );
  connect(ui->actionSave_Shortcut, SIGNAL(triggered()), this, SLOT(save_clicked()) );
  connect(ui->actionNew_Shortcut, SIGNAL(triggered()), this, SLOT(SetupNewFile()) );
  connect(ui->actionOpen_File, SIGNAL(triggered()), this, SLOT(open_file()) );
  connect(ui->actionOpen_Directory, SIGNAL(triggered()), this, SLOT(open_dir()) );
  connect(ui->line_xdg_command, SIGNAL(editingFinished()), this, SLOT(xdgvaluechanged()) );
  connect(ui->line_xdg_comment, SIGNAL(editingFinished()), this, SLOT(xdgvaluechanged()) );
  connect(ui->line_xdg_icon, SIGNAL(textChanged(QString)), this, SLOT(ReloadAppIcon()) );
  connect(ui->tool_xdg_getCommand, SIGNAL(clicked()), this, SLOT(getXdgCommand()) );
  connect(ui->line_xdg_name, SIGNAL(editingFinished()), this, SLOT(xdgvaluechanged()) );
  connect(ui->line_xdg_wdir, SIGNAL(editingFinished()), this, SLOT(xdgvaluechanged()) );
  connect(ui->check_xdg_useTerminal, SIGNAL(clicked()), this, SLOT(xdgvaluechanged()) );
  connect(ui->check_xdg_startupNotify, SIGNAL(clicked()), this, SLOT(xdgvaluechanged()) );
  connect(this, SIGNAL(folder_size_changed(quint64, quint64, quint64, bool)), this, SLOT(refresh_folder_size(quint64, quint64, quint64, bool)));
}

//UI Buttons
void MainUI::closeApplication(){
  terminate_thread = true;
  if(ui->actionSave_Shortcut->isEnabled()){
    //Still have unsaved changes
    //TO-DO - prompt for whether to save the changes
  }
  this->close();
}

void MainUI::save_clicked(){
  //Save all the xdg values into the structure
  QString filePath = INFO->filePath();
  if( !filePath.isEmpty() && !INFO->isDesktopFile() ){ return; }
  if(filePath.isEmpty() || !canwrite){
    //Need to prompt for where to save the file and what to call it
    QString appdir = QString(getenv("XDG_DATA_HOME"))+"/applications/";
    if(!QFile::exists(appdir)){ QDir dir; dir.mkpath(appdir); }
    filePath = QFileDialog::getSaveFileName(this, tr("Save Application File"), appdir, tr("XDG Shortcuts (*.desktop)") );
    if(filePath.isEmpty()){ return; }
    if(!filePath.endsWith(".desktop")){ filePath.append(".desktop"); }
  }
  //qDebug() << " -Try Saving File:" << filePath;
  bool saved = saveFile(filePath);
  //qDebug() << "File Saved:" << saved;
  ui->actionSave_Shortcut->setEnabled( !saved );
  if(saved){
    //Re-load the file info
    LoadFile(filePath);
  }
}

void MainUI::getXdgCommand(QString prev){
  //Find a binary to run
  QString dir = prev; //start with the previous attempt (if there was one)
  if(dir.isEmpty()){ ui->line_xdg_command->text(); }//then try current selection
  if(dir.isEmpty()){ dir = LOS::AppPrefix()+"bin"; } //then open the application binaries directory
  QString file = QFileDialog::getOpenFileName(this, tr("Select a binary"), dir );
  if(file.isEmpty()){ return; } //cancelled
  if(!LUtils::isValidBinary(file)){
    QMessageBox::warning(this, tr("Error"), tr("Invalid selection: Not a valid executable"));
    getXdgCommand(file);
    return;
  }
  ui->line_xdg_command->setText(file);
  xdgvaluechanged();
}

void MainUI::on_tool_xdg_getDir_clicked(){
  //Find a directory
  QString dir = ui->line_xdg_wdir->text();
  if(dir.isEmpty()){ dir = QDir::homePath(); }
  dir = QFileDialog::getExistingDirectory(this, tr("Select a directory"), dir);
  if(dir.isEmpty()){ return; } //cancelled
  ui->line_xdg_wdir->setText(dir);
  xdgvaluechanged();
}

void MainUI::on_push_xdg_getIcon_clicked(){
  //Find an image file
  QString dir = ui->push_xdg_getIcon->whatsThis(); //then try current selection
  if(dir.isEmpty()){ dir = QDir::homePath(); }
  //Get the known file extensions
  QStringList ext = LUtils::imageExtensions();
  for(int i=0; i<ext.length(); i++){ ext[i].prepend("*."); } //turn them into valid filters
  QString file = QFileDialog::getOpenFileName(this, tr("Select an icon"), dir ,QString(tr("Images (%1);; All Files (*)")).arg(ext.join(" ")) );
  if(file.isEmpty()){ return; } //cancelled
  ui->line_xdg_icon->setText(file);
  ReloadAppIcon();
  xdgvaluechanged();
}

//XDG Value Changed
void MainUI::xdgvaluechanged(){
  //qDebug() << "xdgvaluechanged";
  if( INFO->isDesktopFile() || INFO->filePath().isEmpty()  ){
    ui->actionSave_Shortcut->setVisible(true);
    //Compare the current UI values to the file values
    ui->actionSave_Shortcut->setEnabled(canwrite); //assume changed at this point
    // TO-DO

  }else{
    ui->actionSave_Shortcut->setVisible(false);
    ui->actionSave_Shortcut->setEnabled(false);
  }
}

void MainUI::refresh_folder_size(quint64 size, quint64 files, quint64 folders, bool finished) {
  if(finished)
    ui->label_file_size->setText( LUtils::BytesToDisplaySize( size ) + " -- " + tr(" Folders: ") + QString::number(folders) + " / " + tr("Files: ") + QString::number(files) );
  else
    ui->label_file_size->setText( LUtils::BytesToDisplaySize( size ) + " -- " + tr(" Folders: ") + QString::number(folders) + " / " + tr("Files: ") + QString::number(files) + tr("  Calculating..." ));
}
