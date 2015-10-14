//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This is a couple simple widget subclasses to enable drag and drop functionality
// NOTE: The "whatsThis()" item information needs to correspond to the "[cut/copy]::::<file path>" syntax
//NOTE2: The "whatsThis()" information on the widget itself should be the current dir path *if* it can accept drops
//===========================================
#ifndef _LUMINA_FM_DRAG_DROP_WIDGETS_H
#define _LUMINA_FM_DRAG_DROP_WIDGETS_H

#define MIME QString("x-special/lumina-copied-files")

#include <QListWidget>
#include <QTreeWidget>
#include <QDropEvent>
#include <QMimeData>
#include <QDrag>
#include <QFileInfo>
#include <QDebug>
#include <QMouseEvent>
#include <QUrl>
#include <QDir>

//==============
//  LIST WIDGET
//==============
class DDListWidget : public QListWidget{
	Q_OBJECT
public:
	DDListWidget(QWidget *parent=0) : QListWidget(parent){
	  //Drag and Drop Properties
	  this->setDragDropMode(QAbstractItemView::DragDrop);
	  this->setDefaultDropAction(Qt::MoveAction); //prevent any built-in Qt actions - the class handles it
	  //Other custom properties necessary for the FM
	  this->setFocusPolicy(Qt::StrongFocus);
	  this->setContextMenuPolicy(Qt::CustomContextMenu);
	  this->setSelectionMode(QAbstractItemView::ExtendedSelection);
	  this->setSelectionBehavior(QAbstractItemView::SelectRows);
	  this->setFlow(QListView::TopToBottom);
	  this->setWrapping(true);
	  this->setMouseTracking(true);
	  //this->setSortingEnabled(true); //This sorts *only* by name - type is not preserved
	}
	~DDListWidget(){}

signals:
	void DataDropped(QString, QStringList); //Dir path, List of commands
		
protected:
	void startDrag(Qt::DropActions act){
	  QList<QListWidgetItem*> items = this->selectedItems();
	  if(items.length()<1){ return; }
	  QList<QUrl> urilist;
	  for(int i=0; i<items.length(); i++){ 
	    urilist << QUrl::fromLocalFile(items[i]->whatsThis().section("::::",1,100));	  
	  }
	  //Create the mime data
	  QMimeData *mime = new QMimeData;
	    mime->setUrls(urilist);
	  //Create the drag structure
	  QDrag *drag = new QDrag(this);
	  drag->setMimeData(mime);
	  /*if(info.first().section("::::",0,0)=="cut"){
	    drag->exec(act | Qt::MoveAction);
	  }else{*/
	    drag->exec(act | Qt::CopyAction);
	  //}
	}

	void dragEnterEvent(QDragEnterEvent *ev){
	  //qDebug() << "Drag Enter Event:" << ev->mimeData()->hasFormat(MIME);
	  if(ev->mimeData()->hasUrls() && !this->whatsThis().isEmpty() ){
	    ev->acceptProposedAction(); //allow this to be dropped here
	  }else{
	    ev->ignore();
	  }		  
	}
	
	void dragMoveEvent(QDragMoveEvent *ev){
	  if(ev->mimeData()->hasUrls() && !this->whatsThis().isEmpty() ){
	    ev->acceptProposedAction(); //allow this to be dropped here
	  }else{
	    ev->ignore();
	  }
	}
	
	void dropEvent(QDropEvent *ev){
	  if(this->whatsThis().isEmpty() || !ev->mimeData()->hasUrls() ){ ev->ignore(); return; } //not supported
	  //qDebug() << "Drop Event:";
	  ev->accept(); //handled here
	  QString dirpath = this->whatsThis();
	  //See if the item under the drop point is a directory or not
	  QListWidgetItem *it = this->itemAt( ev->pos());
	  if(it!=0){
	    QFileInfo info(it->whatsThis().section("::::",1,100));
	    if(info.isDir() && info.isWritable()){
	      dirpath = info.absoluteFilePath();
	    }
	  }
	  //Now turn the input urls into local file paths
	  QStringList files;
	  QString home = QDir::homePath();
	  foreach(const QUrl &url, ev->mimeData()->urls()){
	    const QString filepath = url.toLocalFile();
	    //If the target file is modifiable, assume a move - otherwise copy
	    if(QFileInfo(filepath).isWritable() && (filepath.startsWith(home) && dirpath.startsWith(home))){ files << "cut::::"+filepath; }
	    else{ files << "copy::::"+filepath; }
	  }
	  //qDebug() << "Drop Event:" << dirpath;
	  emit DataDropped( dirpath, files );
	}
	
	void mouseReleaseEvent(QMouseEvent *ev){
	  if(ev->button() != Qt::RightButton && ev->button() != Qt::LeftButton){ ev->ignore(); }
	  else{ QListWidget::mouseReleaseEvent(ev); } //pass it along to the widget
	}
	void mousePressEvent(QMouseEvent *ev){
	  if(ev->button() != Qt::RightButton && ev->button() != Qt::LeftButton){ ev->ignore(); }
	  else{ QListWidget::mousePressEvent(ev); } //pass it along to the widget	  
	}
	/*void mouseMoveEvent(QMouseEvent *ev){
	  if(ev->button() != Qt::RightButton && ev->button() != Qt::LeftButton){ ev->ignore(); }
	  else{ QListWidget::mouseMoveEvent(ev); } //pass it along to the widget		
	}*/
};

//================
//     TreeWidget
//================
class DDTreeWidget : public QTreeWidget{
	Q_OBJECT
public:
	DDTreeWidget(QWidget *parent=0) : QTreeWidget(parent){
	  //Drag and Drop Properties
	  this->setDragDropMode(QAbstractItemView::DragDrop);
	  this->setDefaultDropAction(Qt::MoveAction); //prevent any built-in Qt actions - the class handles it
	  //Other custom properties necessary for the FM
	  this->setFocusPolicy(Qt::StrongFocus);
	  this->setContextMenuPolicy(Qt::CustomContextMenu);
	  this->setSelectionMode(QAbstractItemView::ExtendedSelection);
	  this->setSelectionBehavior(QAbstractItemView::SelectRows);
	  this->setMouseTracking(true);
	  this->setSortingEnabled(true);
	  this->setIndentation(0);
	  this->setItemsExpandable(false);
	}
	~DDTreeWidget(){}

signals:
	void DataDropped(QString, QStringList); //Dir path, List of commands
		
protected:
	void startDrag(Qt::DropActions act){
	  QList<QTreeWidgetItem*> items = this->selectedItems();
	  if(items.length()<1){ return; }
	  QList<QUrl> urilist;
	  for(int i=0; i<items.length(); i++){ 
	    urilist << QUrl::fromLocalFile(items[i]->whatsThis(0).section("::::",1,100));	  
	  }
	  //Create the mime data
	  QMimeData *mime = new QMimeData;
	    mime->setUrls(urilist);
	  //Create the drag structure
	  QDrag *drag = new QDrag(this);
	  drag->setMimeData(mime);
	  /*if(info.first().section("::::",0,0)=="cut"){
	    drag->exec(act | Qt::MoveAction);
	  }else{*/
	    drag->exec(act | Qt::CopyAction| Qt::MoveAction);
	  //}
	}

	void dragEnterEvent(QDragEnterEvent *ev){
	  //qDebug() << "Drag Enter Event:" << ev->mimeData()->hasFormat(MIME);
	  if(ev->mimeData()->hasUrls() && !this->whatsThis().isEmpty() ){
	    ev->acceptProposedAction(); //allow this to be dropped here
	  }else{
	    ev->ignore();
	  }		  
	}
	
	void dragMoveEvent(QDragMoveEvent *ev){
	  if(ev->mimeData()->hasUrls() && !this->whatsThis().isEmpty() ){
	    ev->acceptProposedAction(); //allow this to be dropped here
	  }else{
	    ev->ignore();
	  }
	}
	
	void dropEvent(QDropEvent *ev){
	  if(this->whatsThis().isEmpty() || !ev->mimeData()->hasUrls() ){ ev->ignore(); return; } //not supported
	  ev->accept(); //handled here
	  QString dirpath = this->whatsThis();
	  //See if the item under the drop point is a directory or not
	  QTreeWidgetItem *it = this->itemAt( ev->pos());
	  if(it!=0){
	    QFileInfo info(it->whatsThis(0).section("::::",1,100));
	    if(info.isDir() && info.isWritable()){
	      dirpath = info.absoluteFilePath();
	    }
	  }
	  //qDebug() << "Drop Event:" << dirpath;
	  //Now turn the input urls into local file paths
	  QStringList files;
	  QString home = QDir::homePath();
	  foreach(const QUrl &url, ev->mimeData()->urls()){
	    const QString filepath = url.toLocalFile();
	    //If the target file is modifiable, assume a move - otherwise copy
	    if(QFileInfo(filepath).isWritable() && (filepath.startsWith(home) && dirpath.startsWith(home)) ){ files << "cut::::"+filepath; }
	    else{ files << "copy::::"+filepath; }
	  }
	  //qDebug() << "Drop Event:" << dirpath;
	  emit DataDropped( dirpath, files );
	}
	
	void mouseReleaseEvent(QMouseEvent *ev){
	  if(ev->button() != Qt::RightButton && ev->button() != Qt::LeftButton){ ev->ignore(); }
	  else{ QTreeWidget::mouseReleaseEvent(ev); } //pass it along to the widget
	}
	void mousePressEvent(QMouseEvent *ev){
	  if(ev->button() != Qt::RightButton && ev->button() != Qt::LeftButton){ ev->ignore(); }
	  else{ QTreeWidget::mousePressEvent(ev); } //pass it along to the widget	  
	}
	/*void mouseMoveEvent(QMouseEvent *ev){
	  if(ev->button() != Qt::RightButton && ev->button() != Qt::LeftButton){ ev->ignore(); }
	  else{ QTreeWidget::mouseMoveEvent(ev); } //pass it along to the widget		
	}*/
};
#endif