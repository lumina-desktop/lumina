//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_OPENGL_WIDGETS_BASE_H
#define _LUMINA_OPENGL_WIDGETS_BASE_H

#include <QOpenGLWidget>
#include <QImage>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QStylePainter>

class GLW_Base : public QOpenGLWidget{
	Q_OBJECT
private:
	QColor bg_color;
	QImage bg_img;
	QWidget *mouse_over_child;

public:
	GLW_Base(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
	~GLW_Base();

	QWidget * mouseOverWidget();

private slots:

public slots:
	void setBackgroundColor(QColor color);
	void setBackground(QRect geom, QImage img);
	void repaintArea(QRect);
	void setMouseOverWidget(QWidget*);

signals:
	void BaseResized();

protected:
	void mouseMoveEvent(QMouseEvent *ev);
	void resizeEvent(QResizeEvent *ev);
	void paintEvent(QPaintEvent *ev);
};

#endif
