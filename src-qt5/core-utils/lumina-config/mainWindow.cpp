//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "mainWindow.h"
#include "ui_mainWindow.h"
#include "globals.h"

#include "pages/getPage.h"

//=============
//      PUBLIC
//=============
mainWindow::mainWindow() : QMainWindow(), ui(new Ui::mainWindow()){
  ui->setupUi(this);
  changePage(""); //load the default main page
}

mainWindow::~mainWindow(){

}

//==============
//  PUBLIC SLOTS
//==============
void mainWindow::slotSingleInstance(){
  this->showNormal(); //just in case it is hidden/minimized
}

void mainWindow::setupIcons(){
  ui->actionSave->setIcon( LXDG::findIcon("document-save","") );
}

//=============
//      PRIVATE
//=============
void mainWindow::changePage(QString id){
  PageWidget *page =  GetNewPage(id, this);
  if(page==0){ return; }
  qDebug() << "Changing page:" << id;
  cpage = id;
  QWidget *old = this->centralWidget();
  this->setCentralWidget(page);
  if(old!=0 && old!=ui->centralwidget){ old->disconnect(); old->deleteLater(); }
  //Connect the new page
  connect(page, SIGNAL(HasPendingChanges(bool)), this, SLOT(pageCanSave(bool)) );
  connect(page, SIGNAL(ChangePageTitle(QString)), this, SLOT(pageSetTitle(QString)) );
  connect(page, SIGNAL(ChangePage(QString)), this, SLOT(page_change(QString)) );
  //Now load the new page
  page->LoadSettings(0); //need to make this show the current screen as needed
  //Now update this UI a bit based on page settings
  bool needscreen = page->needsScreenSelector();

  this->showNormal();
}
//================
//  PRIVATE SLOTS
//================
//Page signal handling
void mainWindow::pageCanSave(bool save){
  ui->actionSave->setVisible(save);
  ui->actionSave->setEnabled(save);
}

void mainWindow::pageSetTitle(QString title){
  this->setWindowTitle(title);
}

void mainWindow::page_change(QString id){
  if(ui->actionSave->isEnabled()){
    //unsaved changed available - prompt to save first
    // TO-DO
  }
  changePage(id);
}
