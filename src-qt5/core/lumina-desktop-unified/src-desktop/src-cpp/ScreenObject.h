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

class ScreenObject : public QObject {
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

public:
	ScreenObject(QScreen *scrn = 0, QObject *parent = 0);

	static void RegisterType();

	Q_INVOKABLE QString name();
	Q_INVOKABLE QString background();
	Q_INVOKABLE int x();
	Q_INVOKABLE int y();
	Q_INVOKABLE int width();
	Q_INVOKABLE int height();

public slots:
	void setBackground(QString fileOrColor);

signals:
	void backgroundChanged();
	void geomChanged();
};

#endif
