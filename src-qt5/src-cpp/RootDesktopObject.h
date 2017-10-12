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
	Q_PROPERTY( QScreen * screen READ screen)

private:
	QScreen* bg_screen;
	QString bg;

public
	ScreenObject(QScreen *scrn, QObject *parent = 0) : QObject(parent){
	  bg_screen = scrn;
	}

	QString name(){ return bg_screen->name(); }
	QString background(){ return bg; }
	QScreen* screen(){ return screen; }

public slots:
	void setBackground(QString fileOrColor){
	  if(bg!=fileOrColor){
	    bg = fileOrColor;
	    emit backgroundChanged();
	  }
	}

signals:
	void backgroundChanged();
};


class RootDesktopObject : public QObject{
	Q_OBJECT
	//Define all the QML Properties here (interface between QML and the C++ methods below)
	Q_PROPERTY( QList<ScreenObject*> screens READ screens NOTIFY screensChanged)

public:
	//main contructor/destructor
	RootDesktopObject(QObject *parent = 0);
	~RootDesktopObject();
	//primary interface to fetch the current instance of the class (so only one is running at any given time)
	static RootDesktopObject* instance();

	//QML Read Functions
	QList<ScreenObject*> screens();

private:
	QList<ScreenObject*> s_objects;

public slots:
	void updateScreens(); //rescan/update screen objects

private slots:

signals:
	void screensChanged();

};
#endif
