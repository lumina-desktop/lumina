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

#include <LUtils.h>
#include <LuminaOS.h>

BrowserWidget::BrowserWidget(QString objID, QWidget *parent) : QWidget(parent){
  //Setup the Widget/UI
  this->setLayout( new QVBoxLayout(this) );
  ID = objID;
  //BROWSER = 0;
  //Setup the backend browser object
  BROWSER = new Browser(this);
  connect(BROWSER, SIGNAL(clearItems()), this, SLOT(clearItems()) );
  connect(BROWSER, SIGNAL(itemRemoved(QString)), this, SLOT(itemRemoved(QString)) );
  connect(BROWSER, SIGNAL(itemDataAvailable(QIcon, LFileInfo*)), this, SLOT(itemDataAvailable(QIcon, LFileInfo*)) );
  connect(BROWSER, SIGNAL(itemsLoading(int)), this, SLOT(itemsLoading(int)) );
  connect(this, SIGNAL(dirChange(QString)), BROWSER, SLOT(loadDirectory(QString)) );
  listWidget = 0;
  treeWidget = 0;
  readDateFormat();
  freshload = true; //nothing loaded yet
  numItems = 0;
  this->setMouseTracking(true);
}

BrowserWidget::~BrowserWidget(){
  BROWSER->deleteLater();
}

void BrowserWidget::changeDirectory(QString dir){
  if(BROWSER->currentDirectory()==dir){ return; } //already on this directory
  //qDebug() << "Change Directory:" << dir << historyList;

  if( !dir.contains("/.zfs/snapshot/") ){
    if(historyList.isEmpty() || !dir.isEmpty()){ historyList << dir; }
  }else{
    //Need to remove the zfs snapshot first and ensure that it is not the same dir (just a diff snapshot)
    QString cleaned = dir;
    cleaned = cleaned.replace( QRegExp("/\\.zfs/snapshot/(.)+/"), "/" );
    if( (historyList.isEmpty() || historyList.last()!=cleaned) && !cleaned.isEmpty() ){ historyList << cleaned; }
  }
  //qDebug() << "History:" << historyList;
  emit dirChange(dir);
}

void BrowserWidget::showDetails(bool show){
  //Clean up widgets first
  QSize iconsize;
  if(show && listWidget!=0){
    //Clean up list widget
    iconsize = listWidget->iconSize();
    this->layout()->removeWidget(listWidget);
    listWidget->deleteLater();
    listWidget = 0;
  }else if(!show && treeWidget!=0){
    iconsize = treeWidget->iconSize();
    this->layout()->removeWidget(treeWidget);
    treeWidget->deleteLater();
    treeWidget = 0;
  }
 // qDebug() << "Create Widget: details:" << show;
  //Now create any new widgets
  if(show && treeWidget == 0){
    treeWidget = new DDTreeWidget(this);
      treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
      if(!iconsize.isNull()){ treeWidget->setIconSize(iconsize); }
    this->layout()->addWidget(treeWidget);
    connect(treeWidget, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SIGNAL(itemsActivated()) );
    connect(treeWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SIGNAL(contextMenuRequested()) );
    connect(treeWidget, SIGNAL(DataDropped(QString, QStringList)), this, SIGNAL(DataDropped(QString, QStringList)) );
    connect(treeWidget, SIGNAL(GotFocus()), this, SLOT(selectionChanged()) );
    retranslate();
    treeWidget->sortItems(0, Qt::AscendingOrder);
    if(!BROWSER->currentDirectory().isEmpty()){ emit dirChange(""); }
  }else if(!show && listWidget==0){
    listWidget = new DDListWidget(this);
     listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
     if(!iconsize.isNull()){ listWidget->setIconSize(iconsize); }
    this->layout()->addWidget(listWidget);
    connect(listWidget, SIGNAL(itemActivated(QListWidgetItem*)), this, SIGNAL(itemsActivated()) );
    connect(listWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SIGNAL(contextMenuRequested()) );
    connect(listWidget, SIGNAL(DataDropped(QString, QStringList)), this, SIGNAL(DataDropped(QString, QStringList)) );
    connect(listWidget, SIGNAL(GotFocus()), this, SLOT(selectionChanged()) );
    if(!BROWSER->currentDirectory().isEmpty()){ emit dirChange(""); }
  }
  //qDebug() << "  Done making widget";
}

bool BrowserWidget::hasDetails(){
  return (treeWidget!=0);
}

void BrowserWidget::showHiddenFiles(bool show){
  BROWSER->showHiddenFiles(show);
}

bool BrowserWidget::hasHiddenFiles(){
  return BROWSER->showingHiddenFiles();
}

void BrowserWidget::showThumbnails(bool show){
  BROWSER->showThumbnails(show);
}

bool BrowserWidget::hasThumbnails(){
  return BROWSER->showingThumbnails();
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
  //qDebug() << "Changing Icon Size:" << px << larger;
  if(BROWSER->currentDirectory().isEmpty() || !larger ){ return; } //don't need to reload icons unless the new size is larger
  emit dirChange("");
}

int BrowserWidget::thumbnailSize(){
  if(listWidget!=0){ return listWidget->iconSize().height(); }
  else if(treeWidget!=0){ return treeWidget->iconSize().height(); }
  return 0;
}

void BrowserWidget::setHistory(QStringList paths){
  //NOTE: later items are used first
   historyList = paths;
}

QStringList BrowserWidget::history(){
  return historyList;
}

void BrowserWidget::setShowActive(bool show){
  QString base = "";//"QListWidget::item,QTreeWidget::item{ border: 1px solid transparent; background-color: red; } QListWidget::item:hover,QTreeWidget::item:hover{ border: 1px solid black; background-color: blue; }";
  if(!show){ base.prepend("QAbstractScrollArea{ background-color: rgba(10,10,10,10); } QHeaderView{ background-color: lightgrey; } "); }
  this->setStyleSheet(base);
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


QStringList BrowserWidget::currentSelection(){
  QStringList out;
  if(listWidget!=0){
    QList<QListWidgetItem*> sel = listWidget->selectedItems();
    //qDebug() << "Selection number:" << sel.length();
    //if(sel.isEmpty() && listWidget->currentItem()!=0){ sel << listWidget->currentItem(); }
    //qDebug() << "Selection number:" << sel.length();
    for(int i=0; i<sel.length(); i++){ out << sel[i]->whatsThis(); qDebug() << "Selection:" << sel[i]->text() << sel[i]->whatsThis(); }
  }else if(treeWidget!=0){
    QList<QTreeWidgetItem*> sel = treeWidget->selectedItems();
    //if(sel.isEmpty() && treeWidget->currentItem()!=0){ sel << treeWidget->currentItem(); }
    for(int i=0; i<sel.length(); i++){ out << sel[i]->whatsThis(0); }
  }
  out.removeDuplicates(); //just in case - tree widgets sometimes "select" each column as an individual item
  return out;
}

QStringList BrowserWidget::currentItems(int type){
  //type: 0=all, -1=files, +1=dirs
  QStringList paths;
  if(listWidget!=0){
    for(int i=0; i<listWidget->count(); i++){
      if(i<0 && (listWidget->item(i)->data(Qt::UserRole).toString()=="file") ){ //FILES
        paths << listWidget->item(i)->whatsThis();
      }else if(i>0 &&  (listWidget->item(i)->data(Qt::UserRole).toString()=="dir")){ //DIRS
        paths << listWidget->item(i)->whatsThis();
      }else if(i==0){ //ALL
        paths << listWidget->item(i)->whatsThis();
      }
    }
  }else if(treeWidget!=0){
    for(int i=0; i<treeWidget->topLevelItemCount(); i++){
      if(i<0 && !treeWidget->topLevelItem(i)->text(1).isEmpty()){ //FILES
        paths << treeWidget->topLevelItem(i)->whatsThis(0);
      }else if(i>0 && treeWidget->topLevelItem(i)->text(1).isEmpty()){ //DIRS
        paths << treeWidget->topLevelItem(i)->whatsThis(0);
      }else if(i==0){ //ALL
        paths << treeWidget->topLevelItem(i)->whatsThis(0);
      }
    }
  }
  return paths;
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
  QStringList fmt = date_format;
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
  //qDebug() << "Clear Items";
  if(listWidget!=0){ listWidget->clear(); }
  else if(treeWidget!=0){ treeWidget->clear(); }
  freshload = true;
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

void BrowserWidget::itemDataAvailable(QIcon ico, LFileInfo *info){
  //qDebug() << "Item Data Available:" << info->fileName();
  int num = 0;
  if(listWidget!=0){
    //LIST WIDGET - name and icon only
    if(!listWidget->findItems(info->fileName(), Qt::MatchExactly).isEmpty()){
      //Update existing item
      QListWidgetItem *it = listWidget->findItems(info->fileName(), Qt::MatchExactly).first();
      it->setText(info->fileName());
      it->setWhatsThis(info->absoluteFilePath());
      it->setIcon(ico);
    }else{
      //New item
      QListWidgetItem *it = new CQListWidgetItem(ico, info->fileName(), listWidget);
        it->setWhatsThis(info->absoluteFilePath());
        it->setData(Qt::UserRole, (info->isDir() ? "dir" : "file")); //used for sorting
      listWidget->addItem(it);
    }
    num = listWidget->count();
  }else if(treeWidget!=0){
    QTreeWidgetItem *it = 0;
    if( ! treeWidget->findItems(info->fileName(), Qt::MatchExactly, 0).isEmpty() ){ it =  treeWidget->findItems(info->fileName(), Qt::MatchExactly, 0).first(); }
    else{
      it = new CQTreeWidgetItem(treeWidget);
      it->setText(0, info->fileName() ); //name (0)
      treeWidget->addTopLevelItem(it);
    }
    //Now set/update all the data
    it->setIcon(0, ico);
    it->setText(1, info->isDir() ? "" : LUtils::BytesToDisplaySize(info->size()) ); //size (1)
    it->setText(2, info->mimetype() ); //type (2)
    it->setText(3, DTtoString(info->lastModified() )); //modification date (3)
    it->setText(4, DTtoString(info->created()) ); //creation date (4)
    //Now all the hidden data
    it->setWhatsThis(0, info->absoluteFilePath());
    it->setWhatsThis(3, info->lastModified().toString("yyyyMMddhhmmsszzz") ); //sorts by this actually
    it->setWhatsThis(4, info->created().toString("yyyyMMddhhmmsszzz") ); //sorts by this actually
    num = treeWidget->topLevelItemCount();
  }

  if(num < numItems){
    //Still loading items
    //this->setEnabled(false);
  }else{
    if(freshload && treeWidget!=0){
      //qDebug() << "Resize Tree Widget Contents";
      for(int i=0; i<treeWidget->columnCount(); i++){ treeWidget->resizeColumnToContents(i); }
    }
    freshload = false; //any further changes are updates - not a fresh load of a dir
    //Done loading items
    //this->setEnabled(true);
    //Assemble any status message
    QString stats = QString(tr("Capacity: %1")).arg(LOS::FileSystemCapacity(BROWSER->currentDirectory()));
    int nF, nD;
    double bytes = 0;
    nF = nD = 0;
    if(listWidget!=0){
      bytes = -1; //not supported for this widget
      for(int i=0; i<listWidget->count(); i++){
        if(listWidget->item(i)->data(Qt::UserRole).toString()=="dir"){ nD++; } //directory
        else{ nF++; } //file
      }
    }else if(treeWidget!=0){
      for(int i=0; i<treeWidget->topLevelItemCount(); i++){
        if(treeWidget->topLevelItem(i)->text(1).isEmpty()){
          nD++; //directory
        }else{
          nF++; //file
          bytes+=LUtils::DisplaySizeToBytes(treeWidget->topLevelItem(i)->text(1));
        }
      }
    }
    if( (nF+nD) >0){
      stats.prepend("\t");
      if(nF>0){
        //Has Files
        if(bytes>0){
          stats.prepend( QString(tr("Files: %1 (%2)")).arg(QString::number(nF), LUtils::BytesToDisplaySize(bytes)) );
        }else{
          stats.prepend( QString(tr("Files: %1")).arg(QString::number(nF)) );
        }
      }
      if(nD > 0){
        //Has Dirs
        if(nF>0){ stats.prepend(" / "); }//has files output already
        stats.prepend( QString(tr("Dirs: %1")).arg(QString::number(nD)) );
      }
    }
    emit updateDirectoryStatus( stats.simplified() );
    statustip = stats.simplified(); //save for later
  }//end check for finished loading items
}

void BrowserWidget::itemsLoading(int total){
  //qDebug() << "Got number of items loading:" << total;
  if(listWidget!=0){ listWidget->setWhatsThis( BROWSER->currentDirectory() ); }
  if(treeWidget!=0){ treeWidget->setWhatsThis(BROWSER->currentDirectory() ); }
  numItems = total; //save this for later
  if(total<1){
    emit updateDirectoryStatus( tr("No Directory Contents") );
    this->setEnabled(true);
  }
}

void BrowserWidget::selectionChanged(){
  emit hasFocus(ID); //let the parent know the widget is "active" with the user
}

void BrowserWidget::resizeEvent(QResizeEvent *ev){
  QWidget::resizeEvent(ev); //do the normal processing first
  //The list widget needs to be poked to rearrange the items to fit the new size
  //  tree widget does this fine at the moment.
  if(listWidget!=0){
    listWidget->sortItems(Qt::AscendingOrder);
  }
}
