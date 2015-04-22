#include "DesktopViewPlugin.h"

#include <QFileInfo>
#include <QDir>
#include <QClipboard>
#include <QMimeData>
#include <QImageReader>

#include <LuminaXDG.h>
#include "LSession.h"


DesktopViewPlugin::DesktopViewPlugin(QWidget* parent, QString ID) : LDPlugin(parent, ID){
  this->setLayout( new QVBoxLayout());
    this->layout()->setContentsMargins(0,0,0,0);
  list = new QListWidget(this);
    list->setUniformItemSizes(true);
    list->setViewMode(QListView::IconMode);
    list->setLayoutMode(QListView::Batched);
    list->setBatchSize(10); //keep it snappy
    list->setSpacing(2);
    list->setSelectionBehavior(QAbstractItemView::SelectItems);
    list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    list->setStyleSheet( "QListWidget{ background: transparent; border: none; }" );
    int icosize = settings->value("IconSize",64).toInt();
    list->setIconSize(QSize(icosize,icosize));
    list->setUniformItemSizes(true);
    list->setContextMenuPolicy(Qt::CustomContextMenu);
	
  menu = new QMenu(this);
    menu->addAction( LXDG::findIcon("run-build-file",""), tr("Open"), this, SLOT(runItems()) );
    menu->addSeparator();
    menu->addAction( LXDG::findIcon("edit-cut",""), tr("Cut"), this, SLOT(cutItems()) );
    menu->addAction( LXDG::findIcon("edit-copy",""), tr("Copy"), this, SLOT(copyItems()) );
    menu->addSeparator();
    menu->addAction( LXDG::findIcon("zoom-in",""), tr("Increase Icons"), this, SLOT(increaseIconSize()) );
    menu->addAction( LXDG::findIcon("zoom-out",""), tr("Decrease Icons"), this, SLOT(decreaseIconSize()) );
    menu->addSeparator();
    menu->addAction( LXDG::findIcon("edit-delete",""), tr("Delete"), this, SLOT(deleteItems()) );
    menu->addSeparator();
    if(LUtils::isValidBinary("lumina-fileinfo")){
      menu->addAction( LXDG::findIcon("system-search",""), tr("Properties"), this, SLOT(displayProperties()) );
    }
  this->layout()->addWidget(list);
  this->setInitialSize(600,600);
  watcher = new QFileSystemWatcher(this);
    watcher->addPath(QDir::homePath()+"/Desktop");
    connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(updateContents()) );
	
  connect(list, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(runItems()) );
  connect(list, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showMenu(const QPoint&)) );
  QTimer::singleShot(0,this, SLOT(updateContents()) );
}

DesktopViewPlugin::~DesktopViewPlugin(){

}

void DesktopViewPlugin::runItems(){
    QList<QListWidgetItem*> sel = list->selectedItems();
    for(int i=0; i<sel.length(); i++){
      LSession::LaunchApplication("lumina-open \""+sel[i]->whatsThis()+"\"");
    }
}

void DesktopViewPlugin::copyItems(){
  QList<QListWidgetItem*> sel = list->selectedItems();
  if(sel.isEmpty()){ return; } //nothing selected
  QStringList items;
  //Format the data string
  for(int i=0; i<sel.length(); i++){
    items << "copy::::"+sel[i]->whatsThis();
  }
  //Now save that data to the global clipboard
  QMimeData *dat = new QMimeData;
	dat->clear();
	dat->setData("x-special/lumina-copied-files", items.join("\n").toLocal8Bit());
  QApplication::clipboard()->clear();
  QApplication::clipboard()->setMimeData(dat);
}

void DesktopViewPlugin::cutItems(){
  QList<QListWidgetItem*> sel = list->selectedItems();
  if(sel.isEmpty()){ return; } //nothing selected
  QStringList items;
  //Format the data string
  for(int i=0; i<sel.length(); i++){
    items << "cut::::"+sel[i]->whatsThis();
  }
  //Now save that data to the global clipboard
  QMimeData *dat = new QMimeData;
	dat->clear();
	dat->setData("x-special/lumina-copied-files", items.join("\n").toLocal8Bit());
  QApplication::clipboard()->clear();
  QApplication::clipboard()->setMimeData(dat);
}

void DesktopViewPlugin::deleteItems(){
  QList<QListWidgetItem*> sel = list->selectedItems();
  for(int i=0; i<sel.length(); i++){
    QFile::remove(sel[i]->whatsThis());
  }
}

void DesktopViewPlugin::showMenu(const QPoint &pos){
  //Make sure there is an item underneath the mouse first
  if(list->itemAt(pos)!=0){
    menu->popup(pos);
  }else{
    //Pass the context menu request on to the desktop (emit it from the plugin)
    emit OpenDesktopMenu();
  }
}

void DesktopViewPlugin::increaseIconSize(){
  int icosize = settings->value("IconSize",64).toInt();
  icosize+=16; //go in orders of 16 pixels
  list->setIconSize(QSize(icosize,icosize));
  settings->setValue("IconSize",icosize);
  updateContents();
}

void DesktopViewPlugin::decreaseIconSize(){
  int icosize = settings->value("IconSize",64).toInt();
  if(icosize < 20){ return; } //too small to decrease more
  icosize-=16; //go in orders of 16 pixels
  list->setIconSize(QSize(icosize,icosize));
  settings->setValue("IconSize",icosize);	
  updateContents();
}

void DesktopViewPlugin::updateContents(){
  list->clear();
  if(imgExtensions.isEmpty()){
    QList<QByteArray> fmt = QImageReader::supportedImageFormats();
    for(int i=0; i<fmt.length(); i++){ imgExtensions << QString::fromLocal8Bit(fmt[i]); }
  }
  int icosize = settings->value("IconSize",64).toInt();
  list->setGridSize(QSize(icosize+8,icosize+4+this->fontMetrics().height()));
  QDir dir(QDir::homePath()+"/Desktop");
  QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Type | QDir::DirsFirst);
  for(int i=0; i<files.length(); i++){
    QListWidgetItem *it = new QListWidgetItem;
    it->setWhatsThis(files[i].absoluteFilePath());
    if(files[i].isDir()){
	it->setIcon( LXDG::findIcon("folder","") );
	it->setText( files[i].fileName() );
    }else if(files[i].suffix() == "desktop" ){
	bool ok = false;
	XDGDesktop desk = LXDG::loadDesktopFile(files[i].absoluteFilePath(), ok);
	if(ok){
	  it->setIcon( LXDG::findIcon(desk.icon,"unknown") );
	  if(desk.name.isEmpty()){
	    it->setText( files[i].fileName() );
	  }else{
            it->setText( desk.name );
	  }
	}else{
	  //Revert back to a standard file handling
          it->setIcon( LXDG::findMimeIcon(files[i].fileName()) );
          it->setText( files[i].fileName() );		
	}
    }else if(imgExtensions.contains(files[i].suffix().toLower()) ){
      it->setIcon( QIcon( QPixmap(files[i].absoluteFilePath()).scaled(icosize,icosize,Qt::IgnoreAspectRatio, Qt::SmoothTransformation) ) );
      it->setText( files[i].fileName() );	    
    }else{
      it->setIcon( LXDG::findMimeIcon( files[i].fileName() ) );
      it->setText( files[i].fileName() );
    }
    list->addItem(it);
  }
}

void DesktopViewPlugin::displayProperties(){
    QList<QListWidgetItem*> sel = list->selectedItems();
    for(int i=0; i<sel.length(); i++){
      //lumina-fileinfo only works on *.desktop files at the moment
      if(sel[i]->whatsThis().endsWith(".desktop")){
        LSession::LaunchApplication("lumina-fileinfo \""+sel[i]->whatsThis());
      }
    }
}
