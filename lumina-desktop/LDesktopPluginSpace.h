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

#include "desktop-plugins/LDPlugin.h"

#define MIMETYPE QString("x-special/lumina-desktop-plugin")
//#define MIMEPOS QString("x-special/lumina-desktop-plugin-pos")
#define GRIDSIZE 16.0 //Need this to be a double/float - usually used for divisions

class LDesktopPluginSpace : public QWidget{
  	Q_OBJECT
	
signals:
	void PluginRemovedByUser(QString ID);
	
public:
	LDesktopPluginSpace(QWidget *parent = 0);
	~LDesktopPluginSpace();

	void LoadItems(QStringList plugs, QStringList files);
	//void setShowGrid(bool show); This is already implemented in QTableView (inherited)
	void SetIconSize(int size);
	void ArrangeTopToBottom(bool ttb); //if false, will arrange left->right
	void cleanup();

public slots:
	void UpdateGeom();

private:
	QSettings *plugsettings;
	QSize itemSize;
	QStringList plugins, deskitems;	
	QList<LDPlugin*> ITEMS;
	bool TopToBottom;

	int RoundUp(double num){
	 int out = num; //This will truncate the number
	 if(out < num){ out++; } //need to increase by 1
	 return out;
	}

	QSize calculateItemSize(int icosize);
	void addDesktopItem(QString filepath); //This will convert it into a valid Plugin ID
	void addDesktopPlugin(QString plugID);


	QPoint findOpenSpot(int gridwidth = 1, int gridheight = 1, int startRow = 0, int startCol = 0);
	
	QPoint posToGrid(QPoint pos){
	  //This assumes a point in widget-relative coordinates
	  pos.setX( RoundUp(pos.x()/GRIDSIZE));
	  pos.setY( RoundUp(pos.y()/GRIDSIZE));
	  return pos;
	}
	
	QRect geomToGrid(QRect geom){
	  return QRect( RoundUp(geom.x()/GRIDSIZE), RoundUp(geom.y()/GRIDSIZE), \
			RoundUp(geom.width()/GRIDSIZE), RoundUp(geom.height()/GRIDSIZE) );
		
	}
	QRect gridToGeom(QRect grid){
	  //This function incorporates the bottom/right edge matchins procedures (for incomplete last grid)
	  QRect geom(grid.x()*GRIDSIZE, grid.y()*GRIDSIZE, grid.width()*GRIDSIZE, grid.height()*GRIDSIZE);
	  //Now check the edge conditions (last right/bottom grid points might be smaller than GRIDSIZE)
	  //qDebug() << "GridToGeom:" << grid << geom;
	  if(geom.right() > this->geometry().right() && (geom.right() -this->geometry().right())<GRIDSIZE ){
	    geom.setRight(this->geometry().right()); //match up with the edge
	  }
	  if(geom.bottom() > this->geometry().bottom() && (geom.bottom() -this->geometry().bottom())<GRIDSIZE ){
	    geom.setBottom(this->geometry().bottom()); //match up with the edge
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
	
	bool ValidGeometry(QString id, QRect geom){
	  //First check that it is within the desktop area completely
	  // Note that "this->geometry()" is not in the same coordinate space as the geometry inputs
	  if(!QRect(0,0,this->width(), this->height()).contains(geom)){ return false; }
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
	
private slots:
	void reloadPlugins();

	void StartItemMove(QString ID){
	  setupDrag(ID, "move");
	}
	void StartItemResize(QString ID){
	  setupDrag(ID, "resize");
	}
	void RemoveItem(QString ID){
	  for(int i=0; i<ITEMS.length(); i++){
	    if(ITEMS[i]->whatsThis()==ID){
	      ITEMS[i]->Cleanup();
	      delete ITEMS.takeAt(i);
	      emit PluginRemovedByUser(ID);
	      return;
	    }
	  }
	}

protected:
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
		  geom.moveTo( (geom.topLeft()+diff) );
		  geom = gridToGeom(geom); //convert back to px coords with edge matching
		  //qDebug() << " - Setting Geometry:" << geom;
		  if(ValidGeometry(act.section("::::",1,50), geom)){ 
		    item->setGeometry(geom); 
		    ev->acceptProposedAction(); 
		    item->savePluginGeometry(geomToGrid(geom));
		  }else{ ev->ignore(); } //invalid location
		  
	      }else{
		//Resize operation
		QPoint diff = grid - posToGrid(geom.center());; //need difference from center (grid)
		geom = geomToGrid(geom); //convert to grid coordinates now
		if(diff.x()==0 && diff.y()==0){
		  ev->acceptProposedAction(); //nothing to do - but keep the drag active
		}else{
		  if(diff.x()<0){ geom.setLeft( grid.x()); } //expanding to the left
		  else if(diff.x()>0){ geom.setRight( grid.x()); } //expanding to the right
		  if(diff.y()<0){ geom.setTop( grid.y()); } //expanding above
		  else if(diff.y()>0){ geom.setBottom( grid.y()); } //expanding below
		  //Now convert back to pixel coords (includes edge matching/adjustments)
		  geom = gridToGeom(geom);
		  //Check Validity of new geom
		  if(ValidGeometry(act.section("::::",1,50), geom)){ 
		    item->setGeometry(geom); 
		    ev->acceptProposedAction();
		    item->savePluginGeometry(geomToGrid(geom));
	          }else{ ev->ignore(); } //invalid location
	        }
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