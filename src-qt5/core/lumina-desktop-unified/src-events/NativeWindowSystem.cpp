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
#include <global-objects.h>

#define DISABLE_COMPOSITING 0

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
#include <xcb/screensaver.h>

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
                       XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |	\
                       XCB_EVENT_MASK_POINTER_MOTION | 	\
                       XCB_EVENT_MASK_PROPERTY_CHANGE | 	\
			 XCB_EVENT_MASK_FOCUS_CHANGE |	\
                       XCB_EVENT_MASK_ENTER_WINDOW)

#define NORMAL_WIN_EVENT_MASK (XCB_EVENT_MASK_BUTTON_PRESS | 	\
			XCB_EVENT_MASK_BUTTON_RELEASE | 	\
 			XCB_EVENT_MASK_POINTER_MOTION |	\
			XCB_EVENT_MASK_BUTTON_MOTION |	\
			XCB_EVENT_MASK_EXPOSURE |		\
			XCB_EVENT_MASK_STRUCTURE_NOTIFY |	\
			XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |	\
			XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |	\
			XCB_EVENT_MASK_ENTER_WINDOW | \
			XCB_EVENT_MASK_PROPERTY_CHANGE | \
			XCB_EVENT_MASK_FOCUS_CHANGE)

#define CLIENT_EVENT_MASK (XCB_EVENT_MASK_PROPERTY_CHANGE |  \
                          XCB_EVENT_MASK_STRUCTURE_NOTIFY | \
                          XCB_EVENT_MASK_FOCUS_CHANGE | \
                          XCB_EVENT_MASK_POINTER_MOTION)

#define FRAME_EVENT_MASK (XCB_EVENT_MASK_BUTTON_PRESS | \
                          XCB_EVENT_MASK_BUTTON_RELEASE | \
                          XCB_EVENT_MASK_POINTER_MOTION | \
                          XCB_EVENT_MASK_EXPOSURE | \
                          XCB_EVENT_MASK_STRUCTURE_NOTIFY | \
                          XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | \
                          XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | \
                          XCB_EVENT_MASK_ENTER_WINDOW)

inline void registerClientEvents(WId id, bool client = true){
  uint32_t values[] = {XCB_NONE};
  values[0] = client ? CLIENT_EVENT_MASK : FRAME_EVENT_MASK ;
  /*{ (XCB_EVENT_MASK_PROPERTY_CHANGE
			| XCB_EVENT_MASK_BUTTON_PRESS
			| XCB_EVENT_MASK_BUTTON_RELEASE
 			| XCB_EVENT_MASK_POINTER_MOTION
			| XCB_EVENT_MASK_BUTTON_MOTION
			| XCB_EVENT_MASK_EXPOSURE
			| XCB_EVENT_MASK_STRUCTURE_NOTIFY
			| XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
			| XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
			| XCB_EVENT_MASK_ENTER_WINDOW)
			};*/
  //xcb_change_window_attributes(QX11Info::connection(), id, XCB_CW_EVENT_MASK, values);
  xcb_screensaver_select_input(QX11Info::connection(), id, values[0]);
}

/*inline void registerClientEvents(WId id){
  uint32_t value_list[1] = {NORMAL_WIN_EVENT_MASK};
  xcb_change_window_attributes(QX11Info::connection(), id, XCB_CW_EVENT_MASK, value_list);
}*/

//Internal XCB private objects class
class NativeWindowSystem::p_objects{
public:
	xcb_ewmh_connection_t EWMH; //This is where all the screen info and atoms are located
	QHash<QString, xcb_atom_t> ATOMS;
	xcb_screen_t *root_screen;
	xcb_window_t root_window, wm_window, tray_window;

	//Functions for setting up these objects as needed
	bool init_ATOMS(){
	  xcb_intern_atom_cookie_t *cookie = xcb_ewmh_init_atoms(QX11Info::connection(), &EWMH);
	   if(!xcb_ewmh_init_atoms_replies(&EWMH, cookie, NULL) ){
	     qDebug() << "Error with XCB atom initializations";
	     return false;
	   }

	  QStringList atoms;
	    atoms << "WM_TAKE_FOCUS" << "WM_DELETE_WINDOW" << "WM_PROTOCOLS" << "_NET_WM_WINDOW_OPACITY"
		<< "WM_CHANGE_STATE" << "_NET_SYSTEM_TRAY_OPCODE" << "_NET_SYSTEM_TRAY_ORIENTATION" << "_XEMBED"
		<< "_NET_SYSTEM_TRAY_VISUAL" << QString("_NET_SYSTEM_TRAY_S%1").arg(QString::number(QX11Info::appScreen()));
	    //Create all the requests for the atoms
	    QList<xcb_intern_atom_reply_t*> reply;
	    for(int i=0; i<atoms.length(); i++){
	      reply << xcb_intern_atom_reply(QX11Info::connection(), \
				xcb_intern_atom(QX11Info::connection(), 0, atoms[i].length(), atoms[i].toLocal8Bit()), NULL);
	    }
	    //Now evaluate all the requests and save the atoms
	    for(int i=0; i<reply.length(); i++){ //NOTE: this will always be the same length as the "atoms" list
	      if(reply[i]!=0){
	        ATOMS.insert(atoms[i], reply[i]->atom);
	        free(reply[i]); //done with this reply
	      }else{
	        //Invalid atom - could not be created
	        qDebug() << "Could not initialize XCB atom:" << atoms[i];
	      }
	    } //loop over reply
	  return (ATOMS.keys().length() == atoms.length());
	}

	WId getTransientFor(WId win){
	  xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_transient_for_unchecked(QX11Info::connection(), win);
	  xcb_window_t trans;
	  if(1!= xcb_icccm_get_wm_transient_for_reply(QX11Info::connection(), cookie, &trans, NULL) ){
	    return win; //error in fetching transient window ID (or none found)
	  }else{
	    return trans;
	  }
}

	bool register_wm(){
	  uint32_t value_list[1] = {ROOT_WIN_EVENT_MASK};
	  xcb_generic_error_t *status = xcb_request_check( QX11Info::connection(), xcb_change_window_attributes_checked(QX11Info::connection(), root_window, XCB_CW_EVENT_MASK, value_list));
	  if(status!=0){ return false; }
	  uint32_t params[] = {1};
	  wm_window = xcb_generate_id(QX11Info::connection()); //need a new ID
	  xcb_create_window(QX11Info::connection(), root_screen->root_depth, \
		wm_window, root_window, -1, -1, 1, 1, 0, \
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

	bool start_system_tray(){
	  xcb_atom_t _NET_SYSTEM_TRAY_S = ATOMS.value( QString("_NET_SYSTEM_TRAY_S%1").arg(QString::number(QX11Info::appScreen())) );
	  //Make sure that there is no other system tray running
	  xcb_get_selection_owner_reply_t *ownreply = xcb_get_selection_owner_reply(QX11Info::connection(), \
						xcb_get_selection_owner_unchecked(QX11Info::connection(), _NET_SYSTEM_TRAY_S), NULL);
	  if(ownreply == 0){
	    qWarning() << " - Could not get owner selection reply";
	    return false;
	  }else if(ownreply->owner != 0){
	    free(ownreply);
	    qWarning() << " - An alternate system tray is currently in use";
	    return false;
	  }
	  free(ownreply);
	  //Now create the window to use (just offscreen)
	  tray_window = xcb_generate_id(QX11Info::connection()); //need a new ID
	  uint32_t params[] = {1};
	  xcb_create_window(QX11Info::connection(), root_screen->root_depth, \
		tray_window, root_screen->root, -1, -1, 1, 1, 0, \
		XCB_WINDOW_CLASS_INPUT_OUTPUT, root_screen->root_visual, \
		XCB_CW_OVERRIDE_REDIRECT, params);
	    //Now register this widget as the system tray
	  xcb_set_selection_owner(QX11Info::connection(), tray_window, _NET_SYSTEM_TRAY_S, XCB_CURRENT_TIME);
	  //Make sure that it was registered properly
	  ownreply = xcb_get_selection_owner_reply(QX11Info::connection(), \
						xcb_get_selection_owner_unchecked(QX11Info::connection(), _NET_SYSTEM_TRAY_S), NULL);
	  if(ownreply==0 || ownreply->owner != tray_window){
	    if(ownreply!=0){ free(ownreply); }
	    qWarning() << " - Could not register the system tray";
	    xcb_destroy_window(QX11Info::connection(), tray_window);
	    return false;
	  }
	  free(ownreply); //done with structure
	  //Now register the orientation of the system tray
	  uint32_t orient = _NET_SYSTEM_TRAY_ORIENTATION_HORZ;
	   xcb_change_property(QX11Info::connection(), XCB_PROP_MODE_REPLACE, tray_window, \
			ATOMS.value("_NET_SYSTEM_TRAY_ORIENTATION"), XCB_ATOM_CARDINAL, 32, 1, &orient);

	  //Now set the visual ID for the system tray (same as the root window, but TrueColor)
	  xcb_visualtype_t *type = xcb_aux_find_visual_by_attrs(root_screen, XCB_VISUAL_CLASS_TRUE_COLOR, 32);
	  if(type!=0){
	    xcb_change_property(QX11Info::connection(), XCB_PROP_MODE_REPLACE, tray_window, \
		ATOMS.value("_NET_SYSTEM_TRAY_VISUAL"), XCB_ATOM_VISUALID, 32, 1, &type->visual_id);
	  }else{
	    qWarning() << " - Could not set TrueColor visual for system tray";
	  }

	  //Finally, send out an X event letting others know that the system tray is up and running
	   xcb_client_message_event_t event;
	    event.response_type = XCB_CLIENT_MESSAGE;
	    event.format = 32;
	    event.window = root_screen->root;
	    event.type = EWMH.MANAGER; //MANAGER atom
	    event.data.data32[0] = XCB_TIME_CURRENT_TIME; //CurrentTime;
	    event.data.data32[1] = _NET_SYSTEM_TRAY_S; //_NET_SYSTEM_TRAY_S atom
	    event.data.data32[2] = tray_window;
	    event.data.data32[3] = 0;
	    event.data.data32[4] = 0;

	    xcb_send_event(QX11Info::connection(), 0, root_screen->root,  XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT, (const char *) &event);
	  return true;
	}

}; //end private objects class


//inline functions for setting up the internal objects


// === PUBLIC ===
NativeWindowSystem::NativeWindowSystem() : QObject(){
  obj = 0;
  pingTimer = 0;
  screenLocked = false;
}

NativeWindowSystem::~NativeWindowSystem(){
  xcb_ewmh_connection_wipe(&(obj->EWMH));
  free(obj);
}

//Overarching start/stop functions
bool NativeWindowSystem::start(){
  //Initialize the XCB/EWMH objects
  if(obj==0){
    obj = new p_objects();  //instantiate the private objects
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
  if(ok){
    setRoot_supportedActions();
    ok = obj->start_system_tray();
  }else{
    qWarning() << "Could not register the WM";
  }
  return ok;
}

void NativeWindowSystem::stop(){

}

void NativeWindowSystem::RegisterEventShortcut(Qt::Key key, bool set){
//RegisterEventShortcut( , set);
}

void NativeWindowSystem::RegisterEventShortcut(int keycode, bool set){
 //xcb_input_grab_device_button_checked
 //xcb_input_xi_passive_grab_device_unchecked
}

// === PRIVATE ===
NativeWindowObject* NativeWindowSystem::findWindow(WId id, bool checkRelated){
  //qDebug() << "Find Window:" << id;
  for(int i=0; i<NWindows.length(); i++){
    if(id==NWindows[i]->id() ){ return NWindows[i]; }
    else if(id==NWindows[i]->frameId() ){ return NWindows[i]; }
    if(checkRelated && NWindows[i]->isRelatedTo(id)){ return NWindows[i]; }
    //else if(!checkRelated && id==NWindows[i]->id()){ return NWindows[i]; }
  }
  //Check to see if this is a transient for some other window
  if(checkRelated){
    WId tid = obj->getTransientFor(id);
    if(tid!=id){ return findWindow(tid, checkRelated); } //call it recursively as needed
    //qDebug() << "  -- Could not find Window!";
  }
  return 0;
}

NativeWindowObject* NativeWindowSystem::findTrayWindow(WId id){
  for(int i=0; i<TWindows.length(); i++){
    if(TWindows[i]->id()==id){ return TWindows[i]; }
    else if(TWindows[i]->isRelatedTo(id)){ return TWindows[i]; }
  }
  return 0;
}

void NativeWindowSystem::UpdateWindowProperties(NativeWindowObject* win, QList< NativeWindowObject::Property > props){
  //Put the properties in logical groups as appropriate (some XCB calls return multiple properties)
  if(props.contains(NativeWindowObject::Title)){
    //Try the EWMH standards first
    // _NET_WM_NAME
    QString name;
    xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_name_unchecked(&obj->EWMH, win->id());
    if(cookie.sequence != 0){
      xcb_ewmh_get_utf8_strings_reply_t data;
      if( 1 == xcb_ewmh_get_wm_name_reply(&obj->EWMH, cookie, &data, NULL) ){
        name = QString::fromUtf8(data.strings, data.strings_len);
      }
    }
    if(name.isEmpty()){
      //_NET_WM_VISIBLE_NAME
      xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_visible_name_unchecked(&obj->EWMH, win->id());
      if(cookie.sequence != 0){
        xcb_ewmh_get_utf8_strings_reply_t data;
        if( 1 == xcb_ewmh_get_wm_visible_name_reply(&obj->EWMH, cookie, &data, NULL) ){
          name = QString::fromUtf8(data.strings, data.strings_len);
        }
      }
    }
    if(name.isEmpty()){
      //Now try the ICCCM standard
      xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_name_unchecked(QX11Info::connection(), win->id());
      xcb_icccm_get_text_property_reply_t reply;
      if(1 == xcb_icccm_get_wm_name_reply(QX11Info::connection(), cookie, &reply, NULL) ){
        name = QString::fromLocal8Bit(reply.name, reply.name_len);
        xcb_icccm_get_text_property_reply_wipe(&reply);
      }
    }
    win->setProperty(NativeWindowObject::Title, name);
  } //end TITLE property

  if(props.contains(NativeWindowObject::ShortTitle)){
    //Try the EWMH standards first
    // _NET_WM_ICON_NAME
    QString name;
    xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_icon_name_unchecked(&obj->EWMH, win->id());
    if(cookie.sequence != 0){
      xcb_ewmh_get_utf8_strings_reply_t data;
      if( 1 == xcb_ewmh_get_wm_icon_name_reply(&obj->EWMH, cookie, &data, NULL) ){
        name = QString::fromUtf8(data.strings, data.strings_len);
      }
    }
    if(name.isEmpty()){
      //_NET_WM_VISIBLE_ICON_NAME
      xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_visible_icon_name_unchecked(&obj->EWMH, win->id());
      if(cookie.sequence != 0){
        xcb_ewmh_get_utf8_strings_reply_t data;
        if( 1 == xcb_ewmh_get_wm_visible_icon_name_reply(&obj->EWMH, cookie, &data, NULL) ){
          name = QString::fromUtf8(data.strings, data.strings_len);
        }
      }
    }
    if(name.isEmpty()){
      //Now try the ICCCM standard
      xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_icon_name_unchecked(QX11Info::connection(), win->id());
      xcb_icccm_get_text_property_reply_t reply;
      if(1 == xcb_icccm_get_wm_icon_name_reply(QX11Info::connection(), cookie, &reply, NULL) ){
        name = QString::fromLocal8Bit(reply.name, reply.name_len);
        xcb_icccm_get_text_property_reply_wipe(&reply);
      }
    }
    win->setProperty(NativeWindowObject::ShortTitle, name);
  } //end SHORTTITLE property

  if(props.contains(NativeWindowObject::Icon)){
    //See if this is a tray icon first (different routine - entire app window is the icon)
    QIcon icon;
    if(win == findTrayWindow(win->id())){
      //Tray Icon Window
      icon.addPixmap( GetTrayWindowImage(win) );
      qDebug() << "Loaded Tray Icon" << !icon.isNull();

    }else{
      //Standard window
      //Fetch the _NET_WM_ICON for the window and return it as a QIcon
      xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_icon_unchecked(&obj->EWMH, win->id());
      xcb_ewmh_get_wm_icon_reply_t reply;
      if(1 == xcb_ewmh_get_wm_icon_reply(&obj->EWMH, cookie, &reply, NULL)){
        xcb_ewmh_wm_icon_iterator_t iter = xcb_ewmh_get_wm_icon_iterator(&reply);
        //Just use the first
        bool done =false;
        while(!done){
          //Now convert the current data into a Qt image
          // - first 2 elements are width and height (removed via XCB functions)
          // - data in rows from left to right and top to bottom
          QImage image(iter.width, iter.height, QImage::Format_ARGB32); //initial setup
	    uint* dat = iter.data;
	    //dat+=2; //remember the first 2 element offset
	    for(int i=0; i<image.byteCount()/4; ++i, ++dat){
	      ((uint*)image.bits())[i] = *dat;
	    }
          icon.addPixmap(QPixmap::fromImage(image)); //layer this pixmap onto the icon
          //Now see if there are any more icons available
          done = (iter.rem<1); //number of icons remaining
          if(!done){ xcb_ewmh_get_wm_icon_next(&iter); } //get the next icon data
        }
        xcb_ewmh_get_wm_icon_reply_wipe(&reply);
      }
    } //end type of window
    win->setProperty(NativeWindowObject::Icon, icon);
  } //end ICON property

  if(props.contains(NativeWindowObject::MinSize) || props.contains(NativeWindowObject::MaxSize)
	|| props.contains(NativeWindowObject::Size) || props.contains(NativeWindowObject::GlobalPos) ){
    //Try the ICCCM "Normal Hints" structure first (newer spec?)
    xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_normal_hints_unchecked(QX11Info::connection(), win->id());
    xcb_size_hints_t reply;
    bool ok = false;
    if(1==xcb_icccm_get_wm_normal_hints_reply(QX11Info::connection(), cookie, &reply, NULL) ){  ok = true; }
    else{
      //Could not find normal hints, try the older "size hints" instead
      cookie = xcb_icccm_get_wm_size_hints_unchecked(QX11Info::connection(), win->id(), XCB_ATOM_WM_SIZE_HINTS);
      if(1==xcb_icccm_get_wm_size_hints_reply(QX11Info::connection(), cookie, &reply, NULL) ){ ok = true; }
    }
    if(ok){
      bool initsize = win->property(NativeWindowObject::Size).isNull(); //initial window size
      if( (reply.flags&XCB_ICCCM_SIZE_HINT_US_POSITION)==XCB_ICCCM_SIZE_HINT_US_POSITION ){ win->setProperty(NativeWindowObject::GlobalPos, QPoint(reply.x,reply.y)); }
      if( (reply.flags&XCB_ICCCM_SIZE_HINT_US_SIZE)==XCB_ICCCM_SIZE_HINT_US_SIZE ){ win->setProperty(NativeWindowObject::Size, QSize(reply.width, reply.height)); }
      if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_POSITION)==XCB_ICCCM_SIZE_HINT_P_POSITION ){ win->setProperty(NativeWindowObject::GlobalPos, QPoint(reply.x,reply.y)); }
      if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_SIZE)==XCB_ICCCM_SIZE_HINT_P_SIZE ){ win->setProperty(NativeWindowObject::Size, QSize(reply.width, reply.height)); }
      if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_MIN_SIZE)==XCB_ICCCM_SIZE_HINT_P_MIN_SIZE ){ win->setProperty(NativeWindowObject::MinSize, QSize(reply.min_width, reply.min_height)); }
      if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_MAX_SIZE)==XCB_ICCCM_SIZE_HINT_P_MAX_SIZE ){ win->setProperty(NativeWindowObject::MaxSize, QSize(reply.max_width, reply.max_height)); }
      if( (reply.flags&XCB_ICCCM_SIZE_HINT_BASE_SIZE)==XCB_ICCCM_SIZE_HINT_BASE_SIZE && initsize ){ win->setProperty(NativeWindowObject::Size, QSize(reply.base_width, reply.base_height)); }
      //if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_RESIZE_INC)==XCB_ICCCM_SIZE_HINT_P_RESIZE_INC ){ hints.width_inc=reply.width_inc; hints.height_inc=reply.height_inc; }
      //if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_ASPECT)==XCB_ICCCM_SIZE_HINT_P_ASPECT ){ hints.min_aspect_num=reply.min_aspect_num; hints.min_aspect_den=reply.min_aspect_den; hints.max_aspect_num=reply.max_aspect_num; hints.max_aspect_den=reply.max_aspect_den;}
      //if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_WIN_GRAVITY)==XCB_ICCCM_SIZE_HINT_P_WIN_GRAVITY ){ hints.win_gravity=reply.win_gravity; }
    }
  } //end of geometry properties

  if(props.contains(NativeWindowObject::Name)){
    //Put the app/class name here (much more static than the "Title" properties
    xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_class_unchecked(QX11Info::connection(), win->id());
    xcb_icccm_get_wm_class_reply_t reply;
    if(1 == xcb_icccm_get_wm_class_reply(QX11Info::connection(), cookie, &reply, NULL) ){
      //Returns: "<instance name>::::<class name>"
      win->setProperty(NativeWindowObject::Name, ( QString::fromLocal8Bit(reply.instance_name)+"::::"+QString::fromLocal8Bit(reply.class_name) ));
      xcb_icccm_get_wm_class_reply_wipe(&reply);
    }
  } //end NAME property

  if(props.contains(NativeWindowObject::Workspace)){
    xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_desktop_unchecked(&obj->EWMH, win->id());
    uint32_t num = 0;
    int wkspace = -1;
    if(1==xcb_ewmh_get_wm_desktop_reply(&obj->EWMH, cookie, &num, NULL) ){
      if(num!=0xFFFFFFFF){ wkspace = num; }
    }/*else{
      //Error in fetching property (not set?)
      // - put it on the current screen
      out = WM_Get_Current_Desktop();
    }*/
    win->setProperty(NativeWindowObject::Workspace, wkspace);
  }
  if(props.contains(NativeWindowObject::FrameExtents)){
    //Just assign default values to this - need to automate it later
    //win->setProperty(NativeWindowObject::FrameExtents, QVariant::fromValue<QList<int> >(QList<int>() << 5 << 5 << 5+QFontMetrics(QFont()).height() << 5) );
  }
  if(props.contains(NativeWindowObject::RelatedWindows)){
    WId orig = win->id();
    WId tid = obj->getTransientFor(orig);
    QList<WId> list;
    while(tid != orig){
      list << tid;
      orig = tid;
      tid = obj->getTransientFor(orig);
    }
    win->setProperty(NativeWindowObject::RelatedWindows, QVariant::fromValue(list));
  }
  if(props.contains(NativeWindowObject::Visible)){
    xcb_get_window_attributes_reply_t *attr = xcb_get_window_attributes_reply(QX11Info::connection(), xcb_get_window_attributes(QX11Info::connection(), win->id()) , NULL);
    if(attr != 0){
      win->setProperty(NativeWindowObject::Visible, attr->map_state == XCB_MAP_STATE_VIEWABLE);
      free(attr);
    }
  }
  if(props.contains(NativeWindowObject::WinTypes)){
    QList< NativeWindowObject::Type> types;
    xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_window_type_unchecked(&obj->EWMH, win->id());
    xcb_ewmh_get_atoms_reply_t reply;
    if(1==xcb_ewmh_get_wm_window_type_reply(&obj->EWMH, cookie, &reply, NULL) ){
      for(unsigned int i=0; i<reply.atoms_len; i++){
        if(reply.atoms[i]==obj->EWMH._NET_WM_WINDOW_TYPE_DESKTOP){ types << NativeWindowObject::T_DESKTOP; }
        else if(reply.atoms[i]==obj->EWMH._NET_WM_WINDOW_TYPE_DOCK){ types << NativeWindowObject::T_DOCK; }
        else if(reply.atoms[i]==obj->EWMH._NET_WM_WINDOW_TYPE_TOOLBAR){ types << NativeWindowObject::T_TOOLBAR; }
        else if(reply.atoms[i]==obj->EWMH._NET_WM_WINDOW_TYPE_MENU){ types << NativeWindowObject::T_MENU; }
        else if(reply.atoms[i]==obj->EWMH._NET_WM_WINDOW_TYPE_UTILITY){ types << NativeWindowObject::T_UTILITY; }
        else if(reply.atoms[i]==obj->EWMH._NET_WM_WINDOW_TYPE_SPLASH){ types << NativeWindowObject::T_SPLASH; }
        else if(reply.atoms[i]==obj->EWMH._NET_WM_WINDOW_TYPE_DIALOG){ types << NativeWindowObject::T_DIALOG; }
        else if(reply.atoms[i]==obj->EWMH._NET_WM_WINDOW_TYPE_DROPDOWN_MENU){ types << NativeWindowObject::T_DROPDOWN_MENU; }
        else if(reply.atoms[i]==obj->EWMH._NET_WM_WINDOW_TYPE_POPUP_MENU){ types << NativeWindowObject::T_POPUP_MENU; }
        else if(reply.atoms[i]==obj->EWMH._NET_WM_WINDOW_TYPE_TOOLTIP){ types << NativeWindowObject::T_TOOLTIP; }
        else if(reply.atoms[i]==obj->EWMH._NET_WM_WINDOW_TYPE_NOTIFICATION){ types << NativeWindowObject::T_NOTIFICATION; }
        else if(reply.atoms[i]==obj->EWMH._NET_WM_WINDOW_TYPE_COMBO){ types << NativeWindowObject::T_COMBO; }
        else if(reply.atoms[i]==obj->EWMH._NET_WM_WINDOW_TYPE_DND){ types << NativeWindowObject::T_DND; }
        else if(reply.atoms[i]==obj->EWMH._NET_WM_WINDOW_TYPE_NORMAL){ types << NativeWindowObject::T_NORMAL; }
      }
    }
    if(types.isEmpty()){ types << NativeWindowObject::T_NORMAL; }
    win->setProperty(NativeWindowObject::WinTypes, QVariant::fromValue< QList<NativeWindowObject::Type> >(types) );
  }
}

void NativeWindowSystem::ChangeWindowProperties(NativeWindowObject* win, QList< NativeWindowObject::Property > props, QList<QVariant> vals){
  if(props.length() == 0 || vals.length()!=props.length()  || win ==0 ){ return; }
  //qDebug() << "Change Window Properties:" << props << vals;
  if(props.contains(NativeWindowObject::Title)){

  }
  if(props.contains(NativeWindowObject::ShortTitle)){

  }
  if(props.contains(NativeWindowObject::Icon)){

  }
  if(props.contains(NativeWindowObject::Size) || props.contains(NativeWindowObject::GlobalPos) ){
    xcb_configure_window_value_list_t  valList;
    //valList.x = 0; //Note that this is the relative position - should always be 0,0 relative to the embed widget
    //valList.y = 0;
    QSize sz = win->property(NativeWindowObject::Size).toSize();
    if(props.contains(NativeWindowObject::Size)){
      sz = vals[ props.indexOf(NativeWindowObject::Size) ] .toSize();
    }
    valList.width = sz.width();
    valList.height = sz.height();
    if(props.contains(NativeWindowObject::GlobalPos)){
      QPoint pt = vals[ props.indexOf(NativeWindowObject::GlobalPos) ] .toPoint();
      valList.x = pt.x();
      valList.y = pt.y();
    }else{
      valList.x = win->property(NativeWindowObject::GlobalPos).toPoint().x();
      valList.y = win->property(NativeWindowObject::GlobalPos).toPoint().y();
    }
    uint16_t mask = 0;
    mask = mask | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT | XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y;
    //qDebug() << "Configure window Geometry:" << sz;
    xcb_configure_window_aux(QX11Info::connection(), win->id(), mask, &valList);
  }
  if(props.contains(NativeWindowObject::Name)){

  }
  if(props.contains(NativeWindowObject::Workspace)){
    int num = vals[ props.indexOf(NativeWindowObject::Workspace) ].toInt();
    xcb_ewmh_set_wm_desktop(&obj->EWMH, win->id(), (num<0 ? 0xFFFFFFFF : qAbs(num) ) );
  }
  if(props.contains(NativeWindowObject::RelatedWindows)){

  }
  if(props.contains(NativeWindowObject::Visible)){
    //qDebug() << "Check Window Visibility:" << vals[ props.indexOf(NativeWindowObject::Visible) ];
    if( vals[ props.indexOf(NativeWindowObject::Visible) ].toBool() ){
      //qDebug() << " - Map it!";
      xcb_map_window(QX11Info::connection(), win->id());
    }else{
      //qDebug() << " - Unmap it!";
      xcb_unmap_window(QX11Info::connection(), win->id());
    }
  }
  if(props.contains(NativeWindowObject::Active)){
    //Only one window can be "Active" at a time - so only do anything if this window wants to be active
    if(vals[props.indexOf(NativeWindowObject::Active)].toBool() ){
      //Lower the currently active window (invisible window) to the bottom of the stack
      /*xcb_window_t cactive;
      if( 1 == xcb_ewmh_get_active_window_reply( &obj->EWMH,
		 xcb_ewmh_get_active_window_unchecked(&obj->EWMH, QX11Info::appScreen()),
		&cactive, NULL) ){
          uint32_t val = XCB_STACK_MODE_BELOW;
          xcb_configure_window(QX11Info::connection(),  cactive, XCB_CONFIG_WINDOW_STACK_MODE, &val);
      }*/

      /*uint32_t val = XCB_STACK_MODE_ABOVE;
      if(win->frameId()!=0){ xcb_configure_window(QX11Info::connection(),  win->frameId(), XCB_CONFIG_WINDOW_STACK_MODE, &val); }
      xcb_configure_window(QX11Info::connection(),  win->id(), XCB_CONFIG_WINDOW_STACK_MODE, &val);*/

      xcb_ewmh_set_active_window(&obj->EWMH, QX11Info::appScreen(), win->id() );
      //Also send the active window a message to take input focus
      xcb_set_input_focus(QX11Info::connection(), XCB_INPUT_FOCUS_PARENT, win->id(), XCB_CURRENT_TIME);
      //Send the window a WM_TAKE_FOCUS message
/*        xcb_client_message_event_t event;
        event.response_type = XCB_CLIENT_MESSAGE;
        event.format = 32;
        event.window = win->id();
        event.type = obj->ATOMS["WM_PROTOCOLS"];
        event.data.data32[0] = obj->ATOMS["WM_TAKE_FOCUS"];
        event.data.data32[1] = XCB_TIME_CURRENT_TIME; //CurrentTime;
        event.data.data32[2] = 0;
        event.data.data32[3] = 0;
        event.data.data32[4] = 0;

        xcb_send_event(QX11Info::connection(), 0, win->id(),  XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT, (const char *) &event);
        xcb_flush(QX11Info::connection());
*/
    }
  }

}

void NativeWindowSystem::SetupNewWindow(NativeWindowObject *win){
  if(!DISABLE_COMPOSITING){
    xcb_composite_redirect_window(QX11Info::connection(), win->id(), XCB_COMPOSITE_REDIRECT_MANUAL); //XCB_COMPOSITE_REDIRECT_[MANUAL/AUTOMATIC]);
    xcb_composite_redirect_subwindows(QX11Info::connection(), win->id(), XCB_COMPOSITE_REDIRECT_MANUAL); //AUTOMATIC); //XCB_COMPOSITE_REDIRECT_[MANUAL/AUTOMATIC]);

    //Now create/register the damage handler
    // -- XCB (Note: The XCB damage registration is completely broken at the moment - 9/15/15, Ken Moore)
    //  -- Retested 6/29/17 (no change) Ken Moore
    //xcb_damage_damage_t dmgID = xcb_generate_id(QX11Info::connection()); //This is a typedef for a 32-bit unsigned integer
    //xcb_damage_create(QX11Info::connection(), dmgID, win->id(), XCB_DAMAGE_REPORT_LEVEL_RAW_RECTANGLES);
    // -- XLib (Note: This is only used because the XCB routine above does not work - needs to be fixed upstream in XCB itself).
    Damage dmgID = XDamageCreate(QX11Info::display(), win->id(), XDamageReportRawRectangles);

    win->addDamageID( (uint) dmgID); //save this for later
    registerClientEvents(win->id());
  }else{
    /*
    //xcb_reparent_window(QX11Info::connection(), win->id(), this->winId(), 0, 0);
    //Also use a partial-composite here - make sure the window pixmap is available even when the window is obscured
    xcb_composite_redirect_window(QX11Info::connection(), win->id(), XCB_COMPOSITE_REDIRECT_AUTOMATIC);
    //xcb_composite_redirect_subwindows(QX11Info::connection(), win->id(), XCB_COMPOSITE_REDIRECT_MANUAL);
    //Also alert us when the window visual changes
    Damage dmgID = XDamageCreate(QX11Info::display(), win->id(), XDamageReportRawRectangles);

    win->addDamageID( (uint) dmgID); //save this for later
    */
  }
  //win->addFrameWinID(this->winId());
  //registerClientEvents(win->id());
}

QImage NativeWindowSystem::GetWindowImage(NativeWindowObject* win){
  //qDebug() << "Update Window Image:" << win->name();
  QRect geom(QPoint(0,0), win->property(NativeWindowObject::Size).toSize());
  QImage img; //(geom.width(), geom.height(), QImage::Format_ARGB32);
  //img.fill(QGuiApplication::palette().window());
  if(DISABLE_COMPOSITING){
    QList<QScreen*> screens = static_cast<QApplication*>( QApplication::instance() )->screens();
    if(!screens.isEmpty()){
        img = screens[0]->grabWindow(win->id(), geom.x(), geom.y(), geom.width(), geom.height()).toImage();
    }
  }else{
    //Pull the XCB pixmap out of the compositing layer
    xcb_pixmap_t pix = xcb_generate_id(QX11Info::connection());
    xcb_composite_name_window_pixmap(QX11Info::connection(), win->id(), pix);
    if(pix==0){ return QImage(); }

    //Convert this pixmap into a QImage
    //xcb_image_t *ximg = xcb_image_get(QX11Info::connection(), pix, 0, 0, this->width(), this->height(), ~0, XCB_IMAGE_FORMAT_Z_PIXMAP);
    xcb_image_t *ximg = xcb_image_get(QX11Info::connection(), pix, geom.x(), geom.y(), geom.width(), geom.height(), ~0, XCB_IMAGE_FORMAT_Z_PIXMAP);
    if(ximg == 0){ return QImage(); }
    QImage tmp(ximg->data, ximg->width, ximg->height, ximg->stride, QImage::Format_ARGB32_Premultiplied);
    img = tmp.copy(); //detach this image from the XCB data structures before we clean them up, otherwise the QImage will try to clean it up a second time on window close and crash
    xcb_image_destroy(ximg);

    //Cleanup the XCB data structures
    xcb_free_pixmap(QX11Info::connection(), pix);
  }
  return img;
  //win->setProperty(NativeWindowObject::WinImage, QVariant::fromValue<QImage>(img) );
}

QPixmap NativeWindowSystem::GetTrayWindowImage(NativeWindowObject *win){
  QPixmap pix;

  //Get the current QScreen (for XCB->Qt conversion)
  QList<QScreen*> scrnlist = QApplication::screens();
  if(scrnlist.isEmpty()){ return pix; }

  //Try to grab the given window directly with Qt
  pix = scrnlist[0]->grabWindow(win->id());
  if(!pix.isNull()){
    //done - go ahead and return the image
    return pix;
  }

  //Now try the more complicated XCB read mechanisms
  // -------------------------------
  //First get the pixmap from the XCB compositing layer (since the tray images are redirected there)
  xcb_pixmap_t pixmap = xcb_generate_id(QX11Info::connection());
  xcb_composite_name_window_pixmap(QX11Info::connection(), win->id(), pixmap);
  //Get the sizing information about the pixmap
  xcb_get_geometry_cookie_t Gcookie = xcb_get_geometry_unchecked(QX11Info::connection(), pixmap);
  xcb_get_geometry_reply_t *Greply = xcb_get_geometry_reply(QX11Info::connection(), Gcookie, NULL);
  if(Greply==0){ qDebug() << "[Tray Image] - Geom Fetch Error:"; return QPixmap(); } //Error in geometry detection

  //Now convert the XCB pixmap into an XCB image
  xcb_get_image_cookie_t GIcookie = xcb_get_image_unchecked(QX11Info::connection(), XCB_IMAGE_FORMAT_Z_PIXMAP, pixmap, 0, 0, Greply->width, Greply->height, 0xffffffff);
  xcb_get_image_reply_t *GIreply = xcb_get_image_reply(QX11Info::connection(), GIcookie, NULL);
  if(GIreply==0){ qDebug() << "[Tray Image] - Image Convert Error:"; return QPixmap(); } //Error in conversion
  uint8_t *GIdata = xcb_get_image_data(GIreply);
  uint32_t BPL = xcb_get_image_data_length(GIreply) / Greply->height; //bytes per line
  //Now convert the XCB image into a Qt Image
  QImage image(const_cast<uint8_t *>(GIdata), Greply->width, Greply->height, BPL, QImage::Format_ARGB32_Premultiplied);
  //Free the various data structures
  free(GIreply); //done with get image reply
  xcb_free_pixmap(QX11Info::connection(), pixmap); //done with the raw pixmap
  free(Greply); //done with geom reply

  return pix;

}

// === PUBLIC SLOTS ===
//These are the slots which are typically only used by the desktop system itself or the NativeEventFilter
void NativeWindowSystem::RegisterVirtualRoot(WId id){
  //Convert to XCB array
  xcb_window_t array[1];
  array[0] = id;
  //Set the property
  xcb_ewmh_set_virtual_roots(&obj->EWMH, QX11Info::appScreen(), 1, array);
  //Now also enable automatic compositing for children of this window
  //xcb_composite_redirect_window(QX11Info::connection(), id, XCB_COMPOSITE_REDIRECT_AUTOMATIC);
  //xcb_composite_redirect_subwindows(QX11Info::connection(), id, XCB_COMPOSITE_REDIRECT_AUTOMATIC);
}

void NativeWindowSystem::setRoot_supportedActions(){
//NET_WM standards (ICCCM implied - no standard way to list those)
  xcb_atom_t list[] = {obj->EWMH._NET_WM_NAME,
		obj->EWMH._NET_WM_ICON,
		obj->EWMH._NET_WM_ICON_NAME,
		obj->EWMH._NET_WM_DESKTOP,
		/*obj->ATOMS["_NET_WM_WINDOW_OPACITY"],*/
		/*_NET_WINDOW_TYPE (and all the various types - 15 in total*/
		obj->EWMH._NET_WM_WINDOW_TYPE, obj->EWMH._NET_WM_WINDOW_TYPE_DESKTOP, obj->EWMH._NET_WM_WINDOW_TYPE_DOCK,
		obj->EWMH._NET_WM_WINDOW_TYPE_TOOLBAR, obj->EWMH._NET_WM_WINDOW_TYPE_MENU, obj->EWMH._NET_WM_WINDOW_TYPE_UTILITY,
		obj->EWMH._NET_WM_WINDOW_TYPE_SPLASH, obj->EWMH._NET_WM_WINDOW_TYPE_DIALOG, obj->EWMH._NET_WM_WINDOW_TYPE_NORMAL,
		obj->EWMH._NET_WM_WINDOW_TYPE_DROPDOWN_MENU, obj->EWMH._NET_WM_WINDOW_TYPE_POPUP_MENU, obj->EWMH._NET_WM_WINDOW_TYPE_TOOLTIP,
		obj->EWMH._NET_WM_WINDOW_TYPE_NOTIFICATION, obj->EWMH._NET_WM_WINDOW_TYPE_COMBO, obj->EWMH._NET_WM_WINDOW_TYPE_DND,
		};
  xcb_ewmh_set_supported(&obj->EWMH, QX11Info::appScreen(), 20,list);
}

void NativeWindowSystem::setRoot_numberOfWorkspaces(QStringList names){
  if(names.isEmpty()){ names << "one"; }
  //First set the overall number of workspaces
  xcb_ewmh_set_number_of_desktops(&obj->EWMH, QX11Info::appScreen(), names.length());
  //Now set the names for the workspaces
  //EWMH LIBRARY BROKEN  - appears to be a mismatch in the function header (looking for a single char array, instead of a list of char arrays)
  // Ken Moore - 6/27/17
  /*
  char *array[ names.length() ];
  for(int i=0; i<names.length(); i++){array[i] = names[i].toUtf8().data(); } //Convert to an array of char arrays
  xcb_ewmh_set_desktop_names(&obj->EWMH, QX11Info::appScreen(), names.length(), array);
  */
}

void NativeWindowSystem::setRoot_currentWorkspace(int num){
  xcb_ewmh_set_current_desktop(&obj->EWMH, QX11Info::appScreen(), num);
}

void NativeWindowSystem::setRoot_clientList(QList<WId> list, bool stackorder){
  //convert the QList into a generic array
  xcb_window_t array[list.length()];
  for(int i=0; i<list.length(); i++){ array[i] = list[i]; }
  if(stackorder){
    xcb_ewmh_set_client_list_stacking(&obj->EWMH, QX11Info::appScreen(), list.length(), array);
  }else{
    xcb_ewmh_set_client_list(&obj->EWMH, QX11Info::appScreen(), list.length(), array);
  }
}

void NativeWindowSystem::setRoot_desktopGeometry(QRect geom){
  //This one is a combo function
  // This will set the "DESKTOP_VIEWPORT" property (point)
  //    as well as the "DESKTOP_GEOMETRY" property (size)
  //Turn the QList into xcb_ewmh_coordinates_t*
  xcb_ewmh_coordinates_t array[1];
  array[0].x=geom.x(); array[0].y=geom.y();
  //Now set the property
  xcb_ewmh_set_desktop_viewport(&obj->EWMH, QX11Info::appScreen(), 1, array);
  xcb_ewmh_set_desktop_geometry(&obj->EWMH, QX11Info::appScreen(), geom.width(), geom.height());
}

void NativeWindowSystem::setRoot_desktopWorkarea(QList<QRect> list){
  //Convert to the XCB/EWMH data structures
  xcb_ewmh_geometry_t array[list.length()];
  for(int i=0; i<list.length(); i++){
    array[i].x = list[i].x(); array[i].y = list[i].y();
    array[i].width = list[i].width(); array[i].height = list[i].height();
  }
  //Now set the property
  xcb_ewmh_set_workarea(&obj->EWMH, QX11Info::appScreen(), list.length(), array);
}

void NativeWindowSystem::setRoot_activeWindow(WId win){
  /*xcb_ewmh_set_active_window(&obj->EWMH, QX11Info::appScreen(), win);
  //Also send the active window a message to take input focus
  //Send the window a WM_TAKE_FOCUS message
    xcb_client_message_event_t event;
    event.response_type = XCB_CLIENT_MESSAGE;
    event.format = 32;
    event.window = win;
    event.type = obj->ATOMS["WM_PROTOCOLS"];
    event.data.data32[0] = obj->ATOMS["WM_TAKE_FOCUS"];
    event.data.data32[1] = XCB_TIME_CURRENT_TIME; //CurrentTime;
    event.data.data32[2] = 0;
    event.data.data32[3] = 0;
    event.data.data32[4] = 0;

    xcb_send_event(QX11Info::connection(), 0, win,  XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT, (const char *) &event);
    xcb_flush(QX11Info::connection());*/
}

int NativeWindowSystem::currentWorkspace(){
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_current_desktop_unchecked(&obj->EWMH, QX11Info::appScreen());
  uint32_t num = 0;
  if(1==xcb_ewmh_get_current_desktop_reply(&obj->EWMH, cookie, &num, NULL) ){
    return num;
  }else{
    return 0;
  }
}

//NativeWindowEventFilter interactions
void NativeWindowSystem::NewWindowDetected(WId id){
  //Make sure this can be managed first
  if(findWindow(id, false) != 0){ findWindow(id,false)->setProperty(NativeWindowObject::Visible, true, true); return; } //already managed
  xcb_get_window_attributes_cookie_t cookie = xcb_get_window_attributes(QX11Info::connection(), id);
  xcb_get_window_attributes_reply_t *attr = xcb_get_window_attributes_reply(QX11Info::connection(), cookie, NULL);
  if(attr == 0){ return; } //could not get attributes of window
  if(attr->override_redirect){ free(attr); return; } //window has override redirect set (do not manage)
  free(attr);
  //xcb_reparent_window(QX11Info::connection(), id, QX11Info::appRootWindow(), 0, 0);
  //Now go ahead and create/populate the container for this window
  NativeWindowObject *win = new NativeWindowObject(id);

  //Register for events from this window
  //registerClientEvents(win->id());
  NWindows << win;
  UpdateWindowProperties(win, NativeWindowObject::allProperties());
  if(win->showWindowFrame()){
    win->setProperty(NativeWindowObject::FrameExtents, QVariant::fromValue<QList<int> >( QList<int>() << 5 << 5 << 30 << 5 ));
  }
  qDebug() << "New Window [& associated ID's]:" << win->id()  << win->property(NativeWindowObject::Name).toString();
  //Now setup the connections with this window
  connect(win, SIGNAL(RequestClose(WId)), this, SLOT(RequestClose(WId)) );
  connect(win, SIGNAL(RequestKill(WId)), this, SLOT(RequestKill(WId)) );
  connect(win, SIGNAL(RequestPing(WId)), this, SLOT(RequestPing(WId)) );
  connect(win, SIGNAL(RequestReparent(WId, WId, QPoint)), this, SLOT(RequestReparent(WId, WId, QPoint)) );
  connect(win, SIGNAL(RequestPropertiesChange(WId, QList<NativeWindowObject::Property>, QList<QVariant>)), this, SLOT(RequestPropertiesChange(WId, QList<NativeWindowObject::Property>, QList<QVariant>)) );
  connect(win, SIGNAL(WindowClosed(WId)), this, SLOT(WindowCloseDetected(WId)) );
  //connect(win, SIGNAL(VerifyNewGeometry(WId)), this, SLOT(CheckWindowPosition(WId)) );
  qDebug() << " - Setup New Window";
  SetupNewWindow(win);
  qDebug() << " - Setup window position";
  CheckWindowPosition(win, true); //first time placement
  //xcb_map_window(QX11Info::connection(), win->id());
  qDebug() << " - Emit new window signal";
  emit NewWindowAvailable(win);
  connect(win, SIGNAL(VerifyNewGeometry(WId)), this, SLOT(CheckWindowPosition(WId)) );
}

void NativeWindowSystem::NewTrayWindowDetected(WId id){
  //Make sure this can be managed first
  qDebug() << "New Tray Window Detected:" << id;
  if(findTrayWindow(id) != 0){ return; } //already managed
  qDebug() << " - Setup new tray window";
  /*xcb_get_window_attributes_cookie_t cookie = xcb_get_window_attributes(QX11Info::connection(), id);
  xcb_get_window_attributes_reply_t *attr = xcb_get_window_attributes_reply(QX11Info::connection(), cookie, NULL);
  if(attr == 0){ qDebug() << " - [WARN] No tray window attributes"; return; } //could not get attributes of window
  if(attr->override_redirect){ qDebug() << " - [WARN] Override Redirect"; free(attr); return; } //window has override redirect set (do not manage)
  qDebug() << " - free attr";
  free(attr);*/
  //Register for events from this window
  #define TRAY_WIN_EVENT_MASK (XCB_EVENT_MASK_EXPOSURE |		\
                          XCB_EVENT_MASK_STRUCTURE_NOTIFY |	\
                          XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |	\
                          XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |	\
                          XCB_EVENT_MASK_ENTER_WINDOW)

  uint32_t value_list[1] = {TRAY_WIN_EVENT_MASK};
  qDebug() << " - change tray window attributes";
  xcb_change_window_attributes(QX11Info::connection(), id, XCB_CW_EVENT_MASK, value_list);
  //Now go ahead and create/populate the container for this window
  NativeWindowObject *win = new NativeWindowObject(id);
  TWindows << win;
  qDebug() << " - update tray window properties";
  UpdateWindowProperties(win, NativeWindowObject::allProperties());
  qDebug() << " - emit new tray window signal";
  emit NewTrayWindowAvailable(win);
}

void NativeWindowSystem::WindowCloseDetected(WId id){
  NativeWindowObject *win = findWindow(id, true);
  //if(win==0){ win = findWindow(id, true); }
  //qDebug() << "Got Window Closed" << id << win;
  //qDebug() << "Old Window List:" << NWindows.length();
  bool ok = false;
  if(win!=0){
    NWindows.removeAll(win);
    win->emit WindowClosed(id);
    //qDebug() << "Visible Window Closed!!!";
    emit WindowClosed();
    win->deleteLater();
    ok = true;
  }else{
    win = findTrayWindow(id);
    if(win!=0){
      TWindows.removeAll(win);
      win->emit WindowClosed(id);
      emit TrayWindowClosed();
      win->deleteLater();
      ok = true;
    }
  }
  if(!ok && !NWindows.isEmpty() && !TWindows.isEmpty() ){ QTimer::singleShot(0, this, SLOT(verifyWindowExistance()) ); }
  //qDebug() << " - Now:" << NWindows.length();
}

void NativeWindowSystem::WindowPropertyChanged(WId id, NativeWindowObject::Property prop){
  //NOTE: This is triggered by the NativeEventFilter - not by changes to the NativeWindow objects themselves
  NativeWindowObject *win = findWindow(id, prop!=NativeWindowObject::Visible);
  if(win==0){ win = findTrayWindow(id); }
  if(win!=0){
    UpdateWindowProperties(win, QList<NativeWindowObject::Property>() << prop);
  }else if(prop != 0){
    //Could not find the window for a specific property with an undefined value
    //  - update this property for all the windows just in case
    for(int i=0; i<NWindows.length(); i++){
      UpdateWindowProperties( NWindows[i], QList<NativeWindowObject::Property>() << prop);
    }
  }
}

void NativeWindowSystem::WindowPropertiesChanged(WId id, QList<NativeWindowObject::Property> props){
  //NOTE: This is triggered by the NativeEventFilter - not by changes to the NativeWindow objects themselves
  NativeWindowObject *win = findWindow(id);
  if(win==0){ win = findTrayWindow(id); }
  if(win!=0){
    UpdateWindowProperties(win, props);
  }else{
    //Could not find the window for a specific property with an undefined value
    //  - update this property for all the windows just in case
    for(int i=0; i<NWindows.length(); i++){
      UpdateWindowProperties( NWindows[i], props);
    }
  }
}

void NativeWindowSystem::WindowPropertyChanged(WId id, NativeWindowObject::Property prop, QVariant val){
  NativeWindowObject *win = findWindow(id,prop!=NativeWindowObject::Visible);
  if(win==0){ win = findTrayWindow(id); }
  if(win!=0){
    win->setProperty(prop, val);
  }
}

void NativeWindowSystem::WindowPropertiesChanged(WId id, QList<NativeWindowObject::Property> props, QList<QVariant> vals){
  NativeWindowObject *win = findWindow(id);
  if(win==0){ win = findTrayWindow(id); }
  if(win!=0){
    for(int i=0; i<props.length() && i<vals.length(); i++){ win->setProperty(props[i], vals[i]); }
  }
}

void NativeWindowSystem::RequestPropertyChange(WId id, NativeWindowObject::Property prop, QVariant val){
  //This is just a simplified version of the multiple-property function
  RequestPropertiesChange(id, QList<NativeWindowObject::Property>() << prop, QList<QVariant>() << val);
}

void NativeWindowSystem::RequestPropertiesChange(WId win, QList<NativeWindowObject::Property> props, QList<QVariant> vals){
  //Find the window object associated with this id
  bool istraywin = false; //just in case we care later if it is a tray window or a regular window
  NativeWindowObject *WIN = findWindow(win);
  if(WIN==0){ istraywin = true; WIN = findTrayWindow(win); }
  if(WIN==0){ return; } //invalid window ID - no longer available
  //Now make any changes as needed
  ChangeWindowProperties(WIN, props, vals);
}

void NativeWindowSystem::GotPong(WId id){
  if(waitingForPong.contains(id)){
    waitingForPong.remove(id);
  }
  if(waitingForPong.isEmpty() && pingTimer!=0){ pingTimer->stop(); }
}

void NativeWindowSystem::NewKeyPress(int keycode, WId win){
  emit NewInputEvent();
  if(screenLocked){ return; }
  Qt::Key key = KeycodeToQt(keycode);
  if(key!=Qt::Key_unknown){ emit KeyPressDetected(win, key); }
}

void NativeWindowSystem::NewKeyRelease(int keycode, WId win){
  emit NewInputEvent();
  if(screenLocked){ return; }
  Qt::Key key = KeycodeToQt(keycode);
  if(key!=Qt::Key_unknown){ emit KeyReleaseDetected(win, key); }
}

void NativeWindowSystem::NewMousePress(int buttoncode, WId win){
  emit NewInputEvent();
  if(screenLocked){ return; }
  emit MousePressDetected(win, MouseToQt(buttoncode));
}

void NativeWindowSystem::NewMouseRelease(int buttoncode, WId win){
  emit NewInputEvent();
  if(screenLocked){ return; }
  emit MouseReleaseDetected(win, MouseToQt(buttoncode));
}

void NativeWindowSystem::CheckDamageID(WId win){
  //qDebug() << "Got Damage Event:" << win;
  for(int i=0; i<NWindows.length(); i++){
    if(NWindows[i]->damageId() == win || NWindows[i]->id() == win || NWindows[i]->frameId()==win){
      //qDebug() << " - Found window";
      //UpdateWindowImage(NWindows[i]);
      NWindows[i]->emit winImageChanged();
      return;
    }
  }
  NativeWindowObject *WIN = findTrayWindow(win);
  if(WIN!=0){
    UpdateWindowProperties(WIN, QList<NativeWindowObject::Property>() << NativeWindowObject::Icon);
  }
}

void NativeWindowSystem::raiseWindow(NativeWindowObject *win){
  qDebug() << "Raise Window:" << win->name() << win->id();
  //Note: Always ensure the desktop canvas is right under the main window that is raised
  //xcb_circulate_window(QX11Info::connection(), XCB_CIRCULATE_RAISE_LOWEST, Lumina::ROOTWIN->viewID());
  if(win->frameId()!=0){ xcb_circulate_window(QX11Info::connection(), XCB_CIRCULATE_RAISE_LOWEST ,win->frameId()); }
  xcb_circulate_window(QX11Info::connection(), XCB_CIRCULATE_RAISE_LOWEST ,win->id());
}


void NativeWindowSystem::lowerWindow(NativeWindowObject *win){
  qDebug() << "Lower Window:" << win->name() << win->id();
  xcb_circulate_window(QX11Info::connection(), XCB_CIRCULATE_LOWER_HIGHEST ,win->id());
  if(win->frameId()!=0){ xcb_circulate_window(QX11Info::connection(), XCB_CIRCULATE_LOWER_HIGHEST ,win->frameId()); }
}

// === PRIVATE SLOTS ===
//These are the slots which are built-in and automatically connected when a new NativeWindow is created
void NativeWindowSystem::verifyWindowExistance(){
  //qDebug() << "Verify Window Existance";
  QList<xcb_void_cookie_t> cookies;
  //Generate all the cookies for the probes
  for(int i=0; i<NWindows.length(); i++){
    cookies << xcb_configure_window_checked(QX11Info::connection(), NWindows[i]->id(), 0, 0);
  }
  //Now look at all the replies and send close messages for any bad-window errors
  for(int i=0; i<cookies.length(); i++){
    xcb_generic_error_t *error = xcb_request_check( QX11Info::connection(), cookies[i]);
    if(error!=0){
      //qDebug() << " - Got missing window" << NWindows[i]->name();
      NWindows[i]->announceClosed();
    }
  }
  //qDebug() << " - End Verify";
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

void NativeWindowSystem::RequestPing(WId win){
  waitingForPong.insert(win, QDateTime::currentDateTime().addSecs(5) );
  xcb_ewmh_send_wm_ping(&obj->EWMH, win, XCB_CURRENT_TIME);
  if(pingTimer==0){
    pingTimer = new QTimer(this);
    pingTimer->setInterval(2000); //2seconds
    connect(pingTimer, SIGNAL(timeout()), this, SLOT(checkPings()) );
  }
  pingTimer->start();
}

void NativeWindowSystem::RequestReparent(WId win, WId container, QPoint relorigin){
  NativeWindowObject *WIN = findWindow(win);
  qDebug() << "Got reparent request:" << win;
  if(WIN==0){ return; } //could not find corresponding window structure
  qDebug() << "Reparent Window into container:" << WIN->name();

  //Reparent the window into the container
  xcb_reparent_window(QX11Info::connection(), win, container, relorigin.x(), relorigin.y());
  xcb_map_window(QX11Info::connection(), win);
  return;
  //Now send the embed event to the app
  //qDebug() << " - send _XEMBED event";
  xcb_client_message_event_t event;
    event.response_type = XCB_CLIENT_MESSAGE;
    event.format = 32;
    event.window = win;
    event.type = obj->ATOMS["_XEMBED"]; //_XEMBED
    event.data.data32[0] = XCB_TIME_CURRENT_TIME; //CurrentTime;
    event.data.data32[1] = 0; //XEMBED_EMBEDDED_NOTIFY
    event.data.data32[2] = 0;
    event.data.data32[3] = container; //WID of the container
    event.data.data32[4] = 0;

    xcb_send_event(QX11Info::connection(), 0, win,  XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT, (const char *) &event);

  //Now setup any redirects and return
  //this->SelectInput(win, true); //Notify of structure changes
  //registerClientEvents(win);
  //xcb_composite_redirect_window(QX11Info::connection(), win, XCB_COMPOSITE_REDIRECT_MANUAL); //XCB_COMPOSITE_REDIRECT_[MANUAL/AUTOMATIC]);

  //Now map the window (will be a transparent child of the container)
  //xcb_map_window(QX11Info::connection(), win);
  //xcb_map_window(QX11Info::connection(), container);
  //Now create/register the damage handler
  // -- XCB (Note: The XCB damage registration is completely broken at the moment - 9/15/15, Ken Moore)
  //  -- Retested 6/29/17 (no change) Ken Moore
  //xcb_damage_damage_t dmgID = xcb_generate_id(QX11Info::connection()); //This is a typedef for a 32-bit unsigned integer
  //xcb_damage_create(QX11Info::connection(), dmgID, win, XCB_DAMAGE_REPORT_LEVEL_RAW_RECTANGLES);
  // -- XLib (Note: This is only used because the XCB routine above does not work - needs to be fixed upstream in XCB itself).
  //Damage dmgID = XDamageCreate(QX11Info::display(), win, XDamageReportRawRectangles);
  //WIN->addDamageID( (uint) dmgID); //save this for later
  //qDebug() << " - Done";
  //return ( (uint) dmgID );
}
/*
  xcb_reparent_window(QX11Info::connection(), client, parent, relorigin.x(), relorigin.y());

  //Now ensure that we still get event for these windows
  registerClientEvents(client); //make sure we re-do this after reparenting
  registerClientEvents(parent);
  xcb_map_window(QX11Info::connection(), parent);
}*/
