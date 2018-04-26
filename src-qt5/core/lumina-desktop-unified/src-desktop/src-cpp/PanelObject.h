//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This is the base C++ object that is used to pass Panel info to the QML classes
//===========================================
#ifndef _LUMINA_DESKTOP_PANEL_OBJECT_H
#define _LUMINA_DESKTOP_PANEL_OBJECT_H
#include <QObject>
#include <QString>
#include <QScreen>

class PanelObject : public QObject {
	Q_OBJECT
	Q_PROPERTY( QString name READ name )
	Q_PROPERTY( QString background READ background NOTIFY backgroundChanged)
	Q_PROPERTY( int x READ x NOTIFY geomChanged)
	Q_PROPERTY( int y READ y NOTIFY geomChanged)
	Q_PROPERTY( int width READ width NOTIFY geomChanged)
	Q_PROPERTY( int height READ height NOTIFY geomChanged)
	Q_PROPERTY( bool isVertical READ isVertical NOTIFY geomChanged)
	Q_PROPERTY( QStringList plugins READ plugins NOTIFY pluginsChanged)

private:
	QString panel_id, bg;
	QRect geom, fullside_geom;
	QStringList panel_plugins;

public:
	PanelObject(QString id = "", QObject *parent = 0);

	static void RegisterType();

	Q_INVOKABLE QString name();
	Q_INVOKABLE QString background();
	Q_INVOKABLE int x();
	Q_INVOKABLE int y();
	Q_INVOKABLE int width();
	Q_INVOKABLE int height();
	Q_INVOKABLE bool isVertical();
	Q_INVOKABLE QStringList plugins();
	Q_INVOKABLE QRect geometry(){ return geom; }
	Q_INVOKABLE QRect fullSideGeometry(){ return fullside_geom; }

public slots:
	void setBackground(QString fileOrColor);
	void setGeometry(QRect newgeom);
	void syncWithSettings(QRect parent_geom);
	void setPlugins(QStringList plist);

signals:
	void backgroundChanged();
	void geomChanged();
	void pluginsChanged();
};

#endif
