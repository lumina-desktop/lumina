//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_NATIVE_WINDOW_WIDGET_H
#define _LUMINA_DESKTOP_NATIVE_WINDOW_WIDGET_H

#include <global-includes.h>
#include <NativeWindowObject.h>

class NativeWindow : public QFrame{
	Q_OBJECT
public:
	NativeWindow(NativeWindowObject *obj);
	~NativeWindow();

	QPoint relativeOrigin(); //origin of the embedded window relative to the frame

public slots:
	void initProperties();

private:
	//Core object
	NativeWindowObject *WIN;
	// Interface items
	void createFrame();
	void loadIcons();

	QToolButton *closeB, *minB, *maxB, *otherB;
	QHBoxLayout *toolbarL;
	QVBoxLayout *vlayout;
	QLabel *titleLabel;
	QWidget *container;
	// Info cache variables
	QRect oldgeom;

private slots:

	//Property Change slots
	void syncWinImage();
	void syncName();
	void syncTitle();
	void syncIcon();
	void syncSticky();
	void syncVisibility(bool init = false);
	void syncWinType();
	void syncGeom();
};

#endif
