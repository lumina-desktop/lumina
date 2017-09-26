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

public:
	GLW_Widget(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
	~GLW_Widget();

	QRect widgetRect(); //converts to the coordinate scheme of the base widget
	bool mouseOverWidget();

	void setGLBase(GLW_Base *base);

	virtual void paintYourself(QStylePainter *painter, QPaintEvent *ev);
	void paintChildren(QStylePainter *painter, QPaintEvent *ev);

private slots:

public slots:

protected:
	void enterEvent(QEvent*){ this->update(); }
	void leaveEvent(QEvent*){ this->update(); }
	void resizeEvent(QResizeEvent *ev);
	void paintEvent(QPaintEvent *ev);

signals:
	void repaintArea(QRect);
};

Q_DECLARE_INTERFACE(GLW_Widget, "GLW_Widget");
#endif
