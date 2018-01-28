//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012-2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_LIBRARY_RESIZE_MENU_H
#define _LUMINA_LIBRARY_RESIZE_MENU_H

#include <QMenu>
#include <QWidget>
#include <QMouseEvent>
#include <QAction>
#include <QWidgetAction>
#include <QRect>
#include <QCursor>
#include <QPoint>

//Special subclass for a menu which the user can grab the edges and resize as necessary
// Note: Make sure that you don't set 0pixel contents margins on this menu
//    - it needs at least 1 pixel margins for the user to be able to grab it
class ResizeMenu : public QMenu{
	Q_OBJECT
public:
	ResizeMenu(QWidget *parent = 0);
	virtual ~ResizeMenu();

	void setContents(QWidget *con);
	void resyncSize();

private:
	enum SideFlag{NONE, TOP, BOTTOM, LEFT, RIGHT};
	SideFlag resizeSide;
	QWidget *contents;
	QWidgetAction *cAct;

private slots:
	void clearFlags(){
	  resizeSide=NONE;
	}

protected:
	virtual void mouseMoveEvent(QMouseEvent *ev);
	virtual void mousePressEvent(QMouseEvent *ev);
	virtual void mouseReleaseEvent(QMouseEvent *ev);

signals:
	void MenuResized(QSize); //Emitted when the menu is manually resized by the user

};

#endif
