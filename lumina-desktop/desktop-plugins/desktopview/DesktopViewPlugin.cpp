//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "DesktopViewPlugin.h"

DesktopViewPlugin::DesktopViewPlugin(QWidget *parent) : LDPlugin(parent, "desktopview"){
  watcher = new QFileSystemWatcher(this);
  deskDir = QDir::homePath();
  if(QFile::exists(deskDir+"/Desktop") ){
    deskDir = deskDir+"/Desktop";
  }else if(QFile::exists(deskDir+"/desktop") ){
    deskDir = deskDir+"/desktop";
  }
  watcher->addPath(deskDir);
  icoSize = 0; //temporary placeholder
  spacing = 0; //temporary placeholder
  ITEMS.clear();
  layout = new QGridLayout(this);
    layout->setContentsMargins(1,1,1,1);
    this->setLayout(layout);
  
  //Connect the signals/slots
  connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(UpdateDesktop()) );
  connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(UpdateDesktop()) );
  
  //Now launch the update mechanisms in a new thread
  QTimer::singleShot(10, this, SLOT(UpdateDesktop()) );
}

DesktopViewPlugin::~DesktopViewPlugin(){

}

void DesktopViewPlugin::UpdateDesktop(){
  //Calculate available rows/columns
  int oldSize = icoSize;	
  icoSize = 64; //64x64 default icons for now (make dynamic later)
  int oldspacing = spacing;
  spacing = 4; // 4 pixel space between items (make dynamic later);
  if(icoSize != oldSize || spacing != oldspacing){
    //Re-create all the items with the proper size
    for(int i=0; i<ITEMS.length(); i++){
      delete ITEMS.takeAt(i); //delete the widget
      i--;
    }
  }
  layout->setSpacing(spacing);

  int rmax = (this->height()-2)/(icoSize+spacing);
  int cmax = (this->width()-2)/(icoSize+spacing);
  //Now get the current items in the folder
  QDir dir(deskDir);
  QStringList items = dir.entryList( QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::Type | QDir::LocaleAware | QDir::DirsFirst);
  //iterate over all current items
  for(int i=0; i<ITEMS.length(); i++){
    int index = items.indexOf( ITEMS[i]->whatsThis().section("/",-1) );
    if( index == -1 ){
      //item no longer exists - remove it
      delete ITEMS.takeAt(i);
      i--;
    }else{
      //Item still exists - remove it from the "new" list
      ITEMS[i]->updateItem();
      items.removeAt(index);
    }
  }
  //Now iterate over the spaces in the widget and create items as necessary
  for(int r=0; r<rmax; r++){
    layout->setRowMinimumHeight(r,icoSize);
    for(int c=0; c<cmax && items.length() > 0; c++){
      if(r==0){ layout->setColumnMinimumWidth(c,icoSize); }
      if(layout->itemAtPosition(r,c)==0 && items.length() > 0){
        //Empty spot, put the first new item here
	DeskItem *it = new DeskItem(this, deskDir+"/"+items[0], icoSize);
	items.removeAt(0);
	layout->addWidget(it, r,c);
	ITEMS << it;
      }
    }
  }
  if(layout->itemAtPosition(rmax,cmax)==0){
    layout->addWidget(new QWidget(this), rmax, cmax); //put an empty widget here as a placeholder
  }
}