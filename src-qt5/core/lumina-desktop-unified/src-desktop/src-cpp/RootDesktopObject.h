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
#include <global-includes.h>
#include <ScreenObject.h>
#include <QThread>

class RootDesktopObject : public QObject{
	Q_OBJECT
	//Define all the QML Properties here (interface between QML and the C++ methods below)
	Q_PROPERTY( QStringList screens READ screens NOTIFY screensChanged)
	Q_PROPERTY( QStringList panels READ panels NOTIFY panelsChanged)
	Q_PROPERTY( QStringList windows READ windows NOTIFY windowsChanged);

public:
	//main contructor/destructor
	RootDesktopObject(QObject *parent = 0);
	~RootDesktopObject();

	static void RegisterType();

	//primary interface to fetch the current instance of the class (so only one is running at any given time)
	static RootDesktopObject* instance();

	//QML Read Functions
	Q_INVOKABLE QStringList screens();
	Q_INVOKABLE ScreenObject* screen(QString id);
	Q_INVOKABLE QStringList panels();
	Q_INVOKABLE PanelObject* panel(QString id);
	Q_INVOKABLE QStringList windows();
	Q_INVOKABLE NativeWindowObject* window(QString id);

	//QML Access Functions
	Q_INVOKABLE void logout();
	Q_INVOKABLE void lockscreen();
	Q_INVOKABLE void mousePositionChanged();
	Q_INVOKABLE void launchApp(QString appOrPath);

private:
	QList<ScreenObject*> s_objects;
	QList<PanelObject*> panel_objects;
	QList<NativeWindowObject*> window_objects;
	QPointer<NativeWindowObject> last_window_up;

public slots:
	void updateScreens(); //rescan/update screen objects
	void ChangeWallpaper(QString screen, QString);
	QString CurrentWallpaper(QString screen);

	void setPanels(QList<PanelObject*> list);
	void setPanels(QStringList ids);
	QList<PanelObject*> panelObjectList(){ return panel_objects; }

	void setWindows(QList<NativeWindowObject*> list);

private slots:

signals:
	void screensChanged();
	void panelsChanged();
	void windowsChanged();

	void startLogout();
	void mouseMoved();
	void lockScreen();
	void launchApplication(QString);

	//Internal signals for thread-safety
	void changePanels(QStringList);
};
#endif
