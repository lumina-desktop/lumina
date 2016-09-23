//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This item widget manages a single file/directory
//===========================================
#ifndef _LUMINA_PANEL_SYSTEM_START_ITEM_WIDGET_H
#define _LUMINA_PANEL_SYSTEM_START_ITEM_WIDGET_H

#include <QFrame>
#include <QLabel>
#include <QToolButton>
#include <QString>
#include <QHBoxLayout>
#include <QSize>
#include <QDir>
#include <QFile>
#include <QMouseEvent>
#include <QAction>
#include <QMenu>
#include <QTimer>
#include <QResizeEvent>

#include <LuminaXDG.h>

class ItemWidget : public QFrame{
	Q_OBJECT
public:
	//Overloaded Constructors for various uses
	// - Favorites (path/type)
	ItemWidget(QWidget *parent=0, QString itemPath="", QString type="unknown", bool goback=false);
	// - Generic Apps
	ItemWidget(QWidget *parent=0, XDGDesktop *item= 0);

	~ItemWidget();

	bool gooditem;

	void triggerItem(); //trigger this item - just as if it was clicked on

private:
	QToolButton *actButton;
	QMenu *contextMenu;
	QLabel *icon, *name;
	bool isDirectory, isShortcut, menuopen;
	QString linkPath, iconPath, text;
	QTimer *menureset;
	
	void createWidget();

	void setupContextMenu();
	void setupActions(XDGDesktop*);

	void updateItems(); //update the text/icon to match sizes

private slots:
	void PinToDesktop();
	void RemoveFavorite();
	void AddFavorite();
	void RemoveQL();
	void AddQL();
	void ItemClicked();
	void actionClicked(QAction*);
	//Functions to fix the submenu open/close issues
	void actionMenuOpen(){ 
	  if(menureset->isActive()){ menureset->stop(); } 
	  menuopen = true; 
	}
	void resetmenuflag(){ menuopen = false; } //tied to the "menureset" timer
	void actionMenuClosed(){ menureset->start(); }
	

protected:
	void mouseReleaseEvent(QMouseEvent *event){
	  if(menuopen){ resetmenuflag(); } //skip this event if a submenu was open
	  else if(event->button() == Qt::RightButton && !icon->whatsThis().startsWith("chcat::::") ){ 
	    menuopen = true; 
	    setupContextMenu();
	    contextMenu->popup(event->globalPos());
          }else if(event->button() != Qt::NoButton){ ItemClicked(); }
	}
	
	void resizeEvent(QResizeEvent *ev){
	  updateItems(); //update the sizing of everything
	  QFrame::resizeEvent(ev); // do the normal procedures
	}
	
signals:
	void NewShortcut();
	void RemovedShortcut();
	void RunItem(QString cmd);
	void toggleQuickLaunch(QString path, bool ok);

};

#endif
