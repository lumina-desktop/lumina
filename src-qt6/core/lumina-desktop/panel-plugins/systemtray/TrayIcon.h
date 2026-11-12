//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  Note: The basic idea behind this class that that it puts the app window
//    in the same spot as the tray icon (to directly pass mouse events and such),
//    while keeping the tray icon visual in sync with the app window
//===========================================
#ifndef _LUMINA_PANEL_PLUGIN_SYSTEM_TRAY_ICON_H
#define _LUMINA_PANEL_PLUGIN_SYSTEM_TRAY_ICON_H

//Qt includes
#include <QWidget>
#include <QTimer>
#include <QPaintEvent>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QPixmap>
#include <QImage>
//#include <QWindow>
// libLumina includes
//#include <LuminaX11.h>

class TrayIcon : public QWidget{
	Q_OBJECT
public:
	TrayIcon(QWidget* parent = 0);
	~TrayIcon();

	void cleanup(); //about to be removed after window was detroyed

	WId appID(); //the ID for the attached application
	void attachApp(WId id);
	void setSizeSquare(int side);

public slots:
	void detachApp();
	void updateIcon();

private:
	WId IID, AID; //icon ID and app ID
	int badpaints;
	uint dmgID;
	int scalefactor;

protected:
	void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent *event);

signals:
	void BadIcon();
};
#endif
