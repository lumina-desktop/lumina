//===========================================
//  Lumina-DE source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is a container object for embedding a native window into a QWidget
//    and maintaining a 1-to-1 mapping of sizing and other properties
//    while also providing compositing effects between the two windows
//===========================================
#ifndef _LUMINA_NATIVE_EMBED_WIDGET_H
#define _LUMINA_NATIVE_EMBED_WIDGET_H

#include "NativeWindow.h"
#include <QWidget>
#include <QTimer>
#include <QResizeEvent>
#include <QShowEvent>
#include <QHideEvent>
#include <QPaintEvent>

class NativeEmbedWidget : public QWidget{
	Q_OBJECT
private:
	NativeWindow *WIN;
	QSize winSize;

private slots:
	//Simplification functions
	void syncWinSize(QSize sz = QSize());
	void syncWidgetSize(QSize sz);
	void hideWindow();
	void showWindow();
	QImage windowImage(QRect geom);


public:
	NativeEmbedWidget(QWidget *parent);

	bool embedWindow(NativeWindow *window);
	bool detachWindow();
	bool isEmbedded(); //status of the embed

public slots:
	void resyncWindow();
	void repaintWindow();

protected:
	void resizeEvent(QResizeEvent *ev);
	void showEvent(QShowEvent *ev);
	void hideEvent(QHideEvent *ev);
	void paintEvent(QPaintEvent *ev);
};

#endif
