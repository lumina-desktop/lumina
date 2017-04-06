//===========================================
//  Lumina-DE source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This is a drag and drop capable version of the QTabBar
//===========================================
#ifndef _LUMINA_TEXT_EDITOR_DND_TABWIDGET_H
#define _LUMINA_TEXT_EDITOR_DND_TABWIDGET_H

#include <QTabBar>
#include <QTabWidget>
#include <QMouseEvent>
#include <QDebug>
#include <QDrag>
#include <QMimeData>
#include <QMenu>
#include <QAction>

class DnDTabBar : public QTabBar{
	Q_OBJECT
signals:
	void DetachTab(int); //
	void DroppedIn(QStringList); //The full path of some file(s)
	void DraggedOut(int, Qt::DropAction); //The tab number dragged/accepted by another app, and which action was accepted

private:
	QMenu *tabMenu;
	int selTab;
	bool dragging;

private slots:
	void slotDetachTab(){
	  //qDebug() << "Detach Tab:" << selTab;
	  if(selTab>=0){ emit DetachTab(selTab); }
	}
public:
	DnDTabBar(QWidget *parent) : QTabBar(parent){
	  this->setAcceptDrops(true);
	  selTab = -1;
	  tabMenu = new QMenu(this);
	  tabMenu->addAction(tr("Detach Tab"), this, SLOT(slotDetachTab()) );
	  dragging = false;
	}
	~DnDTabBar(){

	}



protected:
	virtual void mousePressEvent(QMouseEvent *ev){
	  if(ev->button() == Qt::RightButton){
	    int tab = this->tabAt(ev->pos());
	    if(tab>=0){
	      selTab = tab;
	     tabMenu->popup(ev->globalPos());
	    }
	    QTabBar::mousePressEvent(ev);

	  }else if(ev->button() == Qt::MiddleButton){
	    int tab = this->tabAt(ev->pos());
	    if(tab>=0){
	      QDrag *drag = new QDrag(this);
              QMimeData *mimeData = new QMimeData;
              mimeData->setUrls(QList<QUrl>() << QUrl::fromLocalFile(this->tabWhatsThis(tab)));
	      //qDebug() << "Start Drag:" << this->tabWhatsThis(tab);
              drag->setMimeData(mimeData);
	      Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
	      this->emit DraggedOut(tab, dropAction);
	    }else{
	      QTabBar::mousePressEvent(ev);
	    }
	  }else{
	    QTabBar::mousePressEvent(ev);
	  }
	}

	virtual void dragEnterEvent(QDragEnterEvent *ev){
	  //qDebug() << "Got Drag Enter Event:" << ev->mimeData()->hasUrls();
	  if(ev->mimeData()->hasUrls() && ev->source()!=this){
	    ev->acceptProposedAction();
	  }
	}

	virtual void dragMoveEvent(QDragMoveEvent *ev){
	  //qDebug() << "Got Drag Move Event:" << ev->mimeData()->hasUrls();
	  if(ev->mimeData()->hasUrls() && ev->source()!=this){
	    ev->accept();
	  }
	}

	virtual void dropEvent(QDropEvent *ev){
	  //qDebug() << "Got Drop Event:" << ev->mimeData()->hasUrls();
	  if(ev->mimeData()->hasUrls() && ev->source()!=this){
	    QStringList files;
	    for(int i=0; i<ev->mimeData()->urls().length(); i++){
	      QString path = ev->mimeData()->urls().at(i).toLocalFile();
	      if(QFile::exists(path)){ files << path; }
	    }
	    //qDebug() << "[DROP] URLS:" << ev->mimeData()->urls() << "Files:" << files;
	    if(!files.isEmpty() && (ev->proposedAction()==Qt::CopyAction || ev->proposedAction()==Qt::MoveAction)){
	      ev->setDropAction(Qt::MoveAction);
	      ev->accept();
	      this->emit DroppedIn(files);
	    }
	  }
	}
};

class DnDTabWidget : public QTabWidget{
	Q_OBJECT
private:
	DnDTabBar *TB;
public:
	DnDTabWidget(QWidget *parent) : QTabWidget(parent){
	  TB = new DnDTabBar(this);
	  this->setTabBar(TB);
	  this->setTabsClosable(true);
	  this->setMovable(true);
	}
	~DnDTabWidget(){}

	DnDTabBar* dndTabBar(){ return TB; }
};

#endif
