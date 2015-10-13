//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LDesktopPluginSpace.h"
#include "LSession.h"
#include "desktop-plugins/NewDP.h"

#include <LuminaXDG.h>
#include <QDesktopWidget>

#define DEBUG 0

// ===================
//      PUBLIC
// ===================
LDesktopPluginSpace::LDesktopPluginSpace(QWidget *parent) : QWidget(parent){
  this->setObjectName("LuminaDesktopPluginSpace");
  this->setStyleSheet("QWidget#LuminaDesktopPluginSpace{ border: none; background: transparent; }"); 
  this->setAcceptDrops(true);
  this->setContextMenuPolicy(Qt::NoContextMenu);
  this->setMouseTracking(true);
  TopToBottom = true;
  plugsettings = LSession::handle()->DesktopPluginSettings();

}

LDesktopPluginSpace::~LDesktopPluginSpace(){
  
}

void LDesktopPluginSpace::LoadItems(QStringList plugs, QStringList files){
  if(DEBUG){ qDebug() << "Loading Desktop Items:" << plugs << files; }
  bool changes = false;
  if(plugs != plugins){ plugins = plugs; changes = true; }
  if(files != deskitems){ deskitems = files; changes = true; }
  if(changes){ QTimer::singleShot(0,this, SLOT(reloadPlugins())); }
}

void LDesktopPluginSpace::SetIconSize(int size){
  if(DEBUG){ qDebug() << "Set Desktop Icon Size:" << size; }
  QSize newsize = calculateItemSize(size);
  itemSize = newsize; //save this for all the later icons which are generated (grid size)
  UpdateGeom();
  //Now re-set the item icon size
  reloadPlugins(true);
}

void LDesktopPluginSpace::cleanup(){
  //Perform any final cleanup actions here
  for(int i=0; i<ITEMS.length(); i++){
    delete ITEMS.takeAt(i);
    i--;
  }
  plugins.clear();
  deskitems.clear();
}
// ===================
//      PUBLIC SLOTS
// ===================
void LDesktopPluginSpace::UpdateGeom(){
  if(DEBUG){ qDebug() << "Update Desktop Geom:"; }
  //Currently no special checks - might need to add validation of all current plugin geometries in the future
}

// ===================
//          PRIVATE
// ===================
QSize LDesktopPluginSpace::calculateItemSize(int icosize){
  //Note: This returns the size in numbers of cells (width = columnspan, height = rowspan)
  QSize sz;
    sz.setWidth(1.8*icosize);
    sz.setWidth( RoundUp(sz.width()/GRIDSIZE)); //always round up to cell numbers
    sz.setHeight(icosize+ 2.1*this->fontMetrics().height() );
    sz.setHeight( RoundUp(sz.height()/GRIDSIZE)); //always round up to cell number
  return sz;
}

void LDesktopPluginSpace::addDesktopItem(QString filepath){
  addDesktopPlugin("applauncher::"+filepath+"---dlink"+QString::number(LSession::handle()->desktop()->screenNumber(this)) );
}

void LDesktopPluginSpace::addDesktopPlugin(QString plugID){
  //This is used for generic plugins (QWidget-based)
  if(DEBUG){ qDebug() << "Adding Desktop Plugin:" << plugID; }
  LDPlugin *plug = NewDP::createPlugin(plugID, this);
    plug->setWhatsThis(plugID);
  //Now get the geometry for the plugin
  QRect geom = plug->loadPluginGeometry(); //in grid coords
  if(geom.isNull()){
    //No previous location - need to calculate initial geom
    QSize sz = plug->sizeHint();
    if(plugID.startsWith("applauncher::") ){ sz = itemSize*GRIDSIZE; }
    geom.setWidth( RoundUp(sz.width()/GRIDSIZE) );
    geom.setHeight( RoundUp(sz.height()/GRIDSIZE) );
    geom.moveTo( findOpenSpot(geom.width(), geom.height()) );
  }else if(!ValidGeometry(plugID, gridToGeom(geom)) ){
    //Find a new location for the plugin (saved location is invalid)
    QPoint pt = findOpenSpot(geom.width(), geom.height(), geom.y()-2, geom.x()-2); //try to get it within the same general area
    geom.moveTo(pt);
  }
  if(geom.x() < 0 || geom.y() < 0){
    qDebug() << "No available space for desktop plugin:" << plugID << " - IGNORING";
    delete plug;
  }else{
    if(DEBUG){ qDebug() <<  " - New Plugin Geometry (grid):" << geom; }
    //Now place the item in the proper spot/size
    plug->setGeometry( gridToGeom(geom) );
    plug->show();
    if(DEBUG){ qDebug() << " - New Plugin Geometry (px):" << plug->geometry(); }
    ITEMS << plug;
    connect(plug, SIGNAL(StartMoving(QString)), this, SLOT(StartItemMove(QString)) );
    connect(plug, SIGNAL(StartResizing(QString)), this, SLOT(StartItemResize(QString)) );
    connect(plug, SIGNAL(RemovePlugin(QString)), this, SLOT(RemoveItem(QString)) );
    connect(plug, SIGNAL(IncreaseIconSize()), this, SIGNAL(IncreaseIcons()) );
    connect(plug, SIGNAL(DecreaseIconSize()), this, SIGNAL(DecreaseIcons()) );
  }
}

QPoint LDesktopPluginSpace::findOpenSpot(int gridwidth, int gridheight, int startRow, int startCol){
  //Note about the return QPoint: x() is the column number, y() is the row number
  QPoint pt(0,0);
  int row = startRow; int col = startCol;
  if(row<0){ row = 0; } //just in case - since this can be recursively called
  if(col<0){ col = 0; } //just in case - since this can be recursively called
  bool found = false;
  int rowCount, colCount;
  rowCount = RoundUp(this->height()/GRIDSIZE);
  colCount = RoundUp(this->width()/GRIDSIZE);
  QRect geom(0, 0, gridwidth*GRIDSIZE, gridheight*GRIDSIZE); //origin point will be adjusted in a moment
  if(DEBUG){ qDebug() << "Search for plugin space:" << rowCount << colCount << gridheight << gridwidth << this->size(); }
  if(TopToBottom){
    //Arrange Top->Bottom
    while(col<(colCount-gridwidth) && !found){
      while(row<(rowCount-gridheight) && !found){
        bool ok = true;
        geom.moveTo(col*GRIDSIZE, row*GRIDSIZE);
	//qDebug() << " - Check Geom:" << geom << col << row;
        //Check all the existing items to ensure no overlap
        for(int i=0; i<ITEMS.length() && ok; i++){
          if(geom.intersects(ITEMS[i]->geometry())){
            //Collision - move the next searchable row/column index
	    ok = false;
	    row = posToGrid(ITEMS[i]->geometry().bottomLeft()).y(); //use bottom edge for next search
	  }
        }
        if(ok){ pt = QPoint(col,row); found = true; } //found an open spot
        //else{ row++; }
      }
      if(!found){ col++; row=0; } //go to the next column
    }	  
  }else{
    //Arrange Left->Right
    while(row<(rowCount-gridheight) && !found){
      while(col<(colCount-gridwidth) && !found){
        bool ok = true;
        geom.moveTo(col*GRIDSIZE, row*GRIDSIZE);
        //Check all the existing items to ensure no overlap
        for(int i=0; i<ITEMS.length() && ok; i++){
          if(geom.intersects(ITEMS[i]->geometry())){
            //Collision - move the next searchable row/column index
	    ok = false;
	    col = posToGrid(ITEMS[i]->geometry().topRight()).x(); // Fill according to row/column
	  }
        }
        if(ok){ pt = QPoint(col,row); found = true; } //found an open spot
        //else{ col++; }
      }
      if(!found){ row++; col=0;} //go to the next row
    }
  }
  if(!found){
    //Decrease the size of the item by 1x1 grid points and try again
    if(gridwidth>2 && gridheight>2){
      pt = findOpenSpot(gridwidth-1, gridheight-1, 0, 0);
    }else{
      pt.setX(-1); pt.setY(-1); //invalid 
      qDebug() << "Could not find an open spot for a desktop plugin:" << gridwidth << gridheight << startRow << startCol;
    }
  }
  return pt;
}

// ===================
//     PRIVATE SLOTS
// ===================
void LDesktopPluginSpace::reloadPlugins(bool ForceIconUpdate ){
  //Remove any plugins as necessary
  QStringList plugs = plugins;
  QStringList items = deskitems;
  for(int i=0; i<ITEMS.length(); i++){
    if(plugs.contains(ITEMS[i]->whatsThis())){ plugs.removeAll(ITEMS[i]->whatsThis()); }
    else if(ITEMS[i]->whatsThis().contains("---dlink") && items.contains(ITEMS[i]->whatsThis().section("---",0,0).section("::",1,50)) ){ 
      //Account for the variation in the Plugin ID for desktop files
      if(ForceIconUpdate){ 
	//Change the size of the existing plugin - preserving the location if possible
	QRect geom = ITEMS[i]->loadPluginGeometry();
	if(!geom.isNull()){
	  geom.setSize(itemSize); //Reset back to default size (does not change location)
	  ITEMS[i]->savePluginGeometry(geom);
	}
	//Now remove the plugin for the moment - run it through the re-creation routine below
	delete ITEMS.takeAt(i);  
	i--;
      }else{
        items.removeAll(ITEMS[i]->whatsThis().section("---",0,0).section("::",1,50));
      }

    }else{ ITEMS[i]->removeSettings(true); delete ITEMS.takeAt(i);  i--; } //this is considered a permanent removal (cleans settings)
  }
  
  //Now create any new items
  //First load the plugins (almost always have fixed locations)
  for(int i=0; i<plugs.length(); i++){
    addDesktopPlugin(plugs[i]);
  }
  //Now load the desktop shortcuts (fill in the gaps as needed)
  for(int i=0; i<items.length(); i++){
    addDesktopItem(items[i]);
  }
}
