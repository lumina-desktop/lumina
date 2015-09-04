//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LXcbEventFilter.h"

//For all the XCB interactions and atoms
// is accessed via 
//    XCB->EWMH.(atom name)
//    XCB->(do something)
#include <LuminaX11.h>
#include <QDebug>

//#include <xcb/screensaver.h>

// Also keep the root window/screen around for use in the filters
namespace L_XCB{
  xcb_screen_t *root_screen;
  xcb_window_t root;
}

//Constructor for the Event Filter wrapper
EventFilter::EventFilter() : QObject(){
  EF = new XCBEventFilter(this);
  L_XCB::root_screen = xcb_aux_get_screen(QX11Info::connection(), QX11Info::appScreen());
  L_XCB::root = L_XCB::root_screen->root;
}

void EventFilter::start(){
  QCoreApplication::instance()->installNativeEventFilter(EF);
  xcb_generic_error_t *status;
  xcb_connection_t *my_connection;
  xcb_void_cookie_t window_attributes;
  const unsigned int *returned;
  //Also ensure it gets root window events

    //Need the "real" root window (not the virtual root which Qt might return
   my_connection = QX11Info::connection();
   returned = (uint32_t *) ROOT_EVENT_MASK;
   window_attributes = xcb_change_window_attributes_checked(my_connection, L_XCB::root, XCB_CW_EVENT_MASK, returned);
   status = xcb_request_check(my_connection, window_attributes);

   // if( 0!= xcb_request_check( QX11Info::connection(), xcb_change_window_attributes_checked(QX11Info::connection(), L_XCB::root, XCB_CW_EVENT_MASK, (uint32_t[]){ROOT_EVENT_MASK} ) ) ){
   if (status)
   {
     qCritical() << "[ERROR] Unable to setup WM event retrieval. Is another WM running?";
     exit(1);
   }
   //xcb_screensaver_select_input(QX11Info::connection(),QX11Info::appRootWindow() , masks);
   xcb_flush(QX11Info::connection());
	
}
	
//Constructor for the XCB event filter
XCBEventFilter::XCBEventFilter(EventFilter *parent) : QAbstractNativeEventFilter(){
  obj = parent;
  InitAtoms();
}

//This function format taken directly from the Qt5.3 documentation
bool XCBEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *) Q_DECL_OVERRIDE
{
	//if(stopping){ return false; } //don't do any parsing
	//qDebug() << "New Event";
	bool stopevent = false;
	if(eventType=="xcb_generic_event_t"){
	  //Convert to known event type (for X11 systems)
	   xcb_generic_event_t *ev = static_cast<xcb_generic_event_t *>(message);
	  //Now parse the event and emit signals as necessary
	  switch( ev->response_type & ~0x80){
//==============================
	    case XCB_KEY_PRESS:
	 	qDebug() << "Key Press Event";
		obj->emit NewInputEvent();
		break;
	    case XCB_KEY_RELEASE:
		qDebug() << "Key Release Event";
		obj->emit NewInputEvent();
		break;
	    case XCB_BUTTON_PRESS:
		qDebug() << "Button Press Event";
		obj->emit NewInputEvent();
		break;
	    case XCB_BUTTON_RELEASE:
		qDebug() << "Button Release Event";
		obj->emit NewInputEvent();
		break;
	    case XCB_MOTION_NOTIFY:
		qDebug() << "Motion Notify Event";
		obj->emit NewInputEvent();
	        break;
//==============================
	    case XCB_PROPERTY_NOTIFY:
		qDebug() << "Property Notify Event:";
	        //qDebug() << " - Root Window:" << QX11Info::appRootWindow();
		//qDebug() << " - Given Window:" << ((xcb_property_notify_event_t*)ev)->window;
		//System-specific proprty change
		if( SysNotifyAtoms.contains( ((xcb_property_notify_event_t*)ev)->atom ) ){
		  //Update the status/list of all running windows
		  //session->WindowPropertyEvent();	
			
		//window-specific property change
		}else if( WinNotifyAtoms.contains( ((xcb_property_notify_event_t*)ev)->atom ) ){
		  //Ping only that window
		  //session->WindowPropertyEvent( ((xcb_property_notify_event_t*)ev)->window );
		  //session->WindowPropertyEvent();
	        }
		break;
//==============================	    
	    case XCB_CLIENT_MESSAGE:
		qDebug() << "Client Message Event";
		//qDebug() << " - Root Window:" << QX11Info::appRootWindow();
		//qDebug() << " - Given Window:" << ((xcb_client_message_event_t*)ev)->window;
		//if( TrayDmgFlag!=0 &&  ((xcb_client_message_event_t*)ev)->type == _NET_SYSTEM_TRAY_OPCODE && ((xcb_client_message_event_t*)ev)->format == 32){
		  //data32[0] is timestamp, [1] is opcode, [2] is  window handle
		  //if(SYSTEM_TRAY_REQUEST_DOCK == ((xcb_client_message_event_t*)ev)->data.data32[1]){
		      //session->SysTrayDockRequest( ((xcb_client_message_event_t*)ev)->data.data32[2] );
		  //}
		  //Ignore the System Tray messages at the moment (let the WM handle it)
		  
		//window-specific property changes
		/*}else if( ((xcb_client_message_event_t*)ev)->type == session->XCB->EWMH._NET_WM_STATE ){
		  if( session->XCB->WindowIsMaximized( ((xcb_client_message_event_t*)ev)->window ) ){
		    //Quick fix for maximized windows (since Fluxbox is not doing the STRUT detection properly)
		    session->adjustWindowGeom( ((xcb_client_message_event_t*)ev)->window );
		  }
		  session->WindowPropertyEvent( ((xcb_client_message_event_t*)ev)->window );*/
		//}else if( WinNotifyAtoms.contains( ((xcb_client_message_event_t*)ev)->type ) ){
		  //Ping only that window
		  //session->WindowPropertyEvent( ((xcb_client_message_event_t*)ev)->window );
		  //session->WindowPropertyEvent();
	        //}
	        break;
//==============================	    
	    case XCB_DESTROY_NOTIFY:
		qDebug() << "Window Closed Event";
		//session->WindowClosedEvent( ( (xcb_destroy_notify_event_t*)ev )->window );
	        break;
//==============================	    
	    case XCB_CONFIGURE_NOTIFY:
		qDebug() << "Configure Notify Event";
		//session->WindowConfigureEvent( ((xcb_configure_notify_event_t*)ev)->window );
	        break;
//==============================	    
	    case XCB_SELECTION_CLEAR:
		qDebug() << "Selection Clear Event";
		//session->WindowSelectionClearEvent( ((xcb_selection_clear_event_t*)ev)->owner );  
	        break;
//==============================	    
	    default:
		//if(TrayDmgFlag!=0){
		  //if( (ev->response_type & ~0x80)==TrayDmgFlag){
		    //session->WindowDamageEvent( ((xcb_damage_notify_event_t*)ev)->drawable );
		  //}
		//}
	        //else{
	          qDebug() << "Default Event:" << (ev->response_type & ~0x80);
	        //}
//==============================
	  }
	}
	//qDebug() << " - finished event";
	return stopevent;
}
