//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _DESKTOP_WINDOW_EMBED_WIDGET_H
#define _DESKTOP_WINDOW_EMBED_WIDGET_H

#include <global-includes.h>

#define USE_QWINDOW_EMBED  1

class EmbedWidget : public QWidget {
	Q_OBJECT
private:
	NativeWindowObject *WIN;

public:
	EmbedWidget(NativeWindowObject *win, QWidget* parent) : QWidget(parent){
	  WIN = win;
	}
	~EmbedWidget(){}

public slots:
	void submitNewGeom(){
	  QRect global_geom = QRect( this->mapToGlobal(QPoint(0,0)), this->size() );
	  WIN->setGeometryNow(global_geom);
	}

protected:
	void resizeEvent(QResizeEvent *ev){
	  QWidget::resizeEvent(ev);
	  submitNewGeom();
	}
	/*void moveEvent(QMoveEvent *ev){
	  QWidget::moveEvent(ev);
	  submitNewGeom();
	}*/
	void hideEvent(QHideEvent *ev){
	  QWidget::hideEvent(ev);
	  qDebug() << "Hide Event";
	  WIN->requestProperty(NativeWindowObject::Visible, false);
	}
	void showEvent(QShowEvent *ev){
	  QWidget::showEvent(ev);
	  qDebug() << "ShowEvent";
	  WIN->requestProperty(NativeWindowObject::Visible, true);
	}
	void enterEvent(QEvent *ev){
	  QWidget::enterEvent(ev);
	  qDebug() << "Enter event";
	}
	void paintEvent(QPaintEvent *){
	  //Never paint anything with this widget
	}
	void closeEvent(QCloseEvent *ev){
	  qDebug() << "Embed Window Closed";
	  WIN->emit WindowClosed(WIN->id());
	  QWidget::closeEvent(ev);
	}

};

class NativeEmbedWidget : public QObject {
	Q_OBJECT
private:
	QWidget *embedW;
	NativeWindowObject *WIN;

public:
	NativeEmbedWidget(QWidget *parent, NativeWindowObject *obj) : QObject(parent){
	  WIN = obj;
	  if(USE_QWINDOW_EMBED){
	    QWindow* _window = QWindow::fromWinId(WIN->id());
	    embedW = QWidget::createWindowContainer(_window, parent);
	    QList<WId> related; related <<  _window->winId();
	    if(embedW->nativeParentWidget()!=0){ related << embedW->nativeParentWidget()->winId(); }
	    WIN->setProperty(NativeWindowObject::RelatedWindows, QVariant::fromValue< QList<WId> >(related) );
	    //connect(_window, SIGNAL(destroyed(QObject*)), WIN, SLOT(announceClosed()) );
	  }else{
	    embedW = new EmbedWidget(WIN, parent);
	  }
	}
	~NativeEmbedWidget(){}

	QWidget* widget(){ return embedW; }
	QRect geometry(){ return embedW->geometry(); }

public slots:
	void activateWindow(){ WIN->requestActivate(); } //QTimer::singleShot(0, WIN, SLOT(requestActivate())); }
	void windowFrameMoved(){}

};

#endif
