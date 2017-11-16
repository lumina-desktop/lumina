//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This is the base C++ object that is used to pass information to the QML "RootDesktop" object
//===========================================
#ifndef _LUMINA_DESKTOP_QML_BACKEND_ROOT_DESKTOP_OBJECT_H
#define _LUMINA_DESKTOP_QML_BACKEND_ROOT_DESKTOP_OBJECT_H
#include <QObject>
#include <QList>

#include "ScreenObject.h"

class RootDesktopObject : public QObject{
	Q_OBJECT
	//Define all the QML Properties here (interface between QML and the C++ methods below)
	Q_PROPERTY( QStringList screens READ screens NOTIFY screensChanged)

public:
	//main contructor/destructor
	RootDesktopObject(QObject *parent = 0);
	~RootDesktopObject();

	static void RegisterType();

	//primary interface to fetch the current instance of the class (so only one is running at any given time)
	static RootDesktopObject* instance();

	//QML Read Functions
	QStringList screens();
	Q_INVOKABLE ScreenObject* screen(QString id);

	//QML Access Functions
	Q_INVOKABLE void logout();
	Q_INVOKABLE void lockscreen();
	Q_INVOKABLE void mousePositionChanged();
private:
	QList<ScreenObject*> s_objects;

public slots:
	void updateScreens(); //rescan/update screen objects
	void ChangeWallpaper(QString screen, QString);

private slots:

signals:
	void screensChanged();
	void startLogout();
	void mouseMoved();
	void lockScreen();

};
#endif
