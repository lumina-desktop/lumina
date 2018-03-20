//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_NATIVE_WINDOW_WIDGET_H
#define _LUMINA_DESKTOP_NATIVE_WINDOW_WIDGET_H

#include <global-includes.h>

class NativeWindow : public QFrame{
	Q_OBJECT
public:
	NativeWindow(NativeWindowObject *obj);
	~NativeWindow();

private:
	//Core object
	NativeWindowObject *WIN;
	// Interface items
	void createFrame();
	QToolButton *closeB, *minB, *maxB, *otherB;
	QHBoxLayout *toolbarL;
	QVBoxLayout *vlayout;
	QLabel *titleLabel;
	// Info cache variables
	QRect oldgeom;

private slots:

	//Property Change slots
	void syncWinImage();
	void syncName();
	void syncTitle();
	void syncIcon();
	void syncSticky();
	void syncVisibility();
	void syncWinType();
	void syncGeom();
};

#endif
