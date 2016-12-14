//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This item widget manages a single file/directory
//===========================================
#ifndef _LUMINA_PANEL_USER_ITEM_WIDGET_H
#define _LUMINA_PANEL_USER_ITEM_WIDGET_H

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

#include <LuminaXDG.h>

class UserItemWidget : public QFrame{
	Q_OBJECT
public:
	UserItemWidget(QWidget *parent=0, QString itemPath="", QString type="unknown", bool goback=false);
	UserItemWidget(QWidget *parent=0, XDGDesktop *item= 0);
	~UserItemWidget();

	bool gooditem;
private:
	QToolButton *button, *actButton;
	QLabel *icon, *name;
	bool isDirectory, isShortcut, menuopen;
	QString linkPath;
	QTimer *menureset;
	
	void createWidget();
	void setupButton(bool disable = false);
	void setupActions(XDGDesktop*);

private slots:
	void buttonClicked();
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
	  else if(event->button() != Qt::NoButton){ ItemClicked(); }
	}
	
signals:
	void NewShortcut();
	void RemovedShortcut();
	void RunItem(QString cmd);

};

#endif
