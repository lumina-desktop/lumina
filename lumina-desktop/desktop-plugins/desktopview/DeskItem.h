//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_DESKTOP_ITEM_H
#define _LUMINA_DESKTOP_DESKTOP_ITEM_H

#include <QToolButton>
#include <QProcess>
#include <QString>

#include <LuminaXDG.h>

class DeskItem : public QToolButton{
	Q_OBJECT
public:
	DeskItem(QWidget *parent, QString itempath, int ssize);
	~DeskItem();

	void updateItem();

private slots:
	void RunItem(){
	  QProcess::startDetached("lumina-open "+this->whatsThis());
	}
};

#endif