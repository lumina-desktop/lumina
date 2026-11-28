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
#include "NativeEmbedWidget.h"

class NativeWindow : public QFrame{
	Q_OBJECT
public:
	NativeWindow(NativeWindowObject *obj);
	~NativeWindow();

	QPoint relativeOrigin(); //origin of the embedded window relative to the frame

public slots:
	void initProperties();
	//Mouse Interactivity
	void startMoving();
	void startResizing();

private:
	//Window status
	enum ModState{Normal, Move, ResizeTop, ResizeTopRight, ResizeRight, ResizeBottomRight, ResizeBottom, ResizeBottomLeft, ResizeLeft, ResizeTopLeft};
	ModState activeState;
	ModState currentCursor;
	QPoint offset; //needed for movement calculations (offset from mouse click to movement point)
	//Functions for getting/setting state
	ModState getStateAtPoint(QPoint pt, bool setoffset = false); //generally used for mouse location detection
	void setMouseCursor(ModState, bool override = false);  //Update the mouse cursor based on state
	QTimer *moveTimer, *resizeTimer;

	//Core object
	NativeWindowObject *WIN;
	// Interface items
	void createFrame();
	void loadIcons();

	QToolButton *closeB, *minB, *maxB, *otherB;
	QHBoxLayout *toolbarL;
	QVBoxLayout *vlayout;
	QLabel *titleLabel;
	NativeEmbedWidget *container;

	// Info cache variables
	QRect oldgeom, pending_geom;

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
	void submitLocationChange();
	void submitSizeChange();

protected:
	void mousePressEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void leaveEvent(QEvent *ev);
	//void enterEvent(QEvent *ev);
	void moveEvent(QMoveEvent *ev);

signals:
	void windowMoved(NativeWindowObject*);

};

#endif
