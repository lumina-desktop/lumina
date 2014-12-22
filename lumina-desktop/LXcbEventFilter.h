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
#include <QDebug>
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

Also: it appears that the Xlib "Window" is identical to an XCB "xcb_window_t"
*/

//SYSTEM TRAY STANDARD DEFINITIONS
#define SYSTEM_TRAY_REQUEST_DOCK 0
#define SYSTEM_TRAY_BEGIN_MESSAGE 1
#define SYSTEM_TRAY_CANCEL_MESSAGE 2

class XCBEventFilter : public QAbstractNativeEventFilter{
private:
	LSession *session;
	QList<xcb_atom_t> atoms;
	xcb_atom_t _NET_SYSTEM_TRAY_OPCODE;
	int TrayDmgFlag; //internal damage event offset value for the system tray

	void checkClientMessage(xcb_client_message_event_t* event){
	   if(event->type == _NET_SYSTEM_TRAY_OPCODE && event->format == 32){
		//data32[0] is timestamp, [1] is opcode, [2] is  window handle
		uint32_t opcode = event->data.data32[1];
		if(opcode==SYSTEM_TRAY_REQUEST_DOCK){
		      session->SysTrayDockRequest(event->data.data32[2]);
		}
		//Ignore the System Tray messages at the moment (let the WM handle it)
	   }
	}
	
	void InitAtoms(){
	   atoms.clear();
	   QStringList names;
	    //List all the atoms that we want to detect for proprty changes
	    names << "_NET_CLIENT_LIST" << "_NET_ACTIVE_WINDOW" << "_NET_WM_NAME" << "_NET_WM_VISIBLE_NAME" \
			<< "_NET_WM_ICON_NAME" << "_NET_WM_VISIBLE_ICON_NAME" << "_NET_WM_STATE";
	   
	  xcb_connection_t *c = xcb_connect (NULL, NULL);
	  xcb_intern_atom_cookie_t *cs = (xcb_intern_atom_cookie_t *) malloc (names.length() * sizeof(xcb_intern_atom_cookie_t));
	  for(int i = 0; i < names.length(); ++i)
	    cs[i] = xcb_intern_atom (c, 0, names[i].length(), names[i].toStdString().c_str());

	  for(int i = 0; i < names.length(); ++i) {
	    xcb_intern_atom_reply_t *r = xcb_intern_atom_reply(c, cs[i], 0);
	    if(r){ atoms << r->atom; }
	    free(r);
	  }
	  //Also need the _net_system_tray_opcode atom as well
	  xcb_intern_atom_cookie_t cookie = xcb_intern_atom(c, 0, 23,"_NET_SYSTEM_TRAY_OPCODE");
	    xcb_intern_atom_reply_t *r = xcb_intern_atom_reply(c, cookie, 0);
	    if(r){ _NET_SYSTEM_TRAY_OPCODE = r->atom; }
	    free(r);
	}
	
public:
	XCBEventFilter(LSession *sessionhandle = 0) : QAbstractNativeEventFilter(){
	  session = sessionhandle; //save this for interaction with the session later
	  TrayDmgFlag = 0;
	  InitAtoms();
	}
	void setTrayDamageFlag(int flag){
	  //Special flag for system tray damage events
	  TrayDmgFlag = flag + XCB_DAMAGE_NOTIFY; //save the whole flag (no calculations later)
	}
	//This function format taken directly from the Qt5.3 documentation
	virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *) Q_DECL_OVERRIDE
	{
		//qDebug() << "New Event";
		if(eventType=="xcb_generic_event_t"){
		  //Convert to known event type (for X11 systems)
		   xcb_generic_event_t *ev = static_cast<xcb_generic_event_t *>(message);
		  //Now parse the event and emit signals as necessary
		  switch( ev->response_type & ~0x80){
		    case XCB_PROPERTY_NOTIFY:
			//qDebug() << "Property Notify Event:";
			if( atoms.contains( ((xcb_property_notify_event_t*)ev)->atom) ){
			  //qDebug() << " - launch session property event";
			  session->WindowPropertyEvent();		
			}
			break;
		    
		    case XCB_CLIENT_MESSAGE:
			//qDebug() << "Client Message Event";
		        checkClientMessage( (xcb_client_message_event_t*)ev );
		        break;
		    
		    case XCB_DESTROY_NOTIFY:
			//qDebug() << "Window Closed Event";
			session->WindowClosedEvent( ( (xcb_destroy_notify_event_t*)ev )->window );
		        break;
		    
		    case XCB_CONFIGURE_NOTIFY:
			//qDebug() << "Configure Notify Event";
			session->WindowConfigureEvent( ((xcb_configure_notify_event_t*)ev)->window );
		        break;
		    
		    case XCB_SELECTION_CLEAR:
			//qDebug() << "Selection Clear Event";
			session->WindowSelectionClearEvent( ((xcb_selection_clear_event_t*)ev)->owner );  
		        break;
		    
		    default:
			if( (ev->response_type & ~0x80)==TrayDmgFlag){
			  session->WindowDamageEvent( ((xcb_damage_notify_event_t*)ev)->drawable );
			}/*else{
		          qDebug() << "Default Event:" << (ev->response_type & ~0x80);
		        }*/
		  }
		}
		//qDebug() << " - finished event";
		return false; //make sure the handling keeps going (transparent watching of events)
	}
	
};

#endif
