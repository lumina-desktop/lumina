//===========================================
//  Lumina-DE source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the XCB version of the NativeWindowSystem class,
//  used for interacting with the X11 display system on BSD/Linux/Unix systems
//===========================================
#include "NativeWindowSystem.h"

//Additional Qt includes
#include <QX11Info>
#include <QDebug>

//XCB Library functions
#include <xcb/xcb_ewmh.h>

//XCB Library includes
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/xproto.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb_aux.h>
#include <xcb/composite.h>
#include <xcb/damage.h>

//XLib includes (XCB Damage lib does not appear to register for damage events properly)
#include <X11/extensions/Xdamage.h>

//SYSTEM TRAY STANDARD DEFINITIONS
#define _NET_SYSTEM_TRAY_ORIENTATION_HORZ 0
#define _NET_SYSTEM_TRAY_ORIENTATION_VERT 1
#define SYSTEM_TRAY_REQUEST_DOCK 0
#define SYSTEM_TRAY_BEGIN_MESSAGE 1
#define SYSTEM_TRAY_CANCEL_MESSAGE 2

#define URGENCYHINT (1L << 8) //For window urgency detection

#define ROOT_WIN_EVENT_MASK (XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |  \
                         XCB_EVENT_MASK_BUTTON_PRESS | 	\
                         XCB_EVENT_MASK_STRUCTURE_NOTIFY |	\
			 XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |	\
                         XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |	\
                         XCB_EVENT_MASK_POINTER_MOTION | 	\
                         XCB_EVENT_MASK_PROPERTY_CHANGE | 	\
			 XCB_EVENT_MASK_FOCUS_CHANGE |	\
                         XCB_EVENT_MASK_ENTER_WINDOW)

//Internal XCB private objects class
class NativeWindowSystem::p_objects{
public:
	xcb_ewmh_connection_t EWMH; //This is where all the screen info and atoms are located
	QHash<QString, xcb_atom_t> ATOMS;
	xcb_screen_t *root_screen;
	xcb_window_t root_window, wm_window, tray_window;

	//Functions for setting up these objects as needed
	bool init_ATOMS(){
	  QStringList atoms;
	    atoms << "WM_TAKE_FOCUS" << "WM_DELETE_WINDOW" << "WM_PROTOCOLS" << "WM_CHANGE_STATE" << "_NET_SYSTEM_TRAY_OPCODE" << "_NET_SYSTEM_TRAY_ORIENTATION" << "_NET_SYSTEM_TRAY_VISUAL" << QString("_NET_SYSTEM_TRAY_S%1").arg(QString::number(QX11Info::appScreen()));
	    //Create all the requests for the atoms
	    QList<xcb_intern_atom_reply_t*> reply;
	    for(int i=0; i<atoms.length(); i++){
	      reply << xcb_intern_atom_reply(QX11Info::connection(), \
				xcb_intern_atom(QX11Info::connection(), 0, atoms[i].length(), atoms[i].toLocal8Bit()), NULL); 
	    }
	    //Now evaluate all the requests and save the atoms
	    for(int i=0; i<reply.length(); i++){ //NOTE: this will always be the same length as the "atoms" list
	      if(reply[i]!=0){
	        obj->ATOMS.insert(atoms[i], reply[i]->atom);
	        free(reply[i]); //done with this reply
	      }else{
	        //Invalid atom - could not be created
	        qDebug() << "Could not initialize XCB atom:" << atoms[i];
	      }
	    } //loop over reply
	  return (obj->ATOMS.keys.length() == atoms.length());
	}

	bool register_wm(){
	  uint32_t value_list[1] = {ROOT_WIN_EVENT_MASK};
	  xcb_generic_error_t *status = xcb_request_check( QX11Info::connection(), xcb_change_window_attributes_checked(QX11Info::connection(), root_window, XCB_CW_EVENT_MASK, value_list)); 
	  if(status!=0){ return false; }
	  uint32_t params[] = {1};
	  wm_window = xcb_generate_id(QX11Info::connection()); //need a new ID
	  xcb_create_window(QX11Info::connection(), root_screen->root_depth, \
		win, root_window, -1, -1, 1, 1, 0, \
		XCB_WINDOW_CLASS_INPUT_OUTPUT, root_screen->root_visual, \
		XCB_CW_OVERRIDE_REDIRECT, params);
	  if(wm_window==0){ return false; }
	  //Set the _NET_SUPPORTING_WM property on the root window first
	  xcb_ewmh_set_supporting_wm_check(&EWMH, root_window, wm_window);
	  //Also set this property on the child window (pointing to itself)
	  xcb_ewmh_set_supporting_wm_check(&EWMH, wm_window, wm_window);
	  //Now also setup the root event mask on the wm_window
	  status = xcb_request_check( QX11Info::connection(), xcb_change_window_attributes_checked(QX11Info::connection(), wm_window, XCB_CW_EVENT_MASK, value_list)); 
	  if(status!=0){ return false; }
	  return true;
	}

	bool startSystemTray{
	  xcb_atom_t _NET_SYSTEM_TRAY_S = ATOMS.value(QString("_NET_SYSTEM_TRAY_S%1").arg(QString::number(QX11Info::appScreen())) );
	  //Make sure that there is no other system tray running
	  xcb_get_selection_owner_reply_t *ownreply = xcb_get_selection_owner_reply(QX11Info::connection(), \
						xcb_get_selection_owner_unchecked(QX11Info::connection(), _NET_SYSTEM_TRAY_S), NULL);
	  if(ownreply==0){
	    qWarning() << " - Could not get owner selection reply";
	    return false;
	  }else if(ownreply->owner != 0){
	    free(ownreply);
	    qWarning() << " - An alternate system tray is currently in use";
	    return false;
	  }
	  free(ownreply);
	  //Now create the window to use (just offscreen)
	  //TODO
	}

}; //end private objects class


//inline functions for setting up the internal objects


// === PUBLIC ===
NativeWindowSystem::NativeWindowSystem() : QObject(){
  obj = 0; 
}

NativeWindowSystem::~NativeWindowSystem(){
  xcb_ewmh_connection_wipe(obj->EWMH);
  free(obj);
}

//Overarching start/stop functions
bool NativeWindowSystem::start(){
  //Initialize the XCB/EWMH objects
  if(obj==0){ 
    obj = new p_objects(); } //instantiate the private objects
    obj->wm_window = 0;
    obj->tray_window = 0;
    xcb_intern_atom_cookie_t *cookie = xcb_ewmh_init_atoms(QX11Info::connection(), &obj->EWMH);
    if(!xcb_ewmh_init_atoms_replies(&obj->EWMH, cookie, NULL) ){
      qDebug() << "Error with XCB atom initializations";
      return false;
    }
    obj->root_screen = xcb_aux_get_screen(QX11Info::connection(), QX11Info::appScreen());
    obj->root_window = obj->root_screen->root; //simplification for later - minor duplication of memory (unsigned int)
    //Initialize all the extra atoms that the EWMH object does not have
    if( !obj->init_ATOMS() ){ return false; }
  } //Done with private object init
  bool ok  = obj->register_wm();
  if(ok){ ok = obj->startSystemTray(); }
  return ok;
}

void NativeWindowSystem::stop(){

}

// === PRIVATE ===
void NativeWindowSystem::UpdateWindowProperties(NativeWindow* win, QList< NativeWindow::Property > props){

}


// === PUBLIC SLOTS ===
//These are the slots which are only used by the desktop system itself or the NativeWindowEventFilter
void NativeWindowSystem::RegisterVirtualRoot(WId){

}

//NativeWindowEventFilter interactions
void NativeWindowSystem::NewWindowDetected(WId){

}

void NativeWindowSystem::NewTrayWindowDetected(WId){

}

void NativeWindowSystem::WindowCloseDetected(WId){

}

void NativeWindowSystem::WindowPropertyChanged(WId, NativeWindow::Property){

}

void NativeWindowSystem::NewKeyPress(int keycode){

}

void NativeWindowSystem::NewKeyRelease(int keycode){

}

void NativeWindowSystem::NewMousePress(int buttoncode){

}

void NativeWindowSystem::NewMouseRelease(int buttoncode){

}

// === PRIVATE SLOTS ===
//These are the slots which are built-in and automatically connected when a new NativeWindow is created
void NativeWindowSystem::RequestPropertiesChange(WId win, QList<NativeWindow::Property> props, QList<QVariant> vals){
  //Find the window object associated with this id
  bool istraywin = false; //just in case we care later if it is a tray window or a regular window
  NativeWindow *WIN = findWindow(win);
  if(WIN==0){ istraywin = true; WIN = findTrayWindow(win); }
  if(WIN==0){ return; } //invalid window ID - no longer available
  //Now make any changes as needed
  
}

void NativeWindowSystem::RequestClose(WId win){
  //Send the window a WM_DELETE_WINDOW message
    xcb_client_message_event_t event;
    event.response_type = XCB_CLIENT_MESSAGE;
    event.format = 32;
    event.window = win;
    event.type = obj->ATOMS.value("WM_PROTOCOLS");
    event.data.data32[0] = obj->ATOMS.value("WM_DELETE_WINDOW");
    event.data.data32[1] = XCB_TIME_CURRENT_TIME; //CurrentTime;
    event.data.data32[2] = 0;
    event.data.data32[3] = 0;
    event.data.data32[4] = 0;

    xcb_send_event(QX11Info::connection(), 0, win,  XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT, (const char *) &event);
    xcb_flush(QX11Info::connection());
}

void NativeWindowSystem::RequestKill(WId win){
  xcb_kill_client(QX11Info::connection(), win);
}

void NativeWindowSystem::RequestPing(WId){
  xcb_ewmh_send_wm_ping(QX11Info::connection(), win, XCB_CURRENT_TIME);
}
