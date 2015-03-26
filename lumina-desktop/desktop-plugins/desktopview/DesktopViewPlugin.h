//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This plugin is a listing/launcher for things in the ~/Desktop folder
//===========================================
#ifndef _LUMINA_DESKTOP_DESKTOP_VIEW_PLUGIN_H
#define _LUMINA_DESKTOP_DESKTOP_VIEW_PLUGIN_H

#include <QListWidget>
#include <QVBoxLayout>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QMouseEvent>

#include "../LDPlugin.h"

class DesktopViewPlugin : public LDPlugin{
	Q_OBJECT
public:
	DesktopViewPlugin(QWidget* parent, QString ID);
	~DesktopViewPlugin();
	
private:
	QListWidget *list;
	QFileSystemWatcher *watcher;
	QMenu *menu;

private slots:
	void runItems();
	void copyItems();
	void cutItems();
	void deleteItems();
	void showMenu(const QPoint&);
	void increaseIconSize();
	void decreaseIconSize();
	void updateContents();
	void displayProperties();


public slots:
	void LocaleChange(){
	  QTimer::singleShot(0,this, SLOT(updateContents()));
	}
	void ThemeChange(){
	  QTimer::singleShot(0,this, SLOT(updateContents()));
	}
	
/*protected:
	void mousePressEvent(QMouseEvent *ev){
	  if(ev->button()==Qt::RightButton){
	    qDebug() << " - got mouse event";
	    //Only show the context menu if an item is under the mouse (don't block the desktop menu)
	    if(list->itemAt( ev->globalPos()) !=0){
	      ev->accept();
	      showMenu(ev->globalPos());
	    }
	  }
	}
	*/

};
#endif
