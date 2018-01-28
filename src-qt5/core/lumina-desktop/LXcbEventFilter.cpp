//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LXcbEventFilter.h"

//For all the XCB interactions and atoms
// is accessed via
//    session->XCB->EWMH.(atom name)
//    session->XCB->(do something)
#include <LuminaX11.h>
#include <QDebug>

XCBEventFilter::XCBEventFilter(LSession *sessionhandle) : QAbstractNativeEventFilter(){
  session = sessionhandle; //save this for interaction with the session later
  TrayDmgFlag = 0;
  stopping = false;
  session->XCB->SelectInput(QX11Info::appRootWindow()); //make sure we get root window events
  InitAtoms();
}

void XCBEventFilter::setTrayDamageFlag(int flag){
  //Special flag for system tray damage events
  TrayDmgFlag = flag + XCB_DAMAGE_NOTIFY; //save the whole flag (no calculations later)
}

//This function format taken directly from the Qt5.3 documentation
bool XCBEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *){
	if(stopping){ return false; } //don't do any parsing
	//qDebug() << "New Event";
	if(eventType=="xcb_generic_event_t"){
	  //qDebug() << " - XCB event";
	  //Convert to known event type (for X11 systems)
	   xcb_generic_event_t *ev = static_cast<xcb_generic_event_t *>(message);
	  //Now parse the event and emit signals as necessary
	  switch( ev->response_type & ~0x80){
//==============================
	    case XCB_PROPERTY_NOTIFY:
		//qDebug() << "Property Notify Event:";
	        //qDebug() << " - Root Window:" << QX11Info::appRootWindow();
		//qDebug() << " - Given Window:" << ((xcb_property_notify_event_t*)ev)->window;
		//System-specific property change
		if( ((xcb_property_notify_event_t*)ev)->window == QX11Info::appRootWindow() \
			&& ( ( ((xcb_property_notify_event_t*)ev)->atom == session->XCB->EWMH._NET_DESKTOP_GEOMETRY) \
			  ||  (((xcb_property_notify_event_t*)ev)->atom == session->XCB->EWMH._NET_WORKAREA) )){
		  session->RootSizeChange();
		}else if( ((xcb_property_notify_event_t*)ev)->window == QX11Info::appRootWindow() \
			&& ( ( ((xcb_property_notify_event_t*)ev)->atom == session->XCB->EWMH._NET_CURRENT_DESKTOP) )){
 		  //qDebug() << "Got Workspace Change";
		  session->emit WorkspaceChanged();
		  session->WindowPropertyEvent(); //make sure we update the lists again - some windows are now hidden
		}else if( SysNotifyAtoms.contains( ((xcb_property_notify_event_t*)ev)->atom ) ){
		  //Update the status/list of all running windows
		  session->WindowPropertyEvent();

		//window-specific property change
		}else if( WinNotifyAtoms.contains( ((xcb_property_notify_event_t*)ev)->atom ) ){
		  //Ping only that window
		  //session->WindowPropertyEvent( ((xcb_property_notify_event_t*)ev)->window );
		  session->WindowPropertyEvent();
	        }
		break;
//==============================
	    case XCB_CLIENT_MESSAGE:
		//qDebug() << "Client Message Event";
		//qDebug() << " - Root Window:" << QX11Info::appRootWindow();
		//qDebug() << " - Given Window:" << ((xcb_client_message_event_t*)ev)->window;
		if( TrayDmgFlag!=0 &&  ((xcb_client_message_event_t*)ev)->type == _NET_SYSTEM_TRAY_OPCODE && ((xcb_client_message_event_t*)ev)->format == 32){
		  //data32[0] is timestamp, [1] is opcode, [2] is  window handle
		  if(SYSTEM_TRAY_REQUEST_DOCK == ((xcb_client_message_event_t*)ev)->data.data32[1]){
		      session->SysTrayDockRequest( ((xcb_client_message_event_t*)ev)->data.data32[2] );
		  }
		  //Ignore the System Tray messages at the moment (let the WM handle it)

		//window-specific property changes
		/*}else if( ((xcb_client_message_event_t*)ev)->type == session->XCB->EWMH._NET_WM_STATE ){
		  if( session->XCB->WindowIsMaximized( ((xcb_client_message_event_t*)ev)->window ) ){
		    //Quick fix for maximized windows (since Fluxbox is not doing the STRUT detection properly)
		    session->adjustWindowGeom( ((xcb_client_message_event_t*)ev)->window );
		  }
		  session->WindowPropertyEvent( ((xcb_client_message_event_t*)ev)->window );*/
		}else if( WinNotifyAtoms.contains( ((xcb_client_message_event_t*)ev)->type ) ){
		  //Ping only that window
		  //session->WindowPropertyEvent( ((xcb_client_message_event_t*)ev)->window );
		  session->WindowPropertyEvent();
	        }
	        break;
//==============================
	    case XCB_DESTROY_NOTIFY:
		//qDebug() << "Window Closed Event";
		session->WindowClosedEvent( ( (xcb_destroy_notify_event_t*)ev )->window );
	        break;
//==============================
	    case XCB_CONFIGURE_NOTIFY:
		//qDebug() << "Configure Notify Event";
		session->WindowConfigureEvent( ((xcb_configure_notify_event_t*)ev)->window );
	        break;
//==============================
	    case XCB_SELECTION_CLEAR:
		//qDebug() << "Selection Clear Event";
		session->WindowSelectionClearEvent( ((xcb_selection_clear_event_t*)ev)->owner );
	        break;
//==============================
	    default:
		if(TrayDmgFlag!=0){
		  //if( (ev->response_type & ~0x80)==TrayDmgFlag){
		    session->WindowDamageEvent( ((xcb_damage_notify_event_t*)ev)->drawable );
		  //}
		}/*else{
	          qDebug() << "Default Event:" << (ev->response_type & ~0x80);
	        }*/
//==============================
	  }
	}
	//qDebug() << " - finished event";
	return false; //make sure the handling keeps going (transparent watching of events)
}
