//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This class creates and manages a virtual "root" window
//   for all monitors at all times.
//===========================================
#ifndef _LUMINA_ROOT_WINDOW_H
#define _LUMINA_ROOT_WINDOW_H

#include <QWidget>
#include <QRect>
#include <QString>
#include <QPixmap>
#include <QPainter>
#include <QTimer>
#include <QApplication>
#include <QPaintEvent>
#include <QScreen>
#include <QDebug>

#include "RootSubWindow.h"

#include <NativeWindow.h>
#include <DesktopSettings.h>

class RootWindow : public QWidget{
	Q_OBJECT
public:
	enum ScaleType{ SolidColor, Stretch, Full, Fit, Center, Tile, BottomLeft, BottomRight, BottomCenter, \
	  TopLeft, TopRight, TopCenter, CenterLeft, CenterRight};

	RootWindow();
	~RootWindow();

	void start();

private:
	struct screeninfo{
	  QString id;
	  QRect area;
	  QString file;
	  ScaleType scale;
	  QPixmap wallpaper; //Note: This pixmap will always be the same size as "area"
	};
	QTimer *autoResizeTimer, *mouseFocusTimer;
	RootSubWindow *lastActiveMouse;
	QPoint lastCursorPos;

	QList<screeninfo> WALLPAPERS;
	void updateScreenPixmap(screeninfo *info); //used for recalculating the wallpaper pixmap based on file/area/scale as needed

	//Window Management
	QList<RootSubWindow*> WINDOWS;
	RootSubWindow* windowForId(WId id);
	void arrangeWindows(RootSubWindow *primary = 0, QString type = "", bool primaryonly = false);

	QScreen* screenUnderMouse();


public slots:
	void ResizeRoot();
	void ChangeWallpaper(QString id, RootWindow::ScaleType scale, QString file);
	    //Note: for "SingleColor" scaling the "file" variable should be "rgb(R,G,B)" or "#hexcode"
	void checkMouseFocus();

	void NewWindow(NativeWindow*);
	void CloseWindow(WId); //automatically connected for any new native window

	//Window arrangement functions - defined in "RootWindow-mgmt.cpp"
	void ArrangeWindows(WId primary = 0, QString type = "");
	void TileWindows(WId primary = 0, QString type = "");
	void CheckWindowPosition(WId, bool newwindow = false); //used after a "drop" to validate/snap/re-arrange window(s) as needed

private slots:

protected:
	void paintEvent(QPaintEvent *ev);

signals:
	void RegisterVirtualRoot(WId);
	void RootResized(QRect);
	void NewScreens(QStringList); // [screen_id_1, screen_id_2, etc..]
	void RemovedScreens(QStringList); // [screen_id_1, screen_id_2, etc..]
	void WorkspaceChanged(int);
	void MouseMoved();

};

#endif
