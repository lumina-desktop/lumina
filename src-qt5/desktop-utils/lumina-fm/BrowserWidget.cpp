//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "BrowserWidget.h"
BrowserWidget::BrowserWidget(QString objID, QWidget *parent) : QWidget(parent){
  //Setup the Widget/UI
  this->setLayout( new QVBoxLayout(this) );
  
  //Setup the backend browser object
  BROWSER = new Browser(this);
  connect(BROWSER, SIGNAL(clearItems()), this, SLOT(clearItems()) );
  connect(BROWSER, SIGNAL(itemUpdated(QString)), this, SLOT(itemUpdated(QString)) );
  connect(BROWSER, SIGNAL(itemUpdated(QString)), this, SLOT(itemUpdated(QString)) );
  connect(BROWSER, SIGNAL(itemUpdated(QString)), this, SLOT(itemUpdated(QString)) );

  listWidget = 0;
  treeWidget = 0;
}

BrowserWidget::~BrowserWidget(){

}

void BrowserWidget::changeDirectory(QString dir){
  if(BROWSER->currentDirectory()==dir){ return; } //already on this directory
  BROWSER->loadDirectory(dir);
}

// =================
//    PRIVATE SLOTS
// =================
void BrowserWidget::clearItems(){
  if(listWidget!=0){ listWidget->clear(); }
  else if(treeWidget!=0){ treeWidget->clear(); }
  this->setEnabled(false);
}

void BrowserWidget::itemUpdated(QString item){
  if(treeWidget==0){ return; } //only used for the tree widget/details
  qDebug() << "item updated" << item;
  QList<QTreeWidgetItem*> found = treeWidget->findItems(item.section("/",-1), Qt::MatchExactly, 0); //look for exact name match
  if(found.isEmpty()){ return; } //no match
  QTreeWidgetItem *it = found[0]; //onlyp update the first match (should only ever be one - duplicate file names are disallowed)
  //it->setText(
}

void BrowserWidget::itemDataAvailable(QIcon ico, LFileInfo info){
  int num = 0;
  if(listWidget!=0){
    listWidget->addItem( new QListWidgetItem(ico, info.fileName(), listWidget) );
    num = listWidget->count();
  }else if(treeWidget!=0){
    //TODO
  }
  if(num < numItems){
    //Still loading items
    //this->setEnabled(false);
  }else{
    //Done loading items
    this->setEnabled(true);
  }
}

void BrowserWidget::itemsLoading(int total){
  numItems = total; //save this for later
}

