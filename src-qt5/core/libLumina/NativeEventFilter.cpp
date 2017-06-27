//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2015-2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "NativeEventFilter.h"
#include <QCoreApplication>
#include <QDebug>

//#include <xcb/xcb_aux.h>
//#include <xcb/damage.h>

//==================================================
// NOTE: All the XCB interactions and atoms are accessed via:
//    obj->XCB->EWMH.(atom name)
//    obj->XCB->(do something)
//==================================================

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

//#include <LuminaX11.h>
#include <QX11Info>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/damage.h>

#define DEBUG 0

//Special objects/variables for XCB parsing
static xcb_ewmh_connection_t EWMH;
//static LXCB *XCB = 0;
static xcb_atom_t _NET_SYSTEM_TRAY_OPCODE = 0;

inline void ParsePropertyEvent(xcb_property_notify_event_t *ev, NativeEventFilter *obj){
  //qDebug() << "Got Property Event:" << ev->window << ev->atom;
  NativeWindow::Property prop = NativeWindow::None;
  //Now determine which properties are getting changed, and update the native window as appropriate
  if(ev->atom == EWMH._NET_WM_NAME){ prop = NativeWindow::Title; }
  else if(ev->atom == EWMH._NET_WM_ICON){ prop = NativeWindow::Icon; }
  else if(ev->atom == EWMH._NET_WM_ICON_NAME){ prop = NativeWindow::ShortTitle; }
  else if(ev->atom == EWMH._NET_WM_DESKTOP){ prop = NativeWindow::Workspace; }
  else if(ev->atom == EWMH._NET_WM_WINDOW_TYPE ){ prop = NativeWindow::WinTypes; }
  else if( ev->atom == EWMH._NET_WM_STATE){ prop = NativeWindow::States; }
  //Send out the signal if necessary
  if(prop!=NativeWindow::None){
    obj->emit WindowPropertyChanged(ev->window, prop);
  }
}


//Constructor for the Event Filter wrapper
NativeEventFilter::NativeEventFilter() : QObject(){
  EF = new EventFilter(this);
  if(EWMH.nb_screens <=0){
   xcb_intern_atom_cookie_t *cookie = xcb_ewmh_init_atoms(QX11Info::connection(), &EWMH);
   if(!xcb_ewmh_init_atoms_replies(&EWMH, cookie, NULL) ){
     qDebug() << "Error with XCB atom initializations";
   }
  }
  if(_NET_SYSTEM_TRAY_OPCODE==0){
    //_NET_SYSTEM_TRAY_OPCODE
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(QX11Info::connection(), 0, 23,"_NET_SYSTEM_TRAY_OPCODE");
    xcb_intern_atom_reply_t *r = xcb_intern_atom_reply(QX11Info::connection(), cookie, NULL);
    if(r){
      _NET_SYSTEM_TRAY_OPCODE = r->atom;
      free(r);
    }
  }
}

void NativeEventFilter::start(){
  if(DEBUG){ qDebug() << " - Install event filter..."; }
  QCoreApplication::instance()->installNativeEventFilter(EF);
   if(DEBUG){ qDebug() << " - Run request check..."; }

}

void NativeEventFilter::stop(){
  QCoreApplication::instance()->installNativeEventFilter(0);
}

//=============================
//  EventFilter Class
//=============================

//Constructor for the XCB event filter
EventFilter::EventFilter(NativeEventFilter *parent) : QAbstractNativeEventFilter(){
  obj = parent;
}

//This function format taken directly from the Qt5.3 documentation
bool EventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *){
	//qDebug() << "New Event";
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
	 	//qDebug() << "Key Press Event"
	        obj->emit KeyPressed( ((xcb_key_press_event_t *) ev)->detail, ((xcb_key_press_event_t *) ev)->root );
		break;
	    case XCB_KEY_RELEASE:
		//This is a keyboard key release
		//qDebug() << "Key Release Event";
	        obj->emit KeyReleased( ((xcb_key_release_event_t *) ev)->detail, ((xcb_key_release_event_t *) ev)->root );
		break;
	    case XCB_BUTTON_PRESS:
		//This is a mouse button press
		//qDebug() << "Button Press Event";
                  obj->emit MousePressed( ((xcb_button_press_event_t *) ev)->detail, ((xcb_button_press_event_t *) ev)->root );
		break;
	    case XCB_BUTTON_RELEASE:
		//This is a mouse button release
		//qDebug() << "Button Release Event";
                  obj->emit MouseReleased( ((xcb_button_release_event_t *) ev)->detail, ((xcb_button_release_event_t *) ev)->root );
		break;
	    case XCB_MOTION_NOTIFY:
		//This is a mouse movement event
		qDebug() << "Motion Notify Event";
                 obj->emit MouseMovement();
	        break;
	    case XCB_ENTER_NOTIFY:
		//This is a mouse movement event when mouse goes over a new window
		//qDebug() << "Enter Notify Event";
                 obj->emit MouseEnterWindow( ((xcb_enter_notify_event_t *) ev)->root );
	        break;
	    case XCB_LEAVE_NOTIFY:
		//This is a mouse movement event when mouse goes leaves a window
		//qDebug() << "Leave Notify Event";
                 obj->emit MouseLeaveWindow( ((xcb_leave_notify_event_t *) ev)->root );
	        break;
//==============================
	    case XCB_EXPOSE:
		//qDebug() << "Expose Notify Event:";
		//qDebug() << " - Given Window:" << ((xcb_property_notify_event_t*)ev)->window;
		break;
//==============================
	    case XCB_MAP_NOTIFY:
		qDebug() << "Window Map Event:" << ((xcb_map_notify_event_t *)ev)->window;
                   obj->emit WindowPropertyChanged( ((xcb_map_notify_event_t *)ev)->window, NativeWindow::Visible );
		break; //This is just a notification that a window was mapped - nothing needs to change here
	    case XCB_MAP_REQUEST:
		qDebug() << "Window Map Request Event";
                   obj->emit WindowCreated( ((xcb_map_request_event_t *) ev)->window );
		break;
//==============================
	    case XCB_CREATE_NOTIFY:
		qDebug() << "Window Create Event";
	        break;
//==============================
	    case XCB_UNMAP_NOTIFY:
		qDebug() << "Window Unmap Event:" << ((xcb_unmap_notify_event_t *)ev)->window;
                  obj->emit WindowPropertyChanged( ((xcb_map_notify_event_t *)ev)->window, NativeWindow::Visible );
		break;
//==============================
	    case XCB_DESTROY_NOTIFY:
		qDebug() << "Window Closed Event:" << ((xcb_destroy_notify_event_t *)ev)->window;
                  obj->emit WindowDestroyed( ((xcb_destroy_notify_event_t *) ev)->window );
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
		qDebug() << "Property Notify Event:";
		ParsePropertyEvent((xcb_property_notify_event_t*)ev, obj);
		break;
//==============================
	    case XCB_CLIENT_MESSAGE:
		//qDebug() << "Client Message Event";
		//qDebug() << " - Given Window:" << ((xcb_client_message_event_t*)ev)->window;
		if( ((xcb_client_message_event_t*)ev)->type == _NET_SYSTEM_TRAY_OPCODE && ((xcb_client_message_event_t*)ev)->format == 32){
		  //data32[0] is timestamp, [1] is opcode, [2] is  window handle
		  if(SYSTEM_TRAY_REQUEST_DOCK == ((xcb_client_message_event_t*)ev)->data.data32[1]){
                        obj->emit TrayWindowCreated( ((xcb_client_message_event_t*)ev)->data.data32[2] );
		      //addTrayApp( ((xcb_client_message_event_t*)ev)->data.data32[2] );
		  }
		  //Ignore the System Tray messages at the moment
	        }
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
		//if( (ev->response_type & ~0x80)==TrayDmgID){
                    obj->emit PossibleDamageEvent( ((xcb_damage_notify_event_t*)ev)->drawable );
		  //checkDamageID( ((xcb_damage_notify_event_t*)ev)->drawable );
		//}else{
		  qDebug() << "Default Event:" << (ev->response_type & ~0x80);
		//}
//==============================
	  }
	}
	return false;
	//never stop event handling (this will not impact the X events themselves - just the internal Qt application)
}


//=========
//  PRIVATE
//=========


// WINDOW HANDLING FUNCTIONS
/*void EventFilter::SetupNewWindow(xcb_map_request_event_t  *ev){
  WId win = ev->window;

  bool ok = obj->XCB->WM_ManageWindow(win, true);
  //Quick check if this is a transient window if we could not manage it directly
  if(!ok){
    WId tran = obj->XCB->WM_ICCCM_GetTransientFor(win);
    if(tran!=win && tran!=0){
      win = tran;
      ok = obj->XCB->WM_ManageWindow(win);
    }
  }
  qDebug() << "New Window:" << win << obj->XCB->WM_ICCCM_GetClass(win) << " Managed:" << ok;
  obj->XCB->WM_Set_Active_Window(win);
  //Determing the requested geometry/location/management within the event,
  NativeWindow *nwin = new NativeWindow(win);
  QObject::connect(nwin, SIGNAL(RequestClose(WId)), obj, SLOT(TryCloseWindow(WId)) );
  QObject::connect(nwin, SIGNAL(RequestActivate(WId)), obj, SLOT(TryActivateWindow(WId)) );
  windows << nwin;
  bool show_now = !Lumina::SS->isLocked();
  if(!show_now){ waitingToShow << win; } //add to the list to get set visible later
  //populate the native window settings as they are right now
  nwin->setProperty(NativeWindow::Active, true);
  nwin->setProperty(NativeWindow::Visible, show_now);
  nwin->setProperty(NativeWindow::Workspace, obj->XCB->CurrentWorkspace());
  icccm_size_hints hints = obj->XCB->WM_ICCCM_GetNormalHints(win);
  if(!hints.isValid()){ hints = obj->XCB->WM_ICCCM_GetSizeHints(win); }
  if(hints.validMinSize()){ nwin->setProperty(NativeWindow::MinSize, QSize(hints.min_width,hints.min_height)); }
  if(hints.validMaxSize()){ nwin->setProperty(NativeWindow::MaxSize, QSize(hints.max_width,hints.max_height)); }
  if(hints.validBaseSize()){ nwin->setProperty(NativeWindow::Size, QSize(hints.base_width,hints.base_height)); }
  else if(hints.validSize()){ nwin->setProperty(NativeWindow::Size, QSize(hints.width, hints.height)); }
  nwin->setProperty(NativeWindow::Icon, obj->XCB->WM_Get_Icon(win));
  QString title = obj->XCB->WM_Get_Name(win);
    if(title.isEmpty()){ title = obj->XCB->WM_Get_Visible_Name(win); }
    if(title.isEmpty()){ title = obj->XCB->WM_ICCCM_GetName(win); }
  nwin->setProperty(NativeWindow::Title, title);
    title = obj->XCB->WM_Get_Icon_Name(win);
    if(title.isEmpty()){ title = obj->XCB->WM_Get_Visible_Icon_Name(win); }
    if(title.isEmpty()){ title = obj->XCB->WM_ICCCM_GetIconName(win); }
  nwin->setProperty(NativeWindow::ShortTitle, title);

  obj->emit WindowCreated(nwin);
}*/
