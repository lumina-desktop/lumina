//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// Simple subclass of QPrintPreviewWidget to provide
// notification when a context menu is requested
//===========================================
#ifndef _CONTEXT_MENU_PRINT_PREVIEW_WIDGET_H
#define _CONTEXT_MENU_PRINT_PREVIEW_WIDGET_H

#include <QPrintPreviewWidget>
#include <QMouseEvent>
#include <QDebug>

class CM_PrintPreviewWidget : public QPrintPreviewWidget{
	Q_OBJECT
signals:
	void customContextMenuRequested(const QPoint&);

public:
	CM_PrintPreviewWidget(QPrinter * printer = 0, QWidget *parent = 0) : QPrintPreviewWidget(printer, parent){
	  this->setMouseTracking(true);
	  QList<QWidget*> children = this->findChildren<QWidget*>("",Qt::FindChildrenRecursively);
	  for(int i=0; i<children.length(); i++){
	    children[i]->setContextMenuPolicy(Qt::CustomContextMenu);
	    connect(children[i], SIGNAL(customContextMenuRequested(const QPoint&)), this, SIGNAL(customContextMenuRequested(const QPoint&)) );
	  }
	}

};

#endif
