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

#include "NativeWindow.h"


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
	void WindowPropertyChanged(WId, NativeWindow::Property);

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

	//System Tray Functions
	//QList<WId> trayApps(); //return the list of all current tray apps
	//bool startSystemTray();
	//bool stopSystemTray();

	//Window List Functions
	//QList<NativeWindow*> windowList();

private:
	NativeEventFilter *obj;
	/*QList<xcb_atom_t> WinNotifyAtoms, SysNotifyAtoms;
	xcb_atom_t _NET_SYSTEM_TRAY_OPCODE;
	void InitAtoms();

	bool BlockInputEvent(WId win = 0); //Checks the current state of the system to see if the event should be stopped
	WId InputWindow(WId win = 0); //Checks the window ID and determines if this is an external window or returns 0 (for desktop/root windows)
	Lumina::MouseButton MouseKey(int keycode); //convert the keycode into the mouse button code

	//System Tray Variables
	WId SystemTrayID;
	int TrayDmgID;
	QList<WId> RunningTrayApps;
	//System Tray functions
	void addTrayApp(WId);
	bool rmTrayApp(WId); //returns "true" if the tray app was found and removed
	void checkDamageID(WId);

	//Window List Variables
	QList<NativeWindow*> windows;
	QList<WId> waitingToShow;

	//Longer Event handling functions
	void SetupNewWindow(xcb_map_request_event_t  *ev);

	//bool ParseKeyPressEvent();
	//bool ParseKeyReleaseEvent();
	//bool ParseButtonPressEvent();
	//bool ParseButtonReleaseEvent();
	//bool ParseMotionEvent();
	void ParsePropertyEvent(xcb_property_notify_event_t *ev);
	//bool ParseClientMessageEvent();
	//bool ParseDestroyEvent();
	//bool ParseConfigureEvent();
	//bool ParseKeySelectionClearEvent();
	*/
};

#endif
