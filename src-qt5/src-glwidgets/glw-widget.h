//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_OPENGL_WIDGETS_WIDGET_H
#define _LUMINA_OPENGL_WIDGETS_WIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QStylePainter>
#include <QResizeEvent>
#include <QStylePainter>

#include "glw-base.h"

class GLW_Widget : public QWidget{
	Q_OBJECT
private:
	GLW_Base *glw_base;
	QPoint drag_offset;
	bool draggable;

public:
	GLW_Widget(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
	~GLW_Widget();

	QRect widgetRect(); //converts to the coordinate scheme of the base widget
	bool mouseOverWidget();

	void setGLBase(GLW_Base *base);

	virtual void paintYourself(QStylePainter *painter, QPaintEvent *ev);
	void paintChildren(QStylePainter *painter, QPaintEvent *ev);

	//Properties
	bool isDraggable(){ return draggable; }

private slots:

public slots:
	void setDraggable(bool drag){ draggable = drag; }


protected:
	virtual void enterEvent(QEvent*);
	virtual void leaveEvent(QEvent*);

	virtual void mousePressEvent(QMouseEvent *ev);
	virtual void mouseReleaseEvent(QMouseEvent *ev);
	virtual void mouseMoveEvent(QMouseEvent *ev);
	virtual void moveEvent(QMoveEvent *ev);
	virtual void resizeEvent(QResizeEvent *ev);
	virtual void paintEvent(QPaintEvent *ev);

signals:
	void repaintArea(QRect);
	void doneDragging();
};

Q_DECLARE_INTERFACE(GLW_Widget, "GLW_Widget");
#endif
