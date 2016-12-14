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
    list->setViewMode(QListView::IconMode);
    list->setFlow(QListWidget::TopToBottom); //Qt bug workaround - need the opposite flow in the widget constructor
    list->setWrapping(true);
    list->setSpacing(4);
    list->setSelectionBehavior(QAbstractItemView::SelectItems);
    list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    list->setContextMenuPolicy(Qt::CustomContextMenu);
    list->setMovement(QListView::Snap); //make sure items are "stuck" in the grid
	
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
    
  connect(QApplication::instance(), SIGNAL(DesktopFilesChanged()), this, SLOT(updateContents()) );
  connect(list, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(runItems()) );
  connect(list, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showMenu(const QPoint&)) );
  QTimer::singleShot(1000,this, SLOT(updateContents()) ); //wait a second before loading contents
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
    if(QFileInfo(sel[i]->whatsThis()).isDir()){
      QProcess::startDetached("rm -r \""+sel[i]->whatsThis()+"\"");
    }else{
      QFile::remove(sel[i]->whatsThis());
    }
  }
}

void DesktopViewPlugin::showMenu(const QPoint &pos){
  //Make sure there is an item underneath the mouse first
  if(list->itemAt(pos)!=0){
    menu->popup(this->mapToGlobal(pos));
  }else{
    //Pass the context menu request on to the desktop (emit it from the plugin)
    this->showPluginMenu();
    //emit OpenDesktopMenu();
  }
}

void DesktopViewPlugin::increaseIconSize(){
  int icosize = this->readSetting("IconSize",64).toInt();
  icosize+=16; //go in orders of 16 pixels
  //list->setIconSize(QSize(icosize,icosize));
  this->saveSetting("IconSize",icosize);
  QTimer::singleShot(10, this, SLOT(updateContents()));
}

void DesktopViewPlugin::decreaseIconSize(){
  int icosize = this->readSetting("IconSize",64).toInt();
  if(icosize < 20){ return; } //too small to decrease more
  icosize-=16; //go in orders of 16 pixels
  //list->setIconSize(QSize(icosize,icosize));
  this->saveSetting("IconSize",icosize);	
  QTimer::singleShot(10,this, SLOT(updateContents()));
}

void DesktopViewPlugin::updateContents(){
  list->clear();

  int icosize = this->readSetting("IconSize",64).toInt();
  QSize gridSZ = QSize(qRound(1.8*icosize),icosize+4+(2*this->fontMetrics().height()) );
  //qDebug() << "Icon Size:" << icosize <<"Grid Size:" << gridSZ.width() << gridSZ.height();
  list->setGridSize(gridSZ);
  list->setIconSize(QSize(icosize,icosize));
  QDir dir(QDir::homePath()+"/Desktop");
  QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Type | QDir::DirsFirst);
  for(int i=0; i<files.length(); i++){
    QListWidgetItem *it = new QListWidgetItem;
    it->setSizeHint(gridSZ); //ensure uniform item sizes
    //it->setForeground(QBrush(Qt::black, Qt::Dense2Pattern)); //Try to use a font color which will always be visible
    it->setTextAlignment(Qt::AlignCenter);
    it->setWhatsThis(files[i].absoluteFilePath());
    QString txt;
    if(files[i].isDir()){
	it->setIcon( LXDG::findIcon("folder","") );
	txt = files[i].fileName();
    }else if(files[i].suffix() == "desktop" ){
	XDGDesktop desk(files[i].absoluteFilePath());
	if(desk.isValid()){
	  it->setIcon( LXDG::findIcon(desk.icon,"unknown") );
	  if(desk.name.isEmpty()){
	    txt = files[i].fileName();
	  }else{
            txt = desk.name;
	  }
	}else{
	  //Revert back to a standard file handling
          it->setIcon( LXDG::findMimeIcon(files[i].fileName()) );
          txt = files[i].fileName();		
	}
    }else if(LUtils::imageExtensions().contains(files[i].suffix().toLower()) ){
      it->setIcon( QIcon( QPixmap(files[i].absoluteFilePath()).scaled(icosize,icosize,Qt::IgnoreAspectRatio, Qt::SmoothTransformation) ) );
      txt = files[i].fileName();	    
    }else{
      it->setIcon( LXDG::findMimeIcon( files[i].fileName() ) );
      txt = files[i].fileName();
    }
    //Add the sym-link overlay to the icon as necessary
    if(files[i].isSymLink()){
      QImage img = it->icon().pixmap(QSize(icosize,icosize)).toImage();
      int oSize = icosize/2; //overlay size
      QPixmap overlay = LXDG::findIcon("emblem-symbolic-link").pixmap(oSize,oSize).scaled(oSize,oSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
      QPainter painter(&img);
        painter.drawPixmap(icosize-oSize,icosize-oSize,overlay); //put it in the bottom-right corner
      it->setIcon( QIcon(QPixmap::fromImage(img)) );
    }
    //Now adjust the visible text as necessary based on font/grid sizing
    it->setToolTip(txt);
    if(this->fontMetrics().width(txt) > (gridSZ.width()-4) ){
      //int dash = this->fontMetrics().width("-");
      //Text too long, try to show it on two lines
      txt = txt.section(" ",0,2).replace(" ","\n"); //First take care of any natural breaks
      if(txt.contains("\n")){
        //need to check each line
	QStringList txtL = txt.split("\n");
	for(int i=0; i<txtL.length(); i++){ txtL[i] = this->fontMetrics().elidedText(txtL[i], Qt::ElideRight, gridSZ.width()-4); }
	txt = txtL.join("\n");
	if(txtL.length()>2){ txt = txt.section("\n",0,1); } //only keep the first two lines
      }else{
        txt = this->fontMetrics().elidedText(txt,Qt::ElideRight, 2*(gridSZ.width()-4));
        //Now split the line in half for the two lines
        txt.insert( (txt.count()/2), "\n");
      }
    }else{
      txt.append("\n "); //ensure two lines (2nd one invisible) - keeps formatting sane
    }
    it->setText(txt);
    list->addItem(it);
    if( (i%10) == 0){ QApplication::processEvents(); }//keep the UI snappy, every 10 items
  }
  list->setFlow(QListWidget::TopToBottom); //To ensure this is consistent - issues with putting it in the constructor
  list->update(); //Re-paint the widget after all items are added 
}

void DesktopViewPlugin::displayProperties(){
    QList<QListWidgetItem*> sel = list->selectedItems();
    for(int i=0; i<sel.length(); i++){
      LSession::LaunchApplication("lumina-fileinfo \""+sel[i]->whatsThis());
    }
}
