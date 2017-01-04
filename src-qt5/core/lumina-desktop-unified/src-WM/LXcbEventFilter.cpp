//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LXcbEventFilter.h"

//==================================================
// NOTE: All the XCB interactions and atoms are accessed via: 
//    LWM::SYSTEM->EWMH.(atom name)
//    LWM::SYSTEM->(do something)
// (LWM::SYSTEM is the global XCB structure)
//==================================================

#include <LuminaX11.h>
#include <QDebug>

//#include <xcb/screensaver.h>

#define DEBUG 1
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
  SSLocked = false;
  WMFlag = 0;
}

void EventFilter::start(){
  if(DEBUG){ qDebug() << " - Install event filter..."; }
  QCoreApplication::instance()->installNativeEventFilter(EF);
   if(DEBUG){ qDebug() << " - Run request check..."; }
   if(!LWM::SYSTEM->setupEventsForRoot()){
     qCritical() << "[ERROR] Unable to setup WM event retrieval. Is another WM running?";
     exit(1);
   }
  if(DEBUG){ qDebug() << " - Create WM ID Window"; }
  WMFlag = LWM::SYSTEM->WM_CreateWindow();
      LWM::SYSTEM->setupEventsForRoot(WMFlag);
      LWM::SYSTEM->WM_Set_Supporting_WM(WMFlag);
  QCoreApplication::instance()->flush();
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
//  INTERACTIVITY EVENTS
//==============================
	    case XCB_KEY_PRESS:
		//This is a keyboard key press
	 	//qDebug() << "Key Press Event";
		obj->emit NewInputEvent();
	        stopevent = BlockInputEvent( ((xcb_key_press_event_t *) ev)->root ); //use the main "root" window - not the child widget
		break;
	    case XCB_KEY_RELEASE:
		//This is a keyboard key release
		//qDebug() << "Key Release Event";
		obj->emit NewInputEvent();
	        stopevent = BlockInputEvent( ((xcb_key_release_event_t *) ev)->root ); //use the main "root" window - not the child widget
		break;
	    case XCB_BUTTON_PRESS:
		//This is a mouse button press
		//qDebug() << "Button Press Event";
		obj->emit NewInputEvent();
		stopevent = BlockInputEvent( ((xcb_button_press_event_t *) ev)->root ); //use the main "root" window - not the child widget
	        if(!stopevent){
		  //Activate the window right now if needed
		  if(LWM::SYSTEM->WM_Get_Active_Window()!=((xcb_button_press_event_t *) ev)->root){
		    LWM::SYSTEM->WM_Set_Active_Window( ((xcb_button_press_event_t *) ev)->root);
		  }
		}
		break;
	    case XCB_BUTTON_RELEASE:
		//This is a mouse button release
		//qDebug() << "Button Release Event";
	        //xcb_button_release_event_t *tmp = (xcb_button_release_event_t *)ev;
		stopevent = BlockInputEvent( ((xcb_button_release_event_t *) ev)->root ); //use the main "root" window - not the child widget
		break;
	    case XCB_MOTION_NOTIFY:
		//This is a mouse movement event
		//qDebug() << "Motion Notify Event";
		obj->emit NewInputEvent();
	        stopevent = BlockInputEvent( ((xcb_motion_notify_event_t *) ev)->root ); //use the main "root" window - not the child widget);
	        break;
	    case XCB_ENTER_NOTIFY:
		//This is a mouse movement event when mouse goes over a new window
		//qDebug() << "Enter Notify Event";
		obj->emit NewInputEvent();
	        stopevent = BlockInputEvent( ((xcb_enter_notify_event_t *) ev)->root );
	        break;
	    case XCB_LEAVE_NOTIFY:
		//This is a mouse movement event when mouse goes leaves a window
		//qDebug() << "Leave Notify Event";
		obj->emit NewInputEvent();
	        stopevent = BlockInputEvent();
	        break;
//==============================
	    case XCB_EXPOSE:
		//qDebug() << "Expose Notify Event:";
		//qDebug() << " - Given Window:" << ((xcb_property_notify_event_t*)ev)->window;
		break;
//==============================
	    case XCB_MAP_NOTIFY:
		break; //This is just a notification that a window was mapped - nothing needs to change here
	    case XCB_MAP_REQUEST:
		qDebug() << "Window Map Request Event";
	        obj->emit ModifyWindow( ((xcb_map_request_event_t *) ev)->window, LWM::Show);
		break;
//==============================	    
	    case XCB_CREATE_NOTIFY:
		qDebug() << "Window Create Event";
	        break;
//==============================
	    case XCB_UNMAP_NOTIFY:
		qDebug() << "Window Unmap Event";
		obj->emit ModifyWindow( ((xcb_unmap_notify_event_t *)ev)->window, LWM::Hide);
		break;
//==============================	    
	    case XCB_DESTROY_NOTIFY:
		qDebug() << "Window Closed Event";
	        obj->emit WindowClosed( ((xcb_destroy_notify_event_t *) ev)->window );
	        break;
//==============================
	    case XCB_FOCUS_IN:
		//qDebug() << "Focus In Event:";
		break;	    
//==============================
	    case XCB_FOCUS_OUT:
		//qDebug() << "Focus Out Event:";
		break;
//==============================
	    case XCB_PROPERTY_NOTIFY:
		//qDebug() << "Property Notify Event:";
		//qDebug() << " - Given Window:" << ((xcb_property_notify_event_t*)ev)->window;
		break;
//==============================	    
	    case XCB_CLIENT_MESSAGE:
		//qDebug() << "Client Message Event";
		//qDebug() << " - Given Window:" << ((xcb_client_message_event_t*)ev)->window;
	        break;
//==============================	    
	    case XCB_CONFIGURE_NOTIFY:
		//qDebug() << "Configure Notify Event";
	        break;
//==============================	    
	    case XCB_CONFIGURE_REQUEST:
		//qDebug() << "Configure Request Event";
	        break;
//==============================	    
	    case XCB_SELECTION_CLEAR:
		//qDebug() << "Selection Clear Event";
	        break;
//==============================	    
	    case 85: //not sure what event this is - but it seems to come up very often (just hide the notice)
	    case 0:
	    case XCB_GE_GENERIC:
		break; //generic event - don't do anything special
	    default:
		qDebug() << "Default Event:" << (ev->response_type & ~0x80);
//==============================
	  }
	}
	return false; 
	//never stop event handling (this will not impact the X events themselves - just the internal screensaver/WM/widgets)
}

bool XCBEventFilter::BlockInputEvent(WId win){
  //Checks the current state of the WM and sets the stop flag as needed
  // - Always let the screensaver know about the event first (need to reset timers and such)
  obj->emit NewInputEvent();
  // - Check the state of the screensaver
  if(obj->SSLocked){ qDebug() << "SS Locked"; return true; }
  // - Check the state of any fullscreen apps
  else if( win!=0 && !obj->FS_WINS.isEmpty()){
    if(!obj->FS_WINS.contains(win) ){
      //If this event is for an app underneath a fullscreen window - stop it
      if(obj->FS_WINS.length() == QApplication::desktop()->screenCount()){ qDebug() << "Screens Covered"; return true; } //all screens covered right now
    }
  }
  return false;
}
