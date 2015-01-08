//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "FODialog.h"
#include "ui_FODialog.h"

#include <QApplication>

FODialog::FODialog(QWidget *parent) : QDialog(parent), ui(new Ui::FODialog){
  ui->setupUi(this); //load the designer file
  ui->label->setText(tr("Calculating"));
  ui->progressBar->setVisible(false);
  ui->push_stop->setIcon( LXDG::findIcon("edit-delete","") );
  //Now set the internal defaults
  isRM = isCP = isRESTORE = isMV = stopped = noerrors = false;
  overwrite = -1; //set to automatic by default
  this->show();
}

FODialog::~FODialog(){
  stopped = true; //just in case it might still be running when closed
}

void FODialog::setOverwrite(bool ovw){
  if(ovw){ overwrite = 1; }
  else{ overwrite = 0; }
}

//Public "start" functions
void FODialog::RemoveFiles(QStringList paths){
  ofiles = paths;
  isRM = true;
  QTimer::singleShot(10,this, SLOT(slotStartOperations()));
}

void FODialog::CopyFiles(QStringList oldPaths, QStringList newPaths){ 	 
  //same permissions as old files
  if(oldPaths.length() == newPaths.length()){
    ofiles = oldPaths; nfiles = newPaths;
  }
  isCP=true;
  QTimer::singleShot(10,this, SLOT(slotStartOperations()));
}

void FODialog::RestoreFiles(QStringList oldPaths, QStringList newPaths){
  //user/group rw permissions
  if(oldPaths.length() == newPaths.length()){
    ofiles = oldPaths; nfiles = newPaths;
  }
  isRESTORE = true;
  QTimer::singleShot(10,this, SLOT(slotStartOperations()));
}

void FODialog::MoveFiles(QStringList oldPaths, QStringList newPaths){
  //no change in permissions
  if(oldPaths.length() == newPaths.length()){
    ofiles = oldPaths; nfiles = newPaths;
  }
  isMV=true;
  QTimer::singleShot(10,this, SLOT(slotStartOperations()));
}

// ==== PRIVATE ====
QStringList FODialog::subfiles(QString dirpath, bool dirsfirst){
  //NOTE: dirpath (input) is always the first/last item in the output as well!
  QStringList out;
  if(dirsfirst){ out << dirpath; }
  if( QFileInfo(dirpath).isDir() ){
    QDir dir(dirpath);
    if(dirsfirst){
      //Now recursively add any subdirectories and their contents
      QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden, QDir::NoSort);
      for(int i=0; i<subdirs.length(); i++){ out << subfiles(dir.absoluteFilePath(subdirs[i]), dirsfirst); }
    }
    //List the files
    QStringList files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden, QDir::NoSort);
    for(int i=0; i<files.length(); i++){ out << dir.absoluteFilePath(files[i]); }
    
    if(!dirsfirst){
      //Now recursively add any subdirectories and their contents
      QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden, QDir::NoSort);
      for(int i=0; i<subdirs.length(); i++){ out << subfiles(dir.absoluteFilePath(subdirs[i]), dirsfirst); }
    }
  }
  if(!dirsfirst){ out << dirpath; }
  return out;
}

QString FODialog::newFileName(QString path){
  int num=1;
  QString extension = path.section("/",-1).section(".",-1);
  if( path.section("/",-1) == "."+extension || extension ==path.section("/",-1) ){ extension.clear(); } //just a hidden file without extension or directory
  if(!extension.isEmpty() ){ 
    extension.prepend("."); 
    path.chop(extension.length());
  }
  while( QFile::exists(path+"-"+QString::number(num)+extension) ){ num++; }
  return QString(path+"-"+QString::number(num)+extension);
}

QStringList FODialog::removeItem(QString path, bool recursive){
  //qDebug() << "Remove Path:" << path;
  QStringList items;
  if(recursive){ items = subfiles(path,false); }
  else{ items << path; } //only the given path
  //qDebug() << " - Subfiles:" << items;
  QStringList err;	
  for(int i=0; i<items.length(); i++){
    if(QFileInfo(items[i]).isDir()){
      if(items[i]==path){
        //Current Directory Removal
        QDir dir;
        if( !dir.rmdir(items[i]) ){ err << items[i]; }		      
      }else{
        //Recursive Directory Removal
        err << removeItem(items[i], recursive);	      
      }
    }else{
      //Simple File Removal
      if( !QFile::remove(items[i]) ){ err << items[i]; }	    
    }
  }
  return err;
}

QStringList FODialog::copyItem(QString oldpath, QString newpath){
  QStringList err;
  if(QFileInfo(oldpath).isDir()){
    //Create a new directory with the same name (no way to copy dir+contents)
    QDir dir;
    if( !dir.mkpath(newpath) ){ err << oldpath; }
  }else{
    //Copy the file and reset permissions as necessary
    if( !QFile::copy(oldpath, newpath) ){ err << oldpath; }
    else{
      if(isCP){
	QFile::setPermissions(newpath, QFile::permissions(oldpath));
	//Nothing special for copies at the moment (might be something we run into later)
      }else if(isRESTORE){
	QFile::setPermissions(newpath, QFile::permissions(oldpath));
	//Nothing special for restores at the moment (might be something we run into later)
      }
    }
  }
  return err;
}

// ==== PRIVATE SLOTS ====
void FODialog::slotStartOperations(){
  ui->label->setText(tr("Calculating..."));
  //qDebug() << "Start File operations" << isRM << isCP << isMV << ofiles << nfiles;
  //Now setup the UI
  ui->progressBar->setRange(0,ofiles.length());
  ui->progressBar->setValue(0);
  ui->progressBar->setVisible(true);
  QApplication::processEvents();
  if(!isRM && overwrite==-1){
    //Check if the new files already exist, and prompt for action
    QStringList existing;
    for(int i=0; i<nfiles.length(); i++){
      if(QFile::exists(nfiles[i])){ existing << nfiles[i].section("/",-1); }
    }
    if(!existing.isEmpty()){
      //Prompt for whether to overwrite, not overwrite, or cancel
      QMessageBox::StandardButton ans = QMessageBox::question(this, tr("Overwrite Files?"), tr("Do you want to overwrite the existing files?")+"\n"+tr("Note: It will just add a number to the filename otherwise.")+"\n\n"+existing.join(", "), QMessageBox::YesToAll | QMessageBox::NoToAll | QMessageBox::Cancel, QMessageBox::NoToAll);
      if(ans==QMessageBox::NoToAll){ overwrite = 0; } //don't overwrite
      else if(ans==QMessageBox::YesToAll){ overwrite = 1; } //overwrite
      else{ this->close(); return; } //cancel operations
    }
  }
  //Get the complete number of items to be operated on (better tracking)
  QStringList olist, nlist; //old/new list
  for(int i=0; i<ofiles.length(); i++){
    if(isRM){ //only old files
      olist << subfiles(ofiles[i], false); //dirs need to be last for removals
    }else if(isCP || isRESTORE){
      if(QFile::exists(nfiles[i])){
	if(!overwrite){
	  nfiles[i] = newFileName(nfiles[i]); //prompt for new file name up front before anything starts
	}
      }
      QStringList subs = subfiles(ofiles[i], true); //dirs need to be first for additions
      for(int s=0; s<subs.length(); s++){
        olist << subs[s];
	QString newsub = subs[s].section(ofiles[i],0,100, QString::SectionSkipEmpty); 
	    newsub.prepend(nfiles[i]);
	nlist << newsub;
      }
    }else{ //Move/rename
      if( nfiles[i].startsWith(ofiles[i]+"/") ){
	//This is trying to move a directory into itself  (not possible)
	// Example: move "~/mydir" -> "~/mydir/mydir2"
	QMessageBox::warning(this, tr("Invalid Move"), QString(tr("It is not possible to move a directory into itself. Please make a copy of the directory instead.\n\nOld Location: %1\nNew Location: %2")).arg(ofiles[i], nfiles[i]) );
      }else{
	//no changes necessary
        olist << ofiles[i];
        nlist << nfiles[i];
      }
    }
  }
  //Now start iterating over the operations
  QStringList errlist;
  for(int i=0; i<olist.length() && !stopped; i++){
    if(isRM){
      ui->label->setText( QString(tr("Removing: %1")).arg(olist[i].section("/",-1)) );
      errlist << removeItem(olist[i]);
    }else if(isCP || isRESTORE){
      ui->label->setText( QString(tr("Copying: %1 to %2")).arg(olist[i].section("/",-1), nlist[i].section("/",-1)) );
      if(QFile::exists(nlist[i])){
	if(overwrite){
	  errlist << removeItem(nlist[i], true); //recursively remove the file/dir since we are supposed to overwrite it
	}
      }
      //If a parent directory fails to copy, skip all the children as well (they will also fail)
      if( !errlist.contains(olist[i].section("/",0,-1)) ){ 
        errlist << copyItem(olist[i], nlist[i]);
      }
      /*if(err.isEmpty() && !subs.isEmpty() ){ //make sure the initial parent dir did not error out
        //For copies of directories, also need to copy over all the sub files/dirs
        for(int s=0; s<subs.length(); s++){
	  //Replace the old parent directory path with the new parent directory path;
	  QString newsub = subs[s].section(ofiles[i],0,100, QString::SectionSkipEmpty); 
	    newsub.prepend(nfiles[i]);
	  //qDebug() << "Copy Sub:" << subs[s] << newsub;
          errlist << copyItem(subs[s], newsub);
        }
      }*/
    }else if(isMV){
      ui->label->setText( QString(tr("Moving: %1 to %2")).arg(ofiles[i].section("/",-1), nfiles[i].section("/",-1)) );
      if( !QFile::rename(ofiles[i], nfiles[i]) ){
        errlist << ofiles[i];
      }	
    }
    ui->progressBar->setValue(i+1);
    QApplication::processEvents();
  }
  //All finished, so close the dialog if successful
  errlist.removeAll(""); //make sure to clear any empty items
  if(!errlist.isEmpty()){
    QString msg;
    if(isRM){ msg = tr("Could not remove these files:"); }
    else if(isCP){ msg = tr("Could not copy these files:"); }
    else if(isRESTORE){ msg = tr("Could not restore these files:"); }
    else if(isMV){ msg = tr("Could not move these files:"); }
    QMessageBox::warning(this, tr("File Errors"), msg+"\n\n"+errlist.join("\n"));
  }
  noerrors = errlist.isEmpty();
  this->close();
}

void FODialog::on_push_stop_clicked(){
  stopped = true;
}