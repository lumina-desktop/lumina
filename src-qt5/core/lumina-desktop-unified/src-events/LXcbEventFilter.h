//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012-2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This class provides the XCB event handling/registrations that are needed
//===========================================
#ifndef _LUMINA_DESKTOP_XCB_FILTER_H
#define _LUMINA_DESKTOP_XCB_FILTER_H

#include "global-includes.h"


/*
List of XCB response types (since almost impossible to find good docs on XCB)
switch (xcb_generic_event_t*->response_type  & ~0x80)
case values:
XCB_KEY_[PRESS | RELEASE]
XCB_BUTTON_[PRESS | RELEASE]
XCB_MOTION_NOTIFY
XCB_ENTER_NOTIFY
XCB_LEAVE_NOTIFY
XCB_FOCUS_[IN | OUT]
XCB_KEYMAP_NOTIFY
XCB_EXPOSE
XCB_GRAPHICS_EXPOSURE
XCB_VISIBILITY_NOTIFY
XCB_CREATE_NOTIFY
XCB_DESTROY_NOTIFY
XCB_UNMAP_NOTIFY
XCB_MAP_[NOTIFY | REQUEST]
XCB_REPARENT_NOTIFY
XCB_CONFIGURE_[NOTIFY | REQUEST]
XCB_GRAVITY_NOTIFY
XCB_RESIZE_REQUEST
XCB_CIRCULATE_[NOTIFY | REQUEST]
XCB_PROPERTY_NOTIFY
XCB_SELECTION_[CLEAR | REQUEST | NOTIFY]
XCB_COLORMAP_NOTIFY
XCB_CLIENT_MESSAGE
*/

			
class EventFilter : public QObject{
	Q_OBJECT
private:
	QAbstractNativeEventFilter* EF;
	WId WMFlag; //used to flag a running WM process

public:
	EventFilter();
	~EventFilter(){}

	void start();
	void stop();

	//Public System Tray Functions
	QList<WId> currentTrayApps();

	//Public Session Interaction Functions
	unsigned int currentWorkspace();

	//Public Window Management Lists
	QList<NativeWindow*> currentWindows(); //always returned in creation-order (oldest first)
	
	//Variables/Functions needed by the XCBEventFilter class only (not really needed by anything else)
	LXCB *XCB; //used to interact with the X11 graphics subsystem

public slots:
	void RegisterVirtualRoot(WId);
	void TryCloseWindow(WId);
	void TryActivateWindow(WId);

signals:
	void NewInputEvent();
	void WindowCreated(NativeWindow*);
	
	// Session Signals
	void WorkspaceChanged(unsigned int);

	// System Tray Signals
	void Tray_AppAdded(WId); //new tray app registered
	void Tray_AppClosed(WId); //tray app de-registered
	void Tray_AppUpdated(WId); //tray app appearance changed (damage event)
	// Shortcut Signals
	void KeyPressed(WId, int);
	void KeyReleased(WId, int);
	void MousePressed(WId, Lumina::MouseButton);
	void MouseReleased(WId, Lumina::MouseButton);
};
	
class XCBEventFilter : public QAbstractNativeEventFilter{
public:
	XCBEventFilter(EventFilter *parent);
	~XCBEventFilter(){}

	virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *);
		
	//System Tray Functions
	QList<WId> trayApps(); //return the list of all current tray apps
	bool startSystemTray();
	bool stopSystemTray();

	//Window List Functions
	QList<NativeWindow*> windowList();

private:
	EventFilter *obj;
	QList<xcb_atom_t> WinNotifyAtoms, SysNotifyAtoms;
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
	
	

	
};

#endif
