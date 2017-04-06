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
	/*void DetachTab(int); //
	void DroppedIn(QString); //The "whatsThis()" field on some other DnDTabBar tab
	void DraggedOut(QString); //The "whatsThis()" field on *this* DnDTabBar tab*/

private:
	//QMenu *tabMenu;
	//int selTab;

private slots:
	/*void slotDetachTab(){
	  qDebug() << "Detach Tab:" << selTab;
	  if(selTab>=0){ emit DetachTab(selTab); }
	}*/
public:
	DnDTabBar(QWidget *parent) : QTabBar(parent){
	  //this->setAcceptDrops(true);
	  //selTab = -1;
	  //tabMenu = new QMenu(this);
	  //tabMenu->addAction(tr("Detach Tab"), this, SLOT(slotDetachTab()) );
	}
	~DnDTabBar(){

	}



protected:
	/*virtual void mousePressEvent(QMouseEvent *ev){
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
              drag->setMimeData(mimeData);
	      Qt::DropAction dropAction = drag->exec();
	    }else{
	      QTabBar::mousePressEvent(ev);
	    }
	  }else{
	    QTabBar::mousePressEvent(ev);
	  }
	}

	virtual void mouseReleaseEvent(QMouseEvent *ev){

	}

	virtual void mouseMoveEvent(QMouseEvent *ev){

	}*/
};

class DnDTabWidget : public QTabWidget{
	Q_OBJECT
private:
	DnDTabBar *TB;
public:
	DnDTabWidget(QWidget *parent) : QTabWidget(parent){
	  TB = new DnDTabBar(this);
	  this->setTabBar(TB);
	}
	~DnDTabWidget(){}

	DnDTabBar* dndTabBar(){ return TB; }
};

#endif
