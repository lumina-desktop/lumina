//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
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


#include <LuminaXDG.h>

class UserItemWidget : public QFrame{
	Q_OBJECT
public:
	UserItemWidget(QWidget *parent=0, QString itemPath="", bool isDir=false);
	UserItemWidget(QWidget *parent=0, XDGDesktop item= XDGDesktop());
	~UserItemWidget();

private:
	QToolButton *button;
	QLabel *icon, *name;
	bool isDirectory;
	
	void createWidget();
	void setupButton();

private slots:
	void buttonClicked();
	void ItemClicked();

protected:
	void mouseReleaseEvent(QMouseEvent *event){
	  if(event->button() != Qt::NoButton){ ItemClicked(); }
	}
	
signals:
	void NewShortcut();
	void RemovedShortcut();
	void RunItem(QString cmd);

};

#endif