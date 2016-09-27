//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "BrowserWidget.h"

#include <QVBoxLayout>
#include <QTimer>

BrowserWidget::BrowserWidget(QString objID, QWidget *parent) : QWidget(parent){
  //Setup the Widget/UI
  this->setLayout( new QVBoxLayout(this) );
  this->setWhatsThis(objID);
  //Setup the backend browser object
  BROWSER = new Browser(this);
  connect(BROWSER, SIGNAL(clearItems()), this, SLOT(clearItems()) );
  connect(BROWSER, SIGNAL(itemUpdated(QString)), this, SLOT(itemUpdated(QString)) );
  connect(BROWSER, SIGNAL(itemDataAvailable(QIcon, LFileInfo)), this, SLOT(itemDataAvailable(QIcon, LFileInfo)) );
  connect(BROWSER, SIGNAL(itemsLoading(int)), this, SLOT(itemsLoading(int)) );
  connect(this, SIGNAL(dirChange(QString)), BROWSER, SLOT(loadDirectory(QString)) );
  listWidget = 0;
  treeWidget = 0;
}

BrowserWidget::~BrowserWidget(){

}

void BrowserWidget::changeDirectory(QString dir){
  if(BROWSER->currentDirectory()==dir){ return; } //already on this directory
  emit dirChange(dir);
}

void BrowserWidget::showDetails(bool show){
  //Clean up widgets first
  if(show && listWidget!=0){
    //Clean up list widget
    listWidget->deleteLater();
    listWidget = 0;
  }else if(!show && treeWidget!=0){
    treeWidget->deleteLater();
    treeWidget = 0;
  }
  //Now create any new widgets
  if(show && treeWidget == 0){
    treeWidget = new DDTreeWidget(this);
    this->layout()->addWidget(treeWidget);
    if(!BROWSER->currentDirectory().isEmpty()){ emit dirChange(""); }
  }else if(!show && listWidget==0){
    listWidget = new DDListWidget(this);
    this->layout()->addWidget(listWidget);
    if(!BROWSER->currentDirectory().isEmpty()){ emit dirChange(""); }
  }
}

bool BrowserWidget::hasDetails(){
  return (treeWidget!=0);
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
  QTreeWidgetItem *it = found[0]; //only update the first match (should only ever be one - duplicate file names are disallowed)
  //it->setText(
}

void BrowserWidget::itemDataAvailable(QIcon ico, LFileInfo info){
  qDebug() << "Item Data Available:" << info.fileName();
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
