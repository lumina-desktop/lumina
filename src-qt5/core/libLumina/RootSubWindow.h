//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This class embeds a native window
//  within the RootWindow area
//===========================================
#ifndef _LUMINA_ROOT_WINDOW_SUB_WINDOW_H
#define _LUMINA_ROOT_WINDOW_SUB_WINDOW_H

#include <QWindow>
#include <QWidget>
#include <QCloseEvent>
#include <QFrame>
#include <QBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QMenu>
#include <NativeWindow.h>


class RootSubWindow : public QFrame{
	Q_OBJECT
public:
	RootSubWindow(QWidget *root, NativeWindow *win);
	~RootSubWindow();

	WId id();

private:
	//Window status 
	enum ModState{Normal, Move, ResizeTop, ResizeTopRight, ResizeRight, ResizeBottomRight, ResizeBottom, ResizeBottomLeft, ResizeLeft, ResizeTopLeft};
	ModState activeState;
	ModState currentCursor;
	QPoint offset; //needed for movement calculations (offset from mouse click to movement point)
	//Functions for getting/setting state
	ModState getStateAtPoint(QPoint pt, bool setoffset = false); //generally used for mouse location detection
	void setMouseCursor(ModState, bool override = false);  //Update the mouse cursor based on state
	//Native window embed objects
	NativeWindow *WIN;
	QWidget *WinWidget;
	bool closing;
	//Title bar objects
	QBoxLayout *titleBar, *mainLayout;
	QToolButton *closeB, *maxB, *minB, *otherB;
	QLabel *titleLabel;
	QMenu *otherM; //menu of other actions
	void initWindowFrame();

	void LoadProperties( QList< NativeWindow::Property> list);

public slots:
	void clientClosed();

	//Button Actions - public so they can be tied to key shortcuts and stuff as well
	void toggleMinimize();
	void toggleMaximize();
	void triggerClose();
	void toggleSticky();
	void activate();

	//Mouse Interactivity
	void startMoving();
	void startResizing();
	
private slots:
	void propertiesChanged(QList<NativeWindow::Property>, QList<QVariant>);

protected:
	void mousePressEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void leaveEvent(QEvent *ev);

};

#endif
