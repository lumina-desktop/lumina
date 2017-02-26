//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2015-2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LXcbEventFilter.h"

#include <xcb/xcb_aux.h>
#include <xcb/damage.h>

//==================================================
// NOTE: All the XCB interactions and atoms are accessed via: 
//    obj->XCB->EWMH.(atom name)
//    obj->XCB->(do something)
//==================================================
#include "global-objects.h"

//SYSTEM TRAY STANDARD DEFINITIONS
#define SYSTEM_TRAY_REQUEST_DOCK 0
#define SYSTEM_TRAY_BEGIN_MESSAGE 1
#define SYSTEM_TRAY_CANCEL_MESSAGE 2

#include <xcb/xcb_keysyms.h>

#define DEBUG 0

// Also keep the root window/screen around for use in the filters
namespace L_XCB{
  xcb_screen_t *root_screen;
  xcb_window_t root;
}

//Constructor for the Event Filter wrapper
EventFilter::EventFilter() : QObject(){
  XCB = new LXCB();
  EF = new XCBEventFilter(this);
  L_XCB::root_screen = xcb_aux_get_screen(QX11Info::connection(), QX11Info::appScreen());
  L_XCB::root = L_XCB::root_screen->root;
  WMFlag = 0;
}

void EventFilter::start(){
  if(DEBUG){ qDebug() << " - Install event filter..."; }
  QCoreApplication::instance()->installNativeEventFilter(EF);
   if(DEBUG){ qDebug() << " - Run request check..."; }
   if(!XCB->setupEventsForRoot()){
     qCritical() << "[ERROR] Unable to setup WM event retrieval. Is another WM running?";
     exit(1);
   }
  if(DEBUG){ qDebug() << " - Create WM ID Window"; }
  WMFlag = XCB->WM_CreateWindow();
      XCB->setupEventsForRoot(WMFlag);
      XCB->WM_Set_Supporting_WM(WMFlag);

  XCB->WM_Set_Root_Supported(); //announce all the various options that the WM supports
  static_cast<XCBEventFilter*>(EF)->startSystemTray();

  QCoreApplication::instance()->flush();
}

void EventFilter::stop(){
  static_cast<XCBEventFilter*>(EF)->stopSystemTray();
}

//Session Interaction/Information
QList<WId> EventFilter::currentTrayApps(){
  return static_cast<XCBEventFilter*>(EF)->trayApps();
}

unsigned int EventFilter::currentWorkspace(){
return XCB->CurrentWorkspace();
}

QList<NativeWindow*> EventFilter::currentWindows(){
  return static_cast<XCBEventFilter*>(EF)->windowList();
}

// === PUBLIC SLOTS ===
void EventFilter::RegisterVirtualRoot(WId id){
  XCB->WM_Set_Virtual_Roots( QList<WId>() << id );
}

void EventFilter::TryCloseWindow(WId id){
  XCB->WM_CloseWindow(id, false); //do not force close
}

void EventFilter::TryActivateWindow(WId id){
  XCB->WM_Set_Active_Window(id);
}
//=============================
//  XCBEventFilter Class
//=============================

//Constructor for the XCB event filter
XCBEventFilter::XCBEventFilter(EventFilter *parent) : QAbstractNativeEventFilter(){
  obj = parent;
  SystemTrayID = 0;
  TrayDmgID = 0;
  InitAtoms();
}

void XCBEventFilter::InitAtoms(){
  //Initialize any special atoms that we need to save/use regularly
  //NOTE: All the EWMH atoms are already saved globally in obj->XCB->EWMH
  WinNotifyAtoms.clear();
    WinNotifyAtoms << obj->XCB->EWMH._NET_WM_NAME \
				<< obj->XCB->EWMH._NET_WM_VISIBLE_NAME \
				<< obj->XCB->EWMH._NET_WM_ICON_NAME \
				<< obj->XCB->EWMH._NET_WM_VISIBLE_ICON_NAME \
				<< obj->XCB->EWMH._NET_WM_ICON \
				<< obj->XCB->EWMH._NET_WM_ICON_GEOMETRY;
	
  SysNotifyAtoms.clear();
    SysNotifyAtoms << obj->XCB->EWMH._NET_CLIENT_LIST \
				<< obj->XCB->EWMH._NET_CLIENT_LIST_STACKING \
				<< obj->XCB->EWMH._NET_CURRENT_DESKTOP \
				<< obj->XCB->EWMH._NET_WM_STATE \
				<< obj->XCB->EWMH._NET_ACTIVE_WINDOW \
				<< obj->XCB->EWMH._NET_WM_ICON \
				<< obj->XCB->EWMH._NET_WM_ICON_GEOMETRY;

  //_NET_SYSTEM_TRAY_OPCODE
  xcb_intern_atom_cookie_t cookie = xcb_intern_atom(QX11Info::connection(), 0, 23,"_NET_SYSTEM_TRAY_OPCODE");
    xcb_intern_atom_reply_t *r = xcb_intern_atom_reply(QX11Info::connection(), cookie, NULL);
    if(r){ 
      _NET_SYSTEM_TRAY_OPCODE = r->atom; 
      free(r);
    }
}

//This function format taken directly from the Qt5.3 documentation
bool XCBEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *){
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
	        stopevent = BlockInputEvent( ((xcb_key_press_event_t *) ev)->root ); //use the main "root" window - not the child widget
	        if(!stopevent){ obj->emit KeyPressed( InputWindow(((xcb_key_press_event_t *) ev)->root), ((xcb_key_press_event_t *) ev)->detail ); }
		break;
	    case XCB_KEY_RELEASE:
		//This is a keyboard key release
		//qDebug() << "Key Release Event";
	        stopevent = BlockInputEvent( ((xcb_key_release_event_t *) ev)->root ); //use the main "root" window - not the child widget
	        if(!stopevent){ obj->emit KeyReleased( InputWindow(((xcb_key_release_event_t *) ev)->root), ((xcb_key_release_event_t *) ev)->detail ); }
		break;
	    case XCB_BUTTON_PRESS:
		//This is a mouse button press
		qDebug() << "Button Press Event";
		stopevent = BlockInputEvent( ((xcb_button_press_event_t *) ev)->root ); //use the main "root" window - not the child widget
	        if(!stopevent){
		  //Activate the window right now if needed
	         obj->emit MousePressed( InputWindow(((xcb_button_press_event_t *) ev)->root), MouseKey(((xcb_key_press_event_t *) ev)->detail) );
		  if(obj->XCB->WM_Get_Active_Window()!=((xcb_button_press_event_t *) ev)->root){
		    obj->XCB->WM_Set_Active_Window( ((xcb_button_press_event_t *) ev)->root);
		  }
		}
		break;
	    case XCB_BUTTON_RELEASE:
		//This is a mouse button release
		qDebug() << "Button Release Event";
	        //xcb_button_release_event_t *tmp = (xcb_button_release_event_t *)ev;
		stopevent = BlockInputEvent( ((xcb_button_release_event_t *) ev)->root ); //use the main "root" window - not the child widget
	        if(!stopevent){ obj->emit MouseReleased( InputWindow(((xcb_button_release_event_t *) ev)->root), MouseKey(((xcb_key_press_event_t *) ev)->detail) ); }
		break;
	    case XCB_MOTION_NOTIFY:
		//This is a mouse movement event
		//qDebug() << "Motion Notify Event";
	        stopevent = BlockInputEvent( ((xcb_motion_notify_event_t *) ev)->root ); //use the main "root" window - not the child widget);
	        break;
	    case XCB_ENTER_NOTIFY:
		//This is a mouse movement event when mouse goes over a new window
		//qDebug() << "Enter Notify Event";
	        stopevent = BlockInputEvent( ((xcb_enter_notify_event_t *) ev)->root );
	        break;
	    case XCB_LEAVE_NOTIFY:
		//This is a mouse movement event when mouse goes leaves a window
		//qDebug() << "Leave Notify Event";
	        stopevent = BlockInputEvent();
	        break;
//==============================
	    case XCB_EXPOSE:
		//qDebug() << "Expose Notify Event:";
		//qDebug() << " - Given Window:" << ((xcb_property_notify_event_t*)ev)->window;
		break;
//==============================
	    case XCB_MAP_NOTIFY:
		//qDebug() << "Window Map Event:" << ((xcb_map_notify_event_t *)ev)->window;
		if(Lumina::SS->isLocked()){ waitingToShow << ((xcb_map_notify_event_t *)ev)->window ; }
		else{
		  for(int i=0; i<windows.length(); i++){
		    if(windows[i]->id() == ((xcb_map_notify_event_t *)ev)->window){ windows[i]->setProperty(NativeWindow::Visible, true); break; }
		  }
		}
		break; //This is just a notification that a window was mapped - nothing needs to change here
	    case XCB_MAP_REQUEST:
		//qDebug() << "Window Map Request Event";
		SetupNewWindow( ((xcb_map_request_event_t *) ev) );
		break;
//==============================	    
	    case XCB_CREATE_NOTIFY:
		//qDebug() << "Window Create Event";
	        break;
//==============================
	    case XCB_UNMAP_NOTIFY:
		//qDebug() << "Window Unmap Event:" << ((xcb_unmap_notify_event_t *)ev)->window;
		if(waitingToShow.contains(((xcb_unmap_notify_event_t *)ev)->window)){ waitingToShow.removeAll(((xcb_unmap_notify_event_t *)ev)->window); }
		for(int i=0; i<windows.length(); i++){
		  if(windows[i]->id() == ((xcb_unmap_notify_event_t *)ev)->window){ windows[i]->setProperty(NativeWindow::Visible, false); break; }
		}
		break;
//==============================	    
	    case XCB_DESTROY_NOTIFY:
		//qDebug() << "Window Closed Event:" << ((xcb_destroy_notify_event_t *)ev)->window;
		if( !rmTrayApp( ((xcb_destroy_notify_event_t *) ev)->window ) ){
		  //qDebug() <<" - Non-tray window";
		  for(int i=0; i<windows.length(); i++){
		    if(windows[i]->id() == ((xcb_destroy_notify_event_t *)ev)->window){ 
		      windows[i]->emit WindowClosed(windows[i]->id()); 
		      QTimer::singleShot(500, windows.takeAt(i), SLOT(deleteLater()) ); //give a few moments first, then clean up the object
		      break; 
		    }
		  }
		}
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
		ParsePropertyEvent((xcb_property_notify_event_t*)ev);
		break;
//==============================	    
	    case XCB_CLIENT_MESSAGE:
		//qDebug() << "Client Message Event";
		//qDebug() << " - Given Window:" << ((xcb_client_message_event_t*)ev)->window;
		if( ((xcb_client_message_event_t*)ev)->type == _NET_SYSTEM_TRAY_OPCODE && ((xcb_client_message_event_t*)ev)->format == 32){
		  //data32[0] is timestamp, [1] is opcode, [2] is  window handle
		  if(SYSTEM_TRAY_REQUEST_DOCK == ((xcb_client_message_event_t*)ev)->data.data32[1]){
		      addTrayApp( ((xcb_client_message_event_t*)ev)->data.data32[2] );
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
		  checkDamageID( ((xcb_damage_notify_event_t*)ev)->drawable );
		//}else{
		  qDebug() << "Default Event:" << (ev->response_type & ~0x80);
		//}
//==============================
	  }
	}
	return false; 
	//never stop event handling (this will not impact the X events themselves - just the internal screensaver/WM/widgets)
}

//System Tray Functions
QList<WId> XCBEventFilter::trayApps(){
   //return the list of all current tray apps
    //Check the validity of all the current tray apps (make sure nothing closed erratically)
    for(int i=0; i<RunningTrayApps.length(); i++){
      if(obj->XCB->WindowClass(RunningTrayApps[i]).isEmpty()){ 
        obj->emit Tray_AppClosed(RunningTrayApps.takeAt(i) ); 
        i--; 
      }
    }
  return RunningTrayApps;
}

bool XCBEventFilter::startSystemTray(){
  if(SystemTrayID != 0){ return true; } //already started
  RunningTrayApps.clear(); //nothing running yet
  SystemTrayID = obj->XCB->startSystemTray(0);
  if(SystemTrayID!=0){
    obj->XCB->SelectInput(SystemTrayID); //make sure TrayID events get forwarded here
    TrayDmgID = obj->XCB->GenerateDamageID(SystemTrayID);
    qDebug() << "System Tray Started Successfully";
    if(DEBUG){ qDebug() << " - System Tray Flags:" << TrayDmgID; }
  }
  return (SystemTrayID!=0);
}

bool XCBEventFilter::stopSystemTray(){
  if(SystemTrayID==0){ return true; } //already stopped
  qDebug() << "Stopping system tray...";
  //Close all the running Tray Apps
  QList<WId> tmpApps = RunningTrayApps;
  //RunningTrayApps.clear(); //clear this ahead of time so tray's do not attempt to re-access the apps
  //Close all the running tray apps
    for(int i=0; i<tmpApps.length(); i++){
      qDebug() << " - Stopping tray app:" << obj->XCB->WindowClass(tmpApps[i]);
      //Tray apps are special and closing the window does not close the app
      obj->XCB->KillClient(tmpApps[i]);
    }  
  //Now close down the tray backend
 obj->XCB->closeSystemTray(SystemTrayID);
  SystemTrayID = 0;
  TrayDmgID = 0; 
  return true;
}

QList<NativeWindow*> XCBEventFilter::windowList(){
  return windows;
}

//=========
//  PRIVATE
//=========
bool XCBEventFilter::BlockInputEvent(WId){
  //Checks the current state of the WM and sets the stop flag as needed
  // - Always let the screensaver know about the event first (need to reset timers and such)
  obj->emit NewInputEvent();
  // - Check the state of the screensaver
  if(Lumina::SS->isLocked()){ qDebug() << "SS Locked"; return true; }
  // - Check the state of any fullscreen apps
  /*else if( win!=0 && !obj->FS_WINS.isEmpty()){
    if(!obj->FS_WINS.contains(win) ){
      //If this event is for an app underneath a fullscreen window - stop it
      if(obj->FS_WINS.length() == QApplication::desktop()->screenCount()){ qDebug() << "Screens Covered"; return true; } //all screens covered right now
    }
  }*/
  return false;
}

WId XCBEventFilter::InputWindow(WId win){
  //check window and see if it is a desktop/root window (return 0) or an external app window
  if(win == L_XCB::root){ return 0; }
  QString cl = obj->XCB->WindowClass(win);
  qDebug() << "Got Input Event on window:" << cl;
  if(cl.toLower()=="lumina-desktop"){ return 0; }
  return win; //external app window
}

Lumina::MouseButton XCBEventFilter::MouseKey(int keycode){
  switch(keycode){
    case 1:
      return Lumina::LeftButton;
    case 3:
      return Lumina::RightButton;
    case 2:
      return Lumina::MidButton;
    case 4:
      return Lumina::WheelUp;
    case 5:
      return Lumina::WheelDown;
    case 6:
      return Lumina::WheelLeft;
    case 7:
      return Lumina::WheelRight;
    case 8:
      return Lumina::BackButton; //Not sure if this is correct yet (1/27/17)
    case 9:
      return Lumina::ForwardButton; //Not sure if this is correct yet (1/27/17)
    default:
      return Lumina::NoButton;
  }
}

//System Tray functions
void XCBEventFilter::addTrayApp(WId win){
  if(SystemTrayID==0){ return; }
  if(RunningTrayApps.contains(win)){ return; } //already managed
  qDebug() << "Session Tray: Window Added" << obj->XCB->WindowClass(win);
  RunningTrayApps << win;
  if(DEBUG){ qDebug() << "Tray List Changed"; }
  obj->emit Tray_AppAdded(win);
}

bool XCBEventFilter::rmTrayApp(WId win){
   //returns "true" if the tray app was found and removed
  if(SystemTrayID==0){ return false; }
  for(int i=0; i<RunningTrayApps.length(); i++){
    if(win==RunningTrayApps[i]){ 
      qDebug() << "Session Tray: Window Removed";
      RunningTrayApps.removeAt(i); 
      obj->emit Tray_AppClosed(win);
      return true;
    }
  }
  return false;
}

void XCBEventFilter::checkDamageID(WId id){
  if(RunningTrayApps.contains(id)){
    obj->emit Tray_AppUpdated(id);
  }else{
    //Could check for window damage ID's - but we should not need this
  }
}

// WINDOW HANDLING FUNCTIONS
void XCBEventFilter::SetupNewWindow(xcb_map_request_event_t  *ev){
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
}

void XCBEventFilter::ParsePropertyEvent(xcb_property_notify_event_t *ev){
 //First find the NativeWindow associated with the event
  NativeWindow *nwin = 0;
  for(int i=0; i<windows.length() && nwin==0; i++){
    if(windows[i]->id() == ev->window){ nwin = windows[i]; }
  }
  if(nwin==0){ return; } //unmanaged window - ignore this event
  qDebug() << "Got Property Event:" << ev->window << ev->atom;
  //Now determine which properties are getting changed, and update the native window as appropriate
  if(ev->atom == obj->XCB->EWMH._NET_WM_NAME){
    qDebug() << " - Found _NET_WM_NAME atom";
    nwin->setProperty(NativeWindow::Title, obj->XCB->WM_Get_Name(nwin->id()));
  }else if(ev->atom == obj->XCB->EWMH._NET_WM_ICON){
    qDebug() << " - Found _NET_WM_ICON atom";
    nwin->setProperty(NativeWindow::Icon, obj->XCB->WM_Get_Icon(nwin->id()));
  }else if(ev->atom == obj->XCB->EWMH._NET_WM_ICON_NAME){
    qDebug() << " - Found _NET_WM_ICON_NAME atom";
    nwin->setProperty(NativeWindow::ShortTitle, obj->XCB->WM_Get_Icon_Name(nwin->id()));
  }else if(ev->atom == obj->XCB->EWMH._NET_WM_DESKTOP){
    qDebug() << " - Found _NET_WM_DESKTOP atom";
    nwin->setProperty(NativeWindow::Workspace, obj->XCB->WM_Get_Desktop(nwin->id()));
  }else if(ev->atom == obj->XCB->EWMH._NET_WM_WINDOW_TYPE ){
    qDebug() << " - Found _NET_WM_WINDOW_TYPE atom";

  }else if( ev->atom == obj->XCB->EWMH._NET_WM_STATE){
    qDebug() << " - Found _NET_WM_STATE atom";
    
  }
  
}
