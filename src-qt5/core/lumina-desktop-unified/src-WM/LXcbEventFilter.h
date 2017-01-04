//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This class provides the XCB ->Xlib conversion necessary for Qt5 usage
//===========================================
#ifndef _LUMINA_DESKTOP_XCB_FILTER_H
#define _LUMINA_DESKTOP_XCB_FILTER_H

#include "GlobalDefines.h"


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

//SYSTEM TRAY STANDARD DEFINITIONS
//#define SYSTEM_TRAY_REQUEST_DOCK 0
//#define SYSTEM_TRAY_BEGIN_MESSAGE 1
//#define SYSTEM_TRAY_CANCEL_MESSAGE 2

			 
class EventFilter : public QObject{
	Q_OBJECT
private:
	QAbstractNativeEventFilter* EF;
	WId WMFlag; //used to flag a running WM process

public:
	EventFilter();
	~EventFilter(){}

	void start();
	
	//Public variables for the event filter to use/check
	QList<WId> FS_WINS; //Full-screen windows (1 per monitor) - used for hiding non-app events as needed
	bool SSLocked;
		
public slots:
	void StartedSS(){ SSLocked = true; }
	void StoppedSS(){ SSLocked = false; }
	void FullScreenChanged(QList<WId> fslist){ FS_WINS = fslist; }

signals:
	void NewInputEvent();
	void NewManagedWindow(WId);
	void WindowClosed(WId);
	void ModifyWindow(WId win, LWM::WindowAction);
};
	
class XCBEventFilter : public QAbstractNativeEventFilter{
public:
	XCBEventFilter(EventFilter *parent);
	~XCBEventFilter(){}

	virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *);
		
private:
	EventFilter *obj;
	QList<xcb_atom_t> WinNotifyAtoms, SysNotifyAtoms;

	void InitAtoms(){
	  //Initialize any special atoms that we need to save/use regularly
	  //NOTE: All the EWMH atoms are already saved globally in LWM::SYSTEM->EWMH
	  WinNotifyAtoms.clear();
	    WinNotifyAtoms << LWM::SYSTEM->EWMH._NET_WM_NAME \
					<< LWM::SYSTEM->EWMH._NET_WM_VISIBLE_NAME \
					<< LWM::SYSTEM->EWMH._NET_WM_ICON_NAME \
					<< LWM::SYSTEM->EWMH._NET_WM_VISIBLE_ICON_NAME \
					<< LWM::SYSTEM->EWMH._NET_WM_ICON \
					<< LWM::SYSTEM->EWMH._NET_WM_ICON_GEOMETRY;
		
	  SysNotifyAtoms.clear();
	    SysNotifyAtoms << LWM::SYSTEM->EWMH._NET_CLIENT_LIST \
					<< LWM::SYSTEM->EWMH._NET_CLIENT_LIST_STACKING \
					<< LWM::SYSTEM->EWMH._NET_CURRENT_DESKTOP \
					<< LWM::SYSTEM->EWMH._NET_WM_STATE \
					<< LWM::SYSTEM->EWMH._NET_ACTIVE_WINDOW \
					<< LWM::SYSTEM->EWMH._NET_WM_ICON \
					<< LWM::SYSTEM->EWMH._NET_WM_ICON_GEOMETRY;

	}
	
	bool BlockInputEvent(WId win = 0); //Checks the current state of the WM and sets the stop flag as needed
	
	//Longer Event handling functions
	//bool ParseKeyPressEvent();
	//bool ParseKeyReleaseEvent();
	//bool ParseButtonPressEvent();
	//bool ParseButtonReleaseEvent();
	//bool ParseMotionEvent();
	//bool ParsePropertyEvent();
	//bool ParseClientMessageEvent();
	//bool ParseDestroyEvent();
	//bool ParseConfigureEvent();
	//bool ParseKeySelectionClearEvent();
	
	

	
};

#endif
