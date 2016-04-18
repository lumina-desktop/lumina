//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "MainUI.h"
#include "ui_MainUI.h"

#include "syntaxSupport.h"

#include <LuminaXDG.h>

#include <QFileDialog>
#include <QDir>

MainUI::MainUI() : QMainWindow(), ui(new Ui::MainUI){
  ui->setupUi(this);
  this->setWindowTitle(tr("Text Editor"));
  ui->tabWidget->clear();
  //Update the menu of available syntax highlighting modes
  QStringList smodes = Custom_Syntax::availableRules();
  for(int i=0; i<smodes.length(); i++){
    ui->menuSyntax_Highlighting->addAction(smodes[i]);
  }
  //Setup any connections
  connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(close()) );
  connect(ui->actionNew_File, SIGNAL(triggered()), this, SLOT(NewFile()) );
  connect(ui->actionOpen_File, SIGNAL(triggered()), this, SLOT(OpenFile()) );
  connect(ui->actionSave_File, SIGNAL(triggered()), this, SLOT(SaveFile()) );
  connect(ui->actionSave_File_As, SIGNAL(triggered()), this, SLOT(SaveFileAs()) );
  connect(ui->menuSyntax_Highlighting, SIGNAL(triggered(QAction*)), this, SLOT(UpdateHighlighting(QAction*)) );
  connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged()) );
  connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(tabClosed(int)) );
  connect(ui->actionLine_Numbers, SIGNAL(toggled(bool)), this, SLOT(showLineNumbers(bool)) );
  updateIcons();
}

MainUI::~MainUI(){
	
}

void MainUI::LoadArguments(QStringList args){ //CLI arguments
  for(int i=0; i<args.length(); i++){
  	  
  }
  
  /*if(ui->tabWidget->count()<1){
    NewFile();
  }*/
}

// =================
//      PUBLIC SLOTS
//=================
void MainUI::updateIcons(){
  ui->actionClose->setIcon(LXDG::findIcon("application-exit") );
  ui->actionNew_File->setIcon(LXDG::findIcon("document-new") );
  ui->actionOpen_File->setIcon(LXDG::findIcon("document-open") );
  ui->actionSave_File->setIcon(LXDG::findIcon("document-save") );
  ui->actionSave_File_As->setIcon(LXDG::findIcon("document-save-as") );
  ui->menuSyntax_Highlighting->setIcon( LXDG::findIcon("format-text-color") );

}

// =================
//          PRIVATE
//=================
PlainTextEditor* MainUI::currentEditor(){
  if(ui->tabWidget->count()<1){ return 0; }
  return static_cast<PlainTextEditor*>( ui->tabWidget->currentWidget() );
}

QString MainUI::currentFileDir(){
  PlainTextEditor* cur = currentEditor();
  QString dir;
  if(cur!=0){
    if(cur->currentFile().startsWith("/")){
      dir = cur->currentFile().section("/",0,-2);
    }
  }
  return dir;
}

// =================
//    PRIVATE SLOTS
//=================
//Main Actions
void MainUI::NewFile(){
  OpenFile("New-"+QString::number(ui->tabWidget->count()+1));
}

void MainUI::OpenFile(QString file){
  QStringList files;
  if(file.isEmpty()){
    //Prompt for a file to open
    files = QFileDialog::getOpenFileNames(this, tr("Open File(s)"), currentFileDir(), tr("Text Files (*)") );
    if(files.isEmpty()){ return; } //cancelled
  }else{
    files << file;
  }
  for(int i=0; i<files.length(); i++){
    PlainTextEditor *edit = new PlainTextEditor(this);
      connect(edit, SIGNAL(FileLoaded(QString)), this, SLOT(updateTab(QString)) );
      connect(edit, SIGNAL(UnsavedChanges(QString)), this, SLOT(updateTab(QString)) );
    ui->tabWidget->addTab(edit, files[i].section("/",-1));
    edit->showLineNumbers(ui->actionLine_Numbers->isChecked());
    ui->tabWidget->setCurrentWidget(edit);
    edit->LoadFile(files[i]);
    edit->setFocus();
    QApplication::processEvents(); //to catch the fileLoaded() signal
  }
}

void MainUI::SaveFile(){
  PlainTextEditor *cur = currentEditor();
  if(cur==0){ return; }
  cur->SaveFile();
}

void MainUI::SaveFileAs(){
  PlainTextEditor *cur = currentEditor();
  if(cur==0){ return; }
  cur->SaveFile(true);	
}

void MainUI::UpdateHighlighting(QAction *act){
  PlainTextEditor *cur = currentEditor();
  if(cur==0){ return; }
  cur->LoadSyntaxRule(act->text());
}

void MainUI::showLineNumbers(bool show){
  for(int i=0; i<ui->tabWidget->count(); i++){
    PlainTextEditor *edit = static_cast<PlainTextEditor*>(ui->tabWidget->widget(i));
    edit->showLineNumbers(show);
  }
}

void MainUI::updateTab(QString file){
  PlainTextEditor *cur = currentEditor();
  if(cur==0){ return; } //should never happen
  int index = ui->tabWidget->currentIndex();
  if(index<0){ index = 0; } //FALLBACK - use the first tab
  bool changes = cur->hasChange();
  ui->tabWidget->setTabText(index,(changes ? "*" : "") + file.section("/",-1));
  ui->actionSave_File->setEnabled(changes);
  ui->actionSave_File_As->setEnabled(changes);
}

void MainUI::tabChanged(){
  //update the buttons/menus based on the current widget
  PlainTextEditor *cur = currentEditor();
  if(cur==0){ return; } //should never happen though
  bool changes = cur->hasChange();
  ui->actionSave_File->setEnabled(changes);
  ui->actionSave_File_As->setEnabled(changes);
}

void MainUI::tabClosed(int tab){
  PlainTextEditor *edit = static_cast<PlainTextEditor*>(ui->tabWidget->widget(tab));
  if(edit==0){ return; } //should never happen
  if(edit->hasChange()){
    //Verify if the user wants to lose any unsaved changes
	  
  }
  ui->tabWidget->removeTab(tab);
  edit->deleteLater();
}
