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

#include <QAbstractNativeEventFilter>
#include <QList>
#include <QStringList>
#include <QX11Info>

#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/damage.h>
#include <xcb/xcb_atom.h>
#include "LSession.h"

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
#define SYSTEM_TRAY_REQUEST_DOCK 0
#define SYSTEM_TRAY_BEGIN_MESSAGE 1
#define SYSTEM_TRAY_CANCEL_MESSAGE 2

class XCBEventFilter : public QAbstractNativeEventFilter{
private:
	LSession *session;
	xcb_atom_t _NET_SYSTEM_TRAY_OPCODE;
	QList<xcb_atom_t> WinNotifyAtoms, SysNotifyAtoms;
	int TrayDmgFlag; //internal damage event offset value for the system tray
	bool stopping;

	void InitAtoms(){
	  //Initialize any special atoms that we need to save/use regularly
	  //NOTE: All the EWMH atoms are already saved in session->XCB->EWMH
	  WinNotifyAtoms.clear();
	    WinNotifyAtoms << session->XCB->EWMH._NET_WM_NAME \
					<< session->XCB->EWMH._NET_WM_VISIBLE_NAME \
					<< session->XCB->EWMH._NET_WM_ICON_NAME \
					<< session->XCB->EWMH._NET_WM_VISIBLE_ICON_NAME \
					<< session->XCB->EWMH._NET_WM_ICON \
					<< session->XCB->EWMH._NET_WM_ICON_GEOMETRY;

	  SysNotifyAtoms.clear();
	    SysNotifyAtoms << session->XCB->EWMH._NET_CLIENT_LIST \
					<< session->XCB->EWMH._NET_CLIENT_LIST_STACKING \
					<< session->XCB->EWMH._NET_CURRENT_DESKTOP \
					<< session->XCB->EWMH._NET_WM_STATE \
					<< session->XCB->EWMH._NET_ACTIVE_WINDOW \
					<< session->XCB->EWMH._NET_WM_ICON \
					<< session->XCB->EWMH._NET_WM_ICON_GEOMETRY;
	  //_NET_SYSTEM_TRAY_OPCODE
	  xcb_intern_atom_cookie_t cookie = xcb_intern_atom(QX11Info::connection(), 0, 23,"_NET_SYSTEM_TRAY_OPCODE");
	    xcb_intern_atom_reply_t *r = xcb_intern_atom_reply(QX11Info::connection(), cookie, NULL);
	    if(r){
	      _NET_SYSTEM_TRAY_OPCODE = r->atom;
	      free(r);
	    }
	}

public:
	XCBEventFilter(LSession *sessionhandle);
	void setTrayDamageFlag(int flag);
	void StopEventHandling(){ stopping = true; }

	//This function format taken directly from the Qt5.3 documentation
	virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *) Q_DECL_OVERRIDE;

};

#endif
