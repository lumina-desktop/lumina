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
#include <QPropertyAnimation>
#include <NativeWindow.h>
#include <NativeEmbedWidget.h>

class RootSubWindow : public QFrame{
	Q_OBJECT
public:
	RootSubWindow(QWidget *root, NativeWindow *win);
	~RootSubWindow();

	WId id();
	NativeWindow* nativeWindow();

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
	NativeEmbedWidget *WinWidget;
	bool closing;
	//Title bar objects
	QBoxLayout *titleBarL, *mainLayout;
	QToolButton *closeB, *maxB, *minB, *otherB;
	QLabel *titleLabel;
	QMenu *otherM; //menu of other actions
	QWidget *titleBar;
	//Other random objects (animations,etc)
	QPropertyAnimation *anim;
	QVariant animResetProp;
	QTimer *moveTimer;
	QRect lastGeom, lastMaxGeom; //frame coordinates

	void initWindowFrame();
	void enableFrame(bool);
	void enableFrame(QList<NativeWindow::Type> types);

	void LoadProperties( QList< NativeWindow::Property> list);

	static QStringList validAnimations(NativeWindow::Property);

public slots:
	void ensureVisible(){ WIN->setProperty(NativeWindow::Visible, true); }
	void giveMouseFocus(){ WinWidget->raiseWindow(); }
	void removeMouseFocus(){ WinWidget->lowerWindow(); }
	void giveKeyboardFocus(){ WIN->requestProperty(NativeWindow::Active, true, true); }

	void clientClosed();
	void LoadAllProperties();

	QRect clientGlobalGeom();

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

	void loadAnimation(QString name, NativeWindow::Property, QVariant nval); //new val
	void animFinished();

protected:
	void mousePressEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	//void leaveEvent(QEvent *ev);
	//void enterEvent(QEvent *ev);
	void moveEvent(QMoveEvent *ev);

signals:
	void windowMoved(RootSubWindow*);
	void windowAnimFinished();
};

#endif
