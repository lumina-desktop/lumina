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
	QString selTab;

private slots:
	void slotDetachTab(){
	  //qDebug() << "Detach Tab:" << selTab;
	  int tab = tabMenu->whatsThis().toInt();
	  if(tab>=0){ emit DetachTab(tab); }
	}
public:
	DnDTabBar(QWidget *parent) : QTabBar(parent){
	  this->setAcceptDrops(true);
	  this->setMouseTracking(true);
	  tabMenu = new QMenu(this);
	  tabMenu->addAction(tr("Detach Tab"), this, SLOT(slotDetachTab()) );
	}
	~DnDTabBar(){

	}



protected:
	virtual void mousePressEvent(QMouseEvent *ev){
	  int tab = this->tabAt(ev->pos());
	  if(ev->button() == Qt::LeftButton && tab>=0){
	    selTab = this->tabWhatsThis(tab);
	  }
	  QTabBar::mousePressEvent(ev);
	}

	virtual void mouseMoveEvent(QMouseEvent *ev){
          //qDebug() << "Got Move Event:" << this->geometry() << ev->pos();
	  QWidget *parent = this->parentWidget(); //top-level parent widget
          while(parent->parentWidget()!=0 && !parent->isWindow()){ parent = parent->parentWidget(); }
	  if(selTab>=0 && !QRect(QPoint(0,0), parent->size()).contains( this->mapTo(parent, ev->pos())) ){
            //qDebug() << "Got Mouse outside of parent:" << parent->geometry() << this->geometry() << this->mapTo(parent, ev->pos());
            //if(ev->button()!=Qt::LeftButton){ QTabBar::mouseMoveEvent(ev); return; }
	    qDebug() << "Starting Drag:" << this->geometry() << ev->pos();
	    QString tab = selTab;
            this->mouseReleaseEvent(new QMouseEvent(QEvent::MouseButtonRelease, ev->pos(), ev->button(), ev->buttons(), ev->modifiers()) ); //will reset selTab
	    //this->update();
	     QDrag *drag = new QDrag(this);
              QMimeData *mimeData = new QMimeData;
              mimeData->setUrls(QList<QUrl>() << QUrl::fromLocalFile(tab));
	      //qDebug() << "Start Drag:" << this->tabWhatsThis(tab);
              drag->setMimeData(mimeData);
	      Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
	      //Convert the tab->number and emit
              for(int i=0; i<this->count(); i++){
	        if(this->tabWhatsThis(i) == tab){ this->emit DraggedOut(i, dropAction); this->setCurrentIndex(i); break; }
 	      }
	    return;
	  }
	  QTabBar::mouseMoveEvent(ev);
	}

	virtual void mouseReleaseEvent(QMouseEvent *ev){
 	  int tab = this->tabAt(ev->pos());
	  if(ev->button() == Qt::RightButton && tab>=0){
	    tabMenu->setWhatsThis(QString::number(tab));
	    tabMenu->popup(ev->globalPos());
	  }
	  selTab.clear(); //reset this flag - not in a drag right now
	  QTabBar::mouseReleaseEvent(ev);
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
