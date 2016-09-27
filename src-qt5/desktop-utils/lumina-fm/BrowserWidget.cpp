//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "BrowserWidget.h"

#include <QVBoxLayout>
#include <QTimer>
#include <QSettings>

#include <LuminaUtils.h>

BrowserWidget::BrowserWidget(QString objID, QWidget *parent) : QWidget(parent){
  //Setup the Widget/UI
  this->setLayout( new QVBoxLayout(this) );
  ID = objID;
  //this->setWhatsThis(objID);
  //Setup the backend browser object
  BROWSER = new Browser(this);
  connect(BROWSER, SIGNAL(clearItems()), this, SLOT(clearItems()) );
  connect(BROWSER, SIGNAL(itemUpdated(QString)), this, SLOT(itemUpdated(QString)) );
  connect(BROWSER, SIGNAL(itemDataAvailable(QIcon, LFileInfo)), this, SLOT(itemDataAvailable(QIcon, LFileInfo)) );
  connect(BROWSER, SIGNAL(itemsLoading(int)), this, SLOT(itemsLoading(int)) );
  connect(this, SIGNAL(dirChange(QString)), BROWSER, SLOT(loadDirectory(QString)) );
  listWidget = 0;
  treeWidget = 0;
  readDateFormat();
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
    retranslate();
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

void BrowserWidget::setThumbnailSize(int px){
  bool larger = true;
  if(listWidget!=0){ 
    larger = listWidget->iconSize().height() < px;
    listWidget->setIconSize(QSize(px,px));
  }else if(treeWidget!=0){
    larger = treeWidget->iconSize().height() < px;
    treeWidget->setIconSize(QSize(px,px));
  }
  if(BROWSER->currentDirectory().isEmpty() || !larger ){ return; } //don't need to reload icons unless the new size is larger
  emit dirChange("");
}

QStringList BrowserWidget::getDateFormat() {
  return date_format;
}

// This function is only called if user changes sessionsettings. By doing so, operations like sorting by date
// are faster because the date format is already stored in DirWidget::date_format static variable
void BrowserWidget::readDateFormat() {
  if(!date_format.isEmpty())
      date_format.clear();
  QSettings settings("lumina-desktop","sessionsettings");
  // If value doesn't exist or is not setted, empty string is returned
  date_format << settings.value("DateFormat").toString();
  date_format << settings.value("TimeFormat").toString();
}

// =================
//     PUBLIC SLOTS
// =================
void BrowserWidget::retranslate(){
  if(listWidget!=0){

  }else if(treeWidget!=0){
    QTreeWidgetItem *it = new QTreeWidgetItem();
    it->setText(0,tr("Name"));
    it->setText(1,tr("Size"));
    it->setText(2, tr("Type"));
    it->setText(3, tr("Date Modified") );
    it->setText(4, tr("Date Created") );
    treeWidget->setHeaderItem(it);
    //Now reset the sorting (alphabetically, dirs first)
    treeWidget->sortItems(0, Qt::AscendingOrder);  // sort by name
    treeWidget->sortItems(1, Qt::AscendingOrder);  //sort by type
  }
}

// =================
//          PRIVATE
// =================
QString BrowserWidget::DTtoString(QDateTime dt){
  QStringList fmt = getDateFormat();
  if(fmt.isEmpty() || fmt.length()!=2 || (fmt[0].isEmpty() && fmt[1].isEmpty()) ){
    //Default formatting
    return dt.toString(Qt::DefaultLocaleShortDate);
  }else if(fmt[0].isEmpty()){
    //Time format only
    return (dt.date().toString(Qt::DefaultLocaleShortDate)+" "+dt.time().toString(fmt[1]));
  }else if(fmt[1].isEmpty()){
    //Date format only
    return (dt.date().toString(fmt[0])+" "+dt.time().toString(Qt::DefaultLocaleShortDate));
  }else{
    //both date/time formats set
    return dt.toString(fmt.join(" "));
  }
}

// =================
//    PRIVATE SLOTS
// =================
void BrowserWidget::clearItems(){
  if(listWidget!=0){ listWidget->clear(); }
  else if(treeWidget!=0){ treeWidget->clear(); }
  this->setEnabled(false);
}

void BrowserWidget::itemRemoved(QString item){
  //qDebug() << "item removed" << item;
  if(treeWidget!=0){
    QList<QTreeWidgetItem*> found = treeWidget->findItems(item.section("/",-1), Qt::MatchExactly, 0); //look for exact name match
    if(found.isEmpty()){ return; } //no match
    delete found[0];
  }else if(listWidget!=0){
    QList<QListWidgetItem*> found = listWidget->findItems(item.section("/",-1), Qt::MatchExactly); //look for exact name match
    if(found.isEmpty()){ return; }
    delete found[0];
  }
}

void BrowserWidget::itemDataAvailable(QIcon ico, LFileInfo info){
  qDebug() << "Item Data Available:" << info.fileName();
  int num = 0;
  if(listWidget!=0){
    //LIST WIDGET - name and icon only
    if(!listWidget->findItems(info.fileName(), Qt::MatchExactly).isEmpty()){ 
      //Update existing item
      QListWidgetItem *it = listWidget->findItems(info.fileName(), Qt::MatchExactly).first();
      it->setText(info.fileName());
      it->setWhatsThis(info.absoluteFilePath());
      it->setIcon(ico);
    }else{
      //New item
      listWidget->addItem( new QListWidgetItem(ico, info.fileName(), listWidget) );
    }
    num = listWidget->count();
  }else if(treeWidget!=0){
    QTreeWidgetItem *it = 0;
    if( ! treeWidget->findItems(info.fileName(), Qt::MatchExactly, 0).isEmpty() ){ it =  treeWidget->findItems(info.fileName(), Qt::MatchExactly, 0).first(); }
    else{ 
      it = new CQTreeWidgetItem(treeWidget);  
      it->setText(0, info.fileName() ); //name (0)
      treeWidget->addTopLevelItem(it);
    }
    //Now set/update all the data
    it->setText(1, LUtils::BytesToDisplaySize(info.size()) ); //size (1)
    it->setText(2, info.mimetype() ); //type (2)
    it->setText(3, DTtoString(info.lastModified() )); //modification date (3)
    it->setText(4, DTtoString(info.created()) ); //creation date (4)
    //Now all the hidden data
    it->setWhatsThis(0, info.absoluteFilePath());
    it->setWhatsThis(3, info.lastModified().toString("yyyyMMddhhmmsszzz") ); //sorts by this actually
    it->setWhatsThis(4, info.created().toString("yyyyMMddhhmmsszzz") ); //sorts by this actually
    num = treeWidget->topLevelItemCount();
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
