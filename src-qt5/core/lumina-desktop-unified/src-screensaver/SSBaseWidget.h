//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This class is the widget which provides the screensaver painting/plugin functionality
//===========================================
#ifndef _LUMINA_DESKTOP_SCREEN_SAVER_BASE_WIDGET_H
#define _LUMINA_DESKTOP_SCREEN_SAVER_BASE_WIDGET_H

#include "global-includes.h"
#include "animations/BaseAnimGroup.h"

class SSBaseWidget : public QWidget{
	Q_OBJECT
public:	
	SSBaseWidget(QWidget *parent, QSettings *set);
	~SSBaseWidget();
	
	void setPlugin(QString);

public slots:
	void startPainting();
	void stopPainting();

private:
	QString plugType, cplug; //type of custom painting to do
	BaseAnimGroup *ANIM;
	QSettings *settings;

private slots:
	
signals:
	void InputDetected(); //just in case no event handling setup at the WM level

protected:
	void mouseMoveEvent(QMouseEvent *ev){
	  ev->accept();
	  emit InputDetected();
	}
	void keyPressEvent(QKeyEvent *ev){
	  ev->accept();
	  emit InputDetected();
	}
	void paintEvent(QPaintEvent*){
	  QStyleOption opt;
	  opt.init(this);
	  QPainter p(this);
	  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
	}
	
};

#endif
