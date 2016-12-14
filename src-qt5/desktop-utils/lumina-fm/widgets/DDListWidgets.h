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

#include <LUtils.h>

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
	  this->setSortingEnabled(true); //This sorts *only* by name - type is not preserved
	  //this->setStyleSheet("QListWidget::item{ border: 1px solid transparent; border-radius: 5px; background-color: transparent;} QListWidget::item:hover{ border-color: black; } QListWidget::item:focus{ border-color: lightblue; }");
	}
	~DDListWidget(){}

signals:
	void DataDropped(QString, QStringList); //Dir path, List of commands
	void GotFocus();

protected:
	void focusInEvent(QFocusEvent *ev){
	  QListWidget::focusInEvent(ev);
	  emit GotFocus();
	}

	void startDrag(Qt::DropActions act){
	  QList<QListWidgetItem*> items = this->selectedItems();
	  if(items.length()<1){ return; }
	  QList<QUrl> urilist;
	  for(int i=0; i<items.length(); i++){ 
	    urilist << QUrl::fromLocalFile(items[i]->whatsThis());	  
	  }
	  //Create the mime data
	  //qDebug() << "Start Drag:" << urilist;
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
	    //Change the drop type depending on the data/dir
	    QString home = QDir::homePath();
	    //qDebug() << "Drag Move:" << home << this->whatsThis();
	    if( this->whatsThis().startsWith(home) ){ ev->setDropAction(Qt::MoveAction); this->setCursor(Qt::DragMoveCursor); }
	    else{ ev->setDropAction(Qt::CopyAction); this->setCursor(Qt::DragCopyCursor);}
	    ev->acceptProposedAction(); //allow this to be dropped here
	    //this->setCursor(Qt::CrossCursor);
	  }else{
	    this->setCursor(Qt::ForbiddenCursor);
	    ev->ignore();
	  }
	  this->update();
	}
	
	void dropEvent(QDropEvent *ev){
	  if(this->whatsThis().isEmpty() || !ev->mimeData()->hasUrls() ){ ev->ignore(); return; } //not supported
	  //qDebug() << "Drop Event:";
	  ev->accept(); //handled here
	  QString dirpath = this->whatsThis();
	  //See if the item under the drop point is a directory or not
	  QListWidgetItem *it = this->itemAt( ev->pos());
	  if(it!=0){
	    //qDebug() << "Drop Item:" << it->whatsThis();
	    QFileInfo info(it->whatsThis());
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
	    if(QFileInfo(filepath).isWritable() && (filepath.startsWith(home) && dirpath.startsWith(home))){ 
	      if(filepath.section("/",0,-2)!=dirpath){ files << "cut::::"+filepath;  } //don't "cut" a file into the same dir
	    }else{ files << "copy::::"+filepath; }
	  }
	  //qDebug() << "Drop Event:" << dirpath << files;
	  if(!files.isEmpty()){  emit DataDropped( dirpath, files ); }
	  this->setCursor(Qt::ArrowCursor);
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
	void GotFocus();

protected:
	void focusInEvent(QFocusEvent *ev){
	  QTreeWidget::focusInEvent(ev);
	  emit GotFocus();
	}
	void startDrag(Qt::DropActions act){
	  QList<QTreeWidgetItem*> items = this->selectedItems();
	  if(items.length()<1){ return; }
	  QList<QUrl> urilist;
	  for(int i=0; i<items.length(); i++){ 
	    urilist << QUrl::fromLocalFile(items[i]->whatsThis(0));	  
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
	    //Change the drop type depending on the data/dir
	    QString home = QDir::homePath();
	    if( this->whatsThis().startsWith(home) ){ ev->setDropAction(Qt::MoveAction); }
	    else{ ev->setDropAction(Qt::CopyAction); }
	    ev->accept(); //allow this to be dropped here
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
	    QFileInfo info(it->whatsThis(0));
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
	    if(QFileInfo(filepath).isWritable() && (filepath.startsWith(home) && dirpath.startsWith(home))){ 
	      if(filepath.section("/",0,-2)!=dirpath){ files << "cut::::"+filepath;  } //don't "cut" a file into the same dir
	    }else{ files << "copy::::"+filepath; }
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

/*
 * Virtual class for managing the sort of folders/files items. The problem with base class is that it only manages texts fields and
 * we have dates and sizes.
 *
 * On this class, we overwrite the function operator<.
 */

class CQTreeWidgetItem : public QTreeWidgetItem {
public:
    CQTreeWidgetItem(int type = Type) : QTreeWidgetItem(type) {}
    CQTreeWidgetItem(const QStringList & strings, int type = Type) : QTreeWidgetItem(strings, type) {}
    CQTreeWidgetItem(QTreeWidget * parent, int type = Type) : QTreeWidgetItem(parent, type) {}
    CQTreeWidgetItem(QTreeWidget * parent, const QStringList & strings, int type = Type) : QTreeWidgetItem(parent, strings, type) {}
    CQTreeWidgetItem(QTreeWidget * parent, QTreeWidgetItem * preceding, int type = Type) : QTreeWidgetItem(parent, preceding, type) {}
    CQTreeWidgetItem(QTreeWidgetItem * parent, int type = Type) : QTreeWidgetItem(parent, type) {}
    CQTreeWidgetItem(QTreeWidgetItem * parent, const QStringList & strings, int type = Type) : QTreeWidgetItem(parent, strings, type) {}
    CQTreeWidgetItem(QTreeWidgetItem * parent, QTreeWidgetItem * preceding, int type = Type) : QTreeWidgetItem(parent, preceding, type) {}
    virtual ~CQTreeWidgetItem() {}
    inline virtual bool operator<(const QTreeWidgetItem &tmp) const {
      int column = this->treeWidget()->sortColumn();
      // We are in date text
      if(column == 3 || column == 4){
        return this->whatsThis(column) < tmp.whatsThis(column);
      // We are in size text
      }else if(column == 1) {
        QString text = this->text(column);
        QString text_tmp = tmp.text(column);
        double filesize, filesize_tmp;
        // On folders, text is empty so we check for that
        // In case we are in folders, we put -1 for differentiate of regular files with 0 bytes.
        // Doing so, all folders we'll be together instead of mixing with files with 0 bytes.
        if(text.isEmpty())
          filesize = -1;
        else
          filesize = LUtils::DisplaySizeToBytes(text);
        if(text_tmp.isEmpty())
          filesize_tmp = -1;
        else
          filesize_tmp = LUtils::DisplaySizeToBytes(text_tmp);
        return filesize < filesize_tmp;

      //Name column - still sort by type too (folders first)
      }else if(column == 0 && (this->text(2).isEmpty() || tmp.text(2).isEmpty()) ){
        if(this->text(2) != tmp.text(2)){ return this->text(2).isEmpty(); }
      }
      // In other cases, we trust base class implementation
      return QTreeWidgetItem::operator<(tmp);
    }
};

//Item override for sorting purposes of list widget items
class CQListWidgetItem : public QListWidgetItem {
public:
    CQListWidgetItem(const QIcon &icon, const QString &text, QListWidget *parent = Q_NULLPTR) : QListWidgetItem(icon,text,parent) {}
    virtual ~CQListWidgetItem() {}
    inline virtual bool operator<(const QListWidgetItem &tmp) const {
	QString type = this->data(Qt::UserRole).toString();
	QString tmptype = tmp.data(Qt::UserRole).toString();
      //Sort by type first
	if(type!=tmptype){ return (QString::compare(type,tmptype)<0); }
      //Then sort by name using the normal rules
      return QListWidgetItem::operator<(tmp);
    }
};

#endif
