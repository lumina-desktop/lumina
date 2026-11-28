//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This is the base C++ object that is used to pass Screen/Wallpaper info to the QML classes
//===========================================
#ifndef _LUMINA_DESKTOP_SCREEN_DESKTOP_OBJECT_H
#define _LUMINA_DESKTOP_SCREEN_DESKTOP_OBJECT_H
#include <QObject>
#include <QString>
#include <QScreen>
#include <QThread>

#include "PanelObject.h"

class ScreenObject : public QObject {
	Q_OBJECT
	Q_PROPERTY( QString name READ name )
	Q_PROPERTY( QString background READ background NOTIFY backgroundChanged)
	Q_PROPERTY( int x READ x NOTIFY geomChanged)
	Q_PROPERTY( int y READ y NOTIFY geomChanged)
	Q_PROPERTY( int width READ width NOTIFY geomChanged)
	Q_PROPERTY( int height READ height NOTIFY geomChanged)
	Q_PROPERTY( QStringList panels READ panels NOTIFY panelsChanged)
	Q_PROPERTY( QRect availableGeom READ availableGeometry NOTIFY availableGeomChanged)

private:
	QScreen *bg_screen;
	QString bg;
	QList<PanelObject*> panel_objects;
	QRect avail_geom;

public:
	ScreenObject(QScreen *scrn = 0, QObject *parent = 0);

	static void RegisterType();

	Q_INVOKABLE QString name();
	Q_INVOKABLE QString background();
	Q_INVOKABLE int x();
	Q_INVOKABLE int y();
	Q_INVOKABLE int width();
	Q_INVOKABLE int height();
	Q_INVOKABLE QStringList panels();
	Q_INVOKABLE PanelObject* panel(QString id);
	Q_INVOKABLE QRect geometry(){ return bg_screen->geometry(); }
	Q_INVOKABLE QRect availableGeometry();

	void setPanels(QList<PanelObject*> list);

	QList<PanelObject*> panelObjectList(){ return panel_objects; }

public slots:
	void setPanels(QStringList ids);
	void setBackground(QString fileOrColor);
	void updateAvailableGeometry();

signals:
	void backgroundChanged();
	void geomChanged();
	void availableGeomChanged();
	void panelsChanged();

	//Internal signals for thread-safety
	void changePanels(QStringList);

};

#endif
