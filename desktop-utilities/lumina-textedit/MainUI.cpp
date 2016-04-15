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
  updateIcons();
}

MainUI::~MainUI(){
	
}

void MainUI::LoadArguments(QStringList args){ //CLI arguments
  for(int i=0; i<args.length(); i++){
  	  
  }
  
  if(ui->tabWidget->count()<1){
    NewFile();
  }
}

// =================
//      PUBLIC SLOTS
//=================
void MainUI::updateIcons(){
  ui->actionClose->setIcon(LXDG::findIcon("action-close") );

}

// =================
//          PRIVATE
//=================
PlainTextEditor* MainUI::currentEditor(){
  if(ui->tabWidget->count()<1){ return 0; }
  return static_cast<PlainTextEditor*>( ui->tabWidget->currentWidget() );
}

// =================
//    PRIVATE SLOTS
//=================
//Main Actions
void MainUI::NewFile(){
  OpenFile("New-"+QString::number(ui->tabWidget->count()+1));
}

void MainUI::OpenFile(QString file){
  if(file.isEmpty()){
    //Prompt for a file to open
	  
  }
  if(file.isEmpty()){ return; }
  PlainTextEditor *edit = new PlainTextEditor(this);
  ui->tabWidget->addTab(edit, file.section("/",-1));
  edit->showLineNumbers(ui->actionLine_Numbers->isChecked());
  edit->LoadFile(file);
  ui->tabWidget->setCurrentWidget(edit);
}

void MainUI::SaveFile(){
	
}

void MainUI::SaveFileAs(){
	
}

void MainUI::UpdateHighlighting(QAction *act){
  PlainTextEditor *cur = currentEditor();
  if(cur==0){ return; }
  cur->LoadSyntaxRule(act->text());
}
