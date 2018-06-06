//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _DESKTOP_WINDOW_EMBED_WIDGET_H
#define _DESKTOP_WINDOW_EMBED_WIDGET_H
#include <global-includes.h>

class NativeEmbedWidget : public QObject{
	Q_OBJECT
private:
	QWidget *embedW;
	QWindow *_window;
	NativeWindowObject *WIN;

private slots:
	void visibleChanged(bool show){ WIN->setProperty(NativeWindowObject::Visible, show); }
	void windowTitleChanged(QString title){ WIN->setProperty(NativeWindowObject::Title, title); }
	void heightChanged(int val){ qDebug() << "Got Wndow Height change:" << val; } //WIN->setProperty(NativeWindowObject::Size, QSize(WIN->property(NativeWindowObject::Size).toSize().width(),val) ); }
	void widthChanged(int val){ qDebug() << "Got Wndow Width change:" << val; } // WIN->setProperty(NativeWindowObject::Size, QSize(val, WIN->property(NativeWindowObject::Size).toSize().height()) ); }
	void xChanged(int val){ qDebug() << "Got Window X changed:" << val; }
	void yChanged(int val){ qDebug() << "Got Window Y changed:" << val; }


public:
	NativeEmbedWidget(QWidget *parent, NativeWindowObject *obj) : QObject(parent){
	  WIN = obj;
	  _window = QWindow::fromWinId(WIN->id());
	  //embedW = new QWidget(parent);
	  embedW = QWidget::createWindowContainer(_window, parent);
	  //Setup all the internal connections
	  connect(_window, SIGNAL(visibleChanged(bool)), this, SLOT(visibleChanged(bool)) );
	  connect(_window, SIGNAL(windowTitleChanged(const QString&)), this, SLOT(windowTitleChanged(const QString&)) );
	  connect(_window, SIGNAL(widthChanged(int)), this, SLOT(widthChanged(int)) );
	  connect(_window, SIGNAL(heightChanged(int)), this, SLOT(heightChanged(int)) );
	  connect(_window, SIGNAL(xChanged(int)), this, SLOT(xChanged(int)) );
	  connect(_window, SIGNAL(yChanged(int)), this, SLOT(yChanged(int)) );
	}
	~NativeEmbedWidget(){}

	QWidget* widget(){ return embedW; }
	QRect geometry(){ return embedW->geometry(); }

public slots:
	void activateWindow(){ QTimer::singleShot(0, _window, SLOT(requestActivate())); }

};

#endif
