//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_LDESKTOP_PLUGIN_SPACE_H
#define _LUMINA_DESKTOP_LDESKTOP_PLUGIN_SPACE_H

#include <QListWidget>
#include <QDropEvent>
#include <QDrag> //includes all the QDrag*Event classes
#include <QUrl>
#include <QMimeData>
#include <QSettings>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QProcess>

#include "desktop-plugins/LDPlugin.h"

#define MIMETYPE QString("x-special/lumina-desktop-plugin")

class LDesktopPluginSpace : public QWidget{
  	Q_OBJECT
	
signals:
	void PluginRemovedByUser(QString ID);
	void IncreaseIcons(); //increase default icon sizes
	void DecreaseIcons(); //decrease default icon sizes
	void HideDesktopMenu();
	
public:
	LDesktopPluginSpace();
	~LDesktopPluginSpace();

	void LoadItems(QStringList plugs, QStringList files);
	//void setShowGrid(bool show); This is already implemented in QTableView (inherited)
	void SetIconSize(int size);
	void ArrangeTopToBottom(bool ttb); //if false, will arrange left->right
	void cleanup();

	void setBackground(QPixmap pix); //should already be sized appropriately for this widget
	void setDesktopArea(QRect area);

public slots:
	void UpdateGeom(int oldgrid = -1);

private:
	QSettings *plugsettings;
	QStringList plugins, deskitems;	
	QList<LDPlugin*> ITEMS;
	QPixmap wallpaper;
	QRect desktopRect;
	bool TopToBottom;
	float GRIDSIZE;

	int RoundUp(double num){
	 int out = num; //This will truncate the number
	 if(out < num){ out++; } //need to increase by 1
	  //qDebug() << "Round Up:" << num << "->" << out;
	 return out;
	}

	void addDesktopItem(QString filepath); //This will convert it into a valid Plugin ID automatically
	void addDesktopPlugin(QString plugID);


	QRect findOpenSpot(int gridwidth = 1, int gridheight = 1, int startRow = 0, int startCol = 0, bool reversed = false, QString plugID = "");
	QRect findOpenSpot(QRect grid, QString plugID, bool recursive = false);
	
	QPoint posToGrid(QPoint pos){
	  pos.setX( RoundUp((pos.x()-desktopRect.x())/GRIDSIZE));
	  pos.setY( RoundUp((pos.y()-desktopRect.y())/GRIDSIZE));
	  return pos;
	}

	QPoint gridToPos(QPoint grid){
	  grid.setX( (grid.x()*GRIDSIZE)+desktopRect.x() );
	  grid.setY( (grid.y()*GRIDSIZE)+desktopRect.y() );
	  return grid;
	}
	
	QRect geomToGrid(QRect geom, int grid = -1){
	  if(grid<0){ 
	    //use the current grid size
	   return QRect( RoundUp((geom.x()-desktopRect.x())/GRIDSIZE), RoundUp((geom.y()-desktopRect.y())/GRIDSIZE), \
			RoundUp(geom.width()/GRIDSIZE), RoundUp(geom.height()/GRIDSIZE) );
	    //qDebug() << "Geom to Grid:" << geom << desktopRect << tmp << GRIDSIZE;
	    //return tmp;
	  }else{
	    //use the input grid size
	    return QRect( RoundUp((geom.x()-desktopRect.x())/((double) grid)), RoundUp((geom.y()-desktopRect.y())/((double) grid)), \
			RoundUp(geom.width()/((double) grid)), RoundUp(geom.height()/((double) grid)) );
	  }
	}
	
	QRect gridToGeom(QRect grid){
	  //This function incorporates the bottom/right edge matchins procedures (for incomplete last grid)
	  QRect geom((grid.x()*GRIDSIZE)+desktopRect.x(), (grid.y()*GRIDSIZE)+desktopRect.y(), grid.width()*GRIDSIZE, grid.height()*GRIDSIZE);
	  //Now check the edge conditions (last right/bottom grid points might be smaller than GRIDSIZE)
	  //QSize areaSize = desktopRect.size();
	  //qDebug() << "GridToGeom:" << grid << geom << "Area size:" << areaSize;
	  if(geom.right() > desktopRect.right() && (geom.right()-desktopRect.right())<GRIDSIZE ){
	    geom.setRight(desktopRect.right()); //match up with the edge
	  }
	  if(geom.bottom() > desktopRect.bottom() && (geom.bottom() -desktopRect.bottom())<GRIDSIZE ){
	    geom.setBottom(desktopRect.bottom()); //match up with the edge
	  }
	  //qDebug() << " - Adjusted:" << geom;
	  return geom;
	}
	
	//Internal simplification for setting up a drag event
	void setupDrag(QString id, QString type){
	  QMimeData *mime = new QMimeData;
	    mime->setData(MIMETYPE, QString(type+"::::"+id).toLocal8Bit() );
	  //If this is a desktop file - also add it to the generic URI list mimetype
	  if(id.startsWith("applauncher::")){
	    QList<QUrl> urilist;
	      urilist << QUrl::fromLocalFile( id.section("---",0,0).section("::",1,50) );	  
	    mime->setUrls(urilist);
	  }
	  //Create the drag structure
	  QDrag *drag = new QDrag(this);
	  drag->setMimeData(mime);
	    drag->exec(Qt::CopyAction);	
	}
	
	bool ValidGrid(QRect grid){
	  //qDebug() << "Check Valid Grid:" << grid << RoundUp(this->width()/GRIDSIZE) << RoundUp(this->height()/GRIDSIZE);
	  //This just checks that the grid coordinates are not out of bounds - should still run ValidGeometry() below with the actual pixel geom
	  if(grid.x()<0|| grid.y()<0 || grid.width()<0 || grid.height()<0){ return false; }
	  else if( (grid.x()+grid.width()) > RoundUp(desktopRect.width()/GRIDSIZE) ){ return false; }
	  else if( (grid.y()+grid.height()) > RoundUp(desktopRect.height()/GRIDSIZE) ){ return false; }
	  //Final Check - don't let 1x1 items occupy the last row/column (not full size)
	  else if(grid.width()==1 && grid.height()==1 && (grid.x()==RoundUp(desktopRect.width()/GRIDSIZE) || grid.y()==RoundUp(desktopRect.height()/GRIDSIZE)) ){ return false; }
	  return true;
	}
	
	bool ValidGeometry(QString id, QRect geom){
	  //First check that it is within the desktop area completely
	  // Note that "this->geometry()" is not in the same coordinate space as the geometry inputs
	  if(!desktopRect.contains(geom)){ return false; }
	  //Now check that it does not collide with any other items
	  for(int i=0; i<ITEMS.length(); i++){
	    if(ITEMS[i]->whatsThis()==id){ continue; }
	    else if(geom.intersects(ITEMS[i]->geometry())){ return false; }
	  }
	  return true;
	}
	
	LDPlugin* ItemFromID(QString ID){
	  for(int i=0; i<ITEMS.length(); i++){
	    if(ITEMS[i]->whatsThis()==ID){ return ITEMS[i]; }
	  }
	  return 0;
	}
	
	void MovePlugin(LDPlugin* plug, QRect geom){
	  plug->savePluginGeometry(geom);	//save the un-adjusted geometry
	  plug->setGridGeometry(geomToGrid(geom)); //save the actual grid location
	  plug->setGeometry( geom );
	  plug->setFixedSize(geom.size()); //needed for some plugins
	}
	
private slots:
	void reloadPlugins(bool ForceIconUpdate = false);

	void StartItemMove(QString ID){
	  setupDrag(ID, "move");
	}
	void StartItemResize(QString ID){
	  setupDrag(ID, "resize");
	}
	void RemoveItem(QString ID){
	  //Special case - desktop file/dir link using the "applauncher" plugin	
	  if(ID.startsWith("applauncher::")){
	    QFileInfo info(ID.section("---",0,0).section("::",1,50) );
	    if(info.exists() && info.absolutePath()==QDir::homePath()+"/Desktop"){
	      qDebug() << "Deleting Desktop Item:" << info.absoluteFilePath();
	      if(!info.isSymLink() && info.isDir()){ QProcess::startDetached("rm -r \""+info.absoluteFilePath()+"\""); }
              else{ QFile::remove(info.absoluteFilePath()); } //just remove the file/symlink directly  
	      emit PluginRemovedByUser(ID);
	      return;
	    }
	  }
	  //Any other type of plugin
	  for(int i=0; i<ITEMS.length(); i++){
	    if(ITEMS[i]->whatsThis()==ID){
	      ITEMS[i]->Cleanup();
	      ITEMS.takeAt(i)->deleteLater();
	      break;
	    }
	  }
	  emit PluginRemovedByUser(ID);
	}

protected:
	void focusInEvent(QFocusEvent *ev){
	  this->lower(); //make sure we stay on the bottom of the window stack
	  QWidget::focusInEvent(ev); //do normal handling
	}
	void paintEvent(QPaintEvent*ev);

	//Need Drag and Drop functionality (internal movement)
	void dragEnterEvent(QDragEnterEvent *ev){
	  if(ev->mimeData()->hasFormat(MIMETYPE) ){
	    ev->acceptProposedAction(); //allow this to be dropped here
	  }else if(ev->mimeData()->hasUrls()){
  	    ev->acceptProposedAction(); //allow this to be dropped here
	  }else{
	    ev->ignore();
	  }		  
	}
	
	void dragMoveEvent(QDragMoveEvent *ev){
	  if(ev->mimeData()->hasFormat(MIMETYPE) ){
	    //Internal move/resize - Check for validity
	    QString act = QString( ev->mimeData()->data(MIMETYPE) );
	    LDPlugin *item = ItemFromID(act.section("::::",1,50));
	    //qDebug() << "Internal Move Event:" << act << ev->pos();
	    if(item!=0){
	      QRect geom = item->geometry();
	      QPoint grid = posToGrid(ev->pos());
	      if(act.section("::::",0,0)=="move"){
		QPoint diff = grid - posToGrid(geom.center()); //difference in grid coords
		  //qDebug() << "Move Event:" << "Diff:" << diff << "Geom:" << geom << grid << ev->pos();
		  geom = geomToGrid(geom); //convert to grid coords
		  //qDebug() << "Move Event:" << "Old geom (grid):" << geom;
		  geom.moveTo( (geom.topLeft()+diff) );
		  //qDebug() << " - After Move:" << geom;
		  bool valid = ValidGrid(geom);
		  if(valid){
		    //Convert to pixel coordinates and check validity again
		    geom = gridToGeom(geom); //convert back to px coords with edge matching
		    valid = ValidGeometry(act.section("::::",1,50), geom);
		  }
		  if(valid){
		    MovePlugin(item, geom);
		    //item->setGeometry(geom); 
		    //item->setFixedSize(geom.size()); //needed due to resizing limitations and such for some plugins
		    ev->acceptProposedAction(); 
		    //item->savePluginGeometry(geom); //save in pixel coords			  
		  }else{ ev->ignore(); } //invalid location
		  
	      }else{
		//Resize operation
		QPoint diff = ev->pos() - (geom.center()-QPoint(1,1)); //need difference from center (pixels)
		  //Note: Use the 1x1 pixel offset to ensure that the center point is not exactly on a grid point intersection (2x2, 4x4, etc)
		  //qDebug() << "Resize Plugin:" << geom << grid << posToGrid(geom.center()) << diff;
		  geom = geomToGrid(geom); //convert to grid coordinates now
		  //qDebug() << " - Grid Geom:" << geom;
		  if(diff.x()<0){ geom.setLeft(ev->pos().x()/GRIDSIZE); } //expanding to the left (round down)
		  else if(diff.x()>0){ geom.setRight( ev->pos().x()/GRIDSIZE); } //expanding to the right (round down)
		  if(diff.y()<0){ geom.setTop( ev->pos().y()/GRIDSIZE); } //expanding above  (round down)
		  else if(diff.y()>0){ geom.setBottom( ev->pos().y()/GRIDSIZE); } //expanding below (round down)
		  //qDebug() << " - Adjusted:" << geom;
		  bool valid = ValidGrid(geom);
		  if(valid){
		    //Convert to pixel coordinates and check validity again
		    geom = gridToGeom(geom); //convert back to px coords with edge matching
		    valid = ValidGeometry(act.section("::::",1,50), geom);
		  }
		  if(valid){
		    MovePlugin(item, geom);
		    //item->setGeometry(geom); 
		    //item->setFixedSize(geom.size()); //needed due to resizing limitations and such for some plugins
		    ev->acceptProposedAction(); 
		    //item->savePluginGeometry(geom); //save in pixel coords			  
		  }else{ ev->ignore(); } //invalid location
	        }
	    }
	  }else if(ev->mimeData()->hasUrls()){
  	    ev->acceptProposedAction(); //allow this to be dropped here
	  }else{
	    ev->ignore();
	  }
	}
	
	void dropEvent(QDropEvent *ev){
	  //QPoint grid = posToGrid(ev->pos());
	  if(ev->mimeData()->hasFormat(MIMETYPE)){
	    //Desktop Items getting moved around - already performed in the dragMoveEvent
	    ev->accept();
	  }else if(ev->mimeData()->hasUrls()){
	    ev->accept();
	    //Files getting dropped here
	    QList<QUrl> urls = ev->mimeData()->urls();
	    qDebug() << "Desktop Drop Event:" << urls;
	    for(int i=0; i<urls.length(); i++){
	      //If this file is not in the desktop folder, move/copy it here
	      if(urls[i].isLocalFile()){
		QFileInfo info(urls[i].toLocalFile());
		if(info.exists() && !QFile::exists(QDir::homePath()+"/Desktop/"+info.fileName())){
		  //Make a link to the file here
		  QFile::link(info.absoluteFilePath(), QDir::homePath()+"/Desktop/"+info.fileName());
		}else{
		  qWarning() << "Invalid desktop file drop (ignored):" << urls[i].toString();
		}
	      }

	    }
	  }else{
	    //Ignore this event
	    ev->ignore();
	  }
	}

};

#endif
