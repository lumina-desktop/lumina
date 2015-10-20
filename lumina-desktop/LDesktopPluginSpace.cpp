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
  GRIDSIZE = 100.0; //default value if not set
  plugsettings = LSession::handle()->DesktopPluginSettings();

}

LDesktopPluginSpace::~LDesktopPluginSpace(){
  
}

void LDesktopPluginSpace::LoadItems(QStringList plugs, QStringList files){
  if(DEBUG){ qDebug() << "Loading Desktop Items:" << plugs << files << "Area:" << this->size() << GRIDSIZE; }
  bool changes = false;
  if(plugs != plugins){ plugins = plugs; changes = true; }
  if(files != deskitems){ deskitems = files; changes = true; }
  if(changes){ QTimer::singleShot(0,this, SLOT(reloadPlugins())); }
}

void LDesktopPluginSpace::SetIconSize(int size){
  if(DEBUG){ qDebug() << "Set Desktop Icon Size:" << size; }
  //QSize newsize = calculateItemSize(size);
  int oldsize = GRIDSIZE;
  GRIDSIZE = size; //turn the int into a float;
  //itemSize = QSize(1,1); //save this for all the later icons which are generated (grid size)
  UpdateGeom(oldsize);
  //Now re-set the item icon size
  //reloadPlugins(true);
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
void LDesktopPluginSpace::UpdateGeom(int oldgrid){
  if(DEBUG){ qDebug() << "Updated Desktop Geom:" << this->size() << GRIDSIZE << this->size()/GRIDSIZE; }
  //Go through and check the locations/sizes of all items (particularly the ones on the bottom/right edges)
  //bool reload = false;
  for(int i=0; i<ITEMS.length(); i++){
    QRect grid = geomToGrid(ITEMS[i]->geometry(), oldgrid);
    if(DEBUG){ qDebug() << " - Check Plugin:" << ITEMS[i]->whatsThis() << grid; }
    if( !ValidGrid(grid) ){
      //This plugin is too far out of the screen - find new location for it
      if(DEBUG){ qDebug() << " -- Out of bounds - Find a new spot"; }
      grid = findOpenSpot(grid.width(), grid.height(), grid.y()-1, grid.x()-1); //try to get a nearby spot first
    }
    if(!ValidGrid(grid)){
      qDebug() << "No Place for plugin:" << ITEMS[i]->whatsThis();
      qDebug() << " - Removing it for now...";
      delete ITEMS.takeAt(i);
      i--;
    }else{
      //NOTE: We are not doing the ValidGeometry() checks because we are only resizing existing plugin with pre-set & valid grid positions
      grid = gridToGeom(grid); //convert to pixels before saving/sizing
      ITEMS[i]->setGeometry( grid );
      ITEMS[i]->setFixedSize(grid.size());
      ITEMS[i]->savePluginGeometry(grid);
    }
  }
  //if(reload){ QTimer::singleShot(0,this, SLOT(reloadPlugins())); }
}

// ===================
//          PRIVATE
// ===================
void LDesktopPluginSpace::addDesktopItem(QString filepath){
  addDesktopPlugin("applauncher::"+filepath+"---dlink"+QString::number(LSession::handle()->desktop()->screenNumber(this)) );
}

void LDesktopPluginSpace::addDesktopPlugin(QString plugID){
  //This is used for generic plugins (QWidget-based)
  if(DEBUG){ qDebug() << "Adding Desktop Plugin:" << plugID; }
  LDPlugin *plug = NewDP::createPlugin(plugID, this);
    plug->setWhatsThis(plugID);
  //Now get the geometry for the plugin
  QRect geom = plug->loadPluginGeometry(); //in pixel coords
  if(!geom.isNull()){ geom = geomToGrid(geom); } //convert to grid coordinates
  if(geom.isNull()){
    //No previous location - need to calculate initial geom
    QSize sz = plug->defaultPluginSize(); //in grid coordinates
    geom.setSize(sz);
    geom = findOpenSpot(geom.width(), geom.height() );
  }else if(!ValidGeometry(plugID, gridToGeom(geom)) ){
    //Find a new location for the plugin (saved location is invalid)
    geom = findOpenSpot(geom.width(), geom.height(), geom.y()-1, geom.x()-1); //try to get it within the same general area
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

QRect LDesktopPluginSpace::findOpenSpot(int gridwidth, int gridheight, int startRow, int startCol){
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
    
    if(startRow!=0 || startCol!=0){
      //Did not check the entire screen yet - try that first
      return findOpenSpot(gridwidth, gridheight, 0,0);
    }else if(gridwidth>1 && gridheight>1){
      //Decrease the size of the item by 1x1 grid points and try again
      return findOpenSpot(gridwidth-1, gridheight-1, 0, 0);
    }else{
      qDebug() << "Could not find an open spot for a desktop plugin:" << gridwidth << gridheight << startRow << startCol;
      return QRect(-1,-1,-1,-1);
    }
  }else{
    return QRect(pt,QSize(gridwidth,gridheight));
  }
}

// ===================
//     PRIVATE SLOTS
// ===================
void LDesktopPluginSpace::reloadPlugins(bool ForceIconUpdate ){
  //Remove any plugins as necessary
  QStringList plugs = plugins;
  QStringList items = deskitems;
  for(int i=0; i<ITEMS.length(); i++){
    
    if( ITEMS[i]->whatsThis().startsWith("applauncher") && ForceIconUpdate){ 
	//Change the size of the existing plugin - preserving the location if possible
	/*QRect geom = ITEMS[i]->loadPluginGeometry(); //pixel coords
	if(!geom.isNull()){
	  geom = geomToGrid(geom); //convert to grid coords
	  geom.setSize(itemSize); //Reset back to default size (does not change location)
	  ITEMS[i]->savePluginGeometry( gridToGeom(geom)); //save it back in pixel coords
	}*/
	//Now remove the plugin for the moment - run it through the re-creation routine below
	delete ITEMS.takeAt(i);  
	i--;
    }
    else if(plugs.contains(ITEMS[i]->whatsThis())){ plugs.removeAll(ITEMS[i]->whatsThis()); }
    else if(items.contains(ITEMS[i]->whatsThis().section("---",0,0).section("::",1,50))){ items.removeAll(ITEMS[i]->whatsThis().section("---",0,0).section("::",1,50)); }
    else{ ITEMS[i]->removeSettings(true); delete ITEMS.takeAt(i);  i--; } //this is considered a permanent removal (cleans settings)
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
