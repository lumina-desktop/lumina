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
	Q_PROPERTY( QStringList windows READ windows NOTIFY windowsChanged)
	Q_PROPERTY( QStringList trayWindows READ trayWindows NOTIFY trayWindowsChanged)
	Q_PROPERTY( bool hasTrayWindows READ hasTrayWindows NOTIFY trayWindowsChanged)
	Q_PROPERTY( QString currentTime READ currentTime NOTIFY currentTimeChanged);
	Q_PROPERTY( QDateTime currentDateTime READ currentDateTime NOTIFY currentTimeChanged);

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
	Q_INVOKABLE QStringList trayWindows();
	Q_INVOKABLE NativeWindowObject* trayWindow(QString id);
	Q_INVOKABLE bool hasTrayWindows();
	Q_INVOKABLE QString currentTime();
	Q_INVOKABLE QDateTime currentDateTime();


	//QML Globals Access
	Q_INVOKABLE OSInterface* os_interface();

	//QML Access Functions
	Q_INVOKABLE void logout();
	Q_INVOKABLE void lockscreen();
	Q_INVOKABLE void mousePositionChanged(bool lowerall = false);
	Q_INVOKABLE void launchApp(QString appOrPath);

	//C++ Access Functions (simplifications for the QML ones)
	QList<NativeWindowObject*> windowObjects();
	QList<NativeWindowObject*> trayWindowObjects();
	QList<ScreenObject*> screenObjects();
	QRegion* availableGeometry();

private:
	QList<ScreenObject*> s_objects;
	QList<PanelObject*> panel_objects;
	QList<NativeWindowObject*> window_objects;
	QList<NativeWindowObject*> tray_window_objects;
	QPointer<NativeWindowObject> last_window_up;
	QTimer *currentTimeTimer, *availgeomTimer;
	QString currentTimeFormat, currentTimeString;
	QDateTime currentDateTimeStruct;
	QRegion session_avail_geom;

public slots:
	void updateScreens(); //rescan/update screen objects
	void ChangeWallpaper(QString screen, QString);
	QString CurrentWallpaper(QString screen);

	void setPanels(QList<PanelObject*> list);
	void setPanels(QStringList ids);
	QList<PanelObject*> panelObjectList(){ return panel_objects; }

	void setWindows(QList<NativeWindowObject*> list);
	void setTrayWindows(QList<NativeWindowObject*> list);

	void updateCurrentTimeFormat(QString);

private slots:
	void updateCurrentTime();
	void availableScreenGeomChanged();
	void submitSessionGeom();

signals:
	void screensChanged();
	void panelsChanged();
	void windowsChanged();
	void trayWindowsChanged();
	void currentTimeChanged();
	void sessionGeomAvailableChanged();

	void startLogout();
	void mouseMoved();
	void lockScreen();
	void launchApplication(QString);

	//Internal signals for thread-safety
	void changePanels(QStringList);
};
#endif
