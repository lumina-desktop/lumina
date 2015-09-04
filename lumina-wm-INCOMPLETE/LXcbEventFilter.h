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


// ---------------------------------------------
// Lifted these mask definitions from the i3 window manager (BSD Licensed as well) 5/14/15 - Ken Moore
#define CHILD_EVENT_MASK (XCB_EVENT_MASK_PROPERTY_CHANGE |  \
                          XCB_EVENT_MASK_STRUCTURE_NOTIFY | \
                          XCB_EVENT_MASK_FOCUS_CHANGE)

/** The XCB_CW_EVENT_MASK for its frame */
#define FRAME_EVENT_MASK (XCB_EVENT_MASK_BUTTON_PRESS | /* ...mouse is pressed/released */                       \
                          XCB_EVENT_MASK_BUTTON_RELEASE |                                                        \
                          XCB_EVENT_MASK_POINTER_MOTION |        /* ...mouse is moved */                         \
                          XCB_EVENT_MASK_EXPOSURE |              /* ...our window needs to be redrawn */         \
                          XCB_EVENT_MASK_STRUCTURE_NOTIFY |      /* ...the frame gets destroyed */               \
                          XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | /* ...the application tries to resize itself */ \
                          XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |   /* ...subwindows get notifies */                \
                          XCB_EVENT_MASK_ENTER_WINDOW)           /* ...user moves cursor inside our window */

#define ROOT_EVENT_MASK (XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |                                       \
                         XCB_EVENT_MASK_BUTTON_PRESS |                                                \
                         XCB_EVENT_MASK_STRUCTURE_NOTIFY | /* when the user adds a screen (e.g. video \
                                                                  projector), the root window gets a  \
                                                                  ConfigureNotify */                  \
                         XCB_EVENT_MASK_POINTER_MOTION |                                              \
                         XCB_EVENT_MASK_PROPERTY_CHANGE |                                             \
                         XCB_EVENT_MASK_ENTER_WINDOW)
// ---------------------------------------------
			 
			 
class EventFilter : public QObject{
	Q_OBJECT
private:
	QAbstractNativeEventFilter* EF;
public:
	EventFilter();
	~EventFilter(){}

	void start();
	
signals:
	void NewInputEvent();
};
	
class XCBEventFilter : public QAbstractNativeEventFilter{
public:
	XCBEventFilter(EventFilter *parent);
	~XCBEventFilter(){}

	//This function format taken directly from the Qt5.3 documentation
	virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *) Q_DECL_OVERRIDE;
		
private:
	//LXCB *XCB;
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
	
	//Event handling functions (return true if event accepted/blocked)
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
