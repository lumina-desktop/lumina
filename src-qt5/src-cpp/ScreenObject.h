//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This is the base C++ object that is used to pass information to the QML "RootDesktop" object
//===========================================
#ifndef _LUMINA_DESKTOP_ROOT_DESKTOP_OBJECT_H
#define _LUMINA_DESKTOP_ROOT_DESKTOP_OBJECT_H

class ScreenObject : public QObject{
	Q_OBJECT
	Q_PROPERTY( QString name READ name )
	Q_PROPERTY( QString background READ background NOTIFY backgroundChanged)
	Q_PROPERTY( int x READ x NOTIFY geomChanged)
	Q_PROPERTY( int y READ y NOTIFY geomChanged)
	Q_PROPERTY( int width READ width NOTIFY geomChanged)
	Q_PROPERTY( int height READ height NOTIFY geomChanged)

private:
	QScreen *bg_screen;
	QString bg;

public
	ScreenObject(QScreen *scrn, QObject *parent = 0) : QObject(parent){
	  bg_screen = scrn;
	}

	static void RegisterType(){ qmlRegisterType<RootDesktopObject>("Lumina.Backend.ScreenObject",2,0, "ScreenObject");

	QString name(){ return bg_screen->name(); }
	QString background(){ return bg; }
	int x(){ return bg_screen->geometry().x(); }
	int y(){ return bg_screen->geometry().y(); }
	int width(){ return bg_screen->geometry().width(); }
	int height(){ return bg_screen->geometry().height(); }

public slots:
	void setBackground(QString fileOrColor){
	  if(bg!=fileOrColor){
	    bg = fileOrColor;
	    emit backgroundChanged();
	  }
	}

signals:
	void backgroundChanged();
	void geomChanged();
};
