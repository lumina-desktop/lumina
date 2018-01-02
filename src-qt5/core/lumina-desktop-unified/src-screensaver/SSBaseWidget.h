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
#include <plugins-base.h>
#include <plugins-screensaver.h>

class SSBaseWidget : public QQuickView{
	Q_OBJECT
public:
	SSBaseWidget(QWidget *parent);
	~SSBaseWidget();

	void setPlugin(QString);

public slots:
	void startPainting();
	void stopPainting();

private:
  QString plugType;
  SSPlugin cplug;
	QTimer *restartTimer;

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

};

#endif
