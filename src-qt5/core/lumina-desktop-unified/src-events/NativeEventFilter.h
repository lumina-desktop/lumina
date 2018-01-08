//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012-2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This class provides the XCB event handling/registrations that are needed
//===========================================
#ifndef _LUMINA_DESKTOP_NATIVE_EVENT_FILTER_H
#define _LUMINA_DESKTOP_NATIVE_EVENT_FILTER_H

#include <QAbstractNativeEventFilter>
#include <QObject>
#include <QByteArray>

#include <NativeWindowObject.h>


class NativeEventFilter : public QObject{
	Q_OBJECT
private:
	QAbstractNativeEventFilter* EF;
	WId WMFlag; //used to flag a running WM process

public:
	NativeEventFilter();
	~NativeEventFilter(){}

	void start();
	void stop();

signals:
	//Window Signals
	void WindowCreated(WId);
	void WindowDestroyed(WId);
	void WindowPropertyChanged(WId, NativeWindowObject::Property);
	void WindowPropertiesChanged(WId, QList<NativeWindowObject::Property>);
	void WindowPropertyChanged(WId, NativeWindowObject::Property, QVariant);
	void WindowPropertiesChanged(WId, QList<NativeWindowObject::Property>, QList<QVariant>);
	void RequestWindowPropertyChange(WId, NativeWindowObject::Property, QVariant);
	void RequestWindowPropertiesChange(WId, QList<NativeWindowObject::Property>, QList<QVariant>);

	//System Tray Signals
	void TrayWindowCreated(WId);
	void TrayWindowDestroyed(WId);

	//Miscellaneos Signals
	void PossibleDamageEvent(WId);

	//Input Event Signals
	void KeyPressed(int, WId);
	void KeyReleased(int, WId);
	void MousePressed(int, WId);
	void MouseReleased(int, WId);
	void MouseMovement();
	void MouseEnterWindow(WId);
	void MouseLeaveWindow(WId);
};

class EventFilter : public QAbstractNativeEventFilter{
public:
	EventFilter(NativeEventFilter *parent);
	~EventFilter(){}

	virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *);

private:
	NativeEventFilter *obj;
};

#endif
