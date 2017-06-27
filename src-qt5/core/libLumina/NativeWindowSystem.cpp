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
#include <QApplication>
#include <QScreen>

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
	  xcb_intern_atom_cookie_t *cookie = xcb_ewmh_init_atoms(QX11Info::connection(), &EWMH);
	   if(!xcb_ewmh_init_atoms_replies(&EWMH, cookie, NULL) ){
	     qDebug() << "Error with XCB atom initializations";
	     return false;
	   }

	  QStringList atoms;
	    atoms << "WM_TAKE_FOCUS" << "WM_DELETE_WINDOW" << "WM_PROTOCOLS"
		<< "WM_CHANGE_STATE" << "_NET_SYSTEM_TRAY_OPCODE" << "_NET_SYSTEM_TRAY_ORIENTATION"
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

//Small simplification functions
Qt::Key NativeWindowSystem::KeycodeToQt(int keycode){
  qDebug() << "Try to convert keycode to Qt::Key:" << keycode;
  qDebug() << " - Not implemented yet";
  return Qt::Key_unknown;
}

NativeWindowSystem::MouseButton NativeWindowSystem::MouseToQt(int keycode){
  switch(keycode){
    case 1:
      return NativeWindowSystem::LeftButton;
    case 3:
      return NativeWindowSystem::RightButton;
    case 2:
      return NativeWindowSystem::MidButton;
    case 4:
      return NativeWindowSystem::WheelUp;
    case 5:
      return NativeWindowSystem::WheelDown;
    case 6:
      return NativeWindowSystem::WheelLeft;
    case 7:
      return NativeWindowSystem::WheelRight;
    case 8:
      return NativeWindowSystem::BackButton; //Not sure if this is correct yet (1/27/17)
    case 9:
      return NativeWindowSystem::ForwardButton; //Not sure if this is correct yet (1/27/17)
    default:
      return NativeWindowSystem::NoButton;
  }
}

// === PRIVATE ===
void NativeWindowSystem::UpdateWindowProperties(NativeWindow* win, QList< NativeWindow::Property > props){
  //Put the properties in logical groups as appropriate (some XCB calls return multiple properties)
  if(props.contains(NativeWindow::Title)){
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
    win->setProperty(NativeWindow::Name, name);
  } //end TITLE property

  if(props.contains(NativeWindow::ShortTitle)){
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
    win->setProperty(NativeWindow::ShortTitle, name);
  } //end SHORTTITLE property

  if(props.contains(NativeWindow::Icon)){
    //See if this is a tray icon first (different routine - entire app window is the icon)
    QIcon icon;
    if(win == findTrayWindow(win->id())){
      //Tray Icon Window
      QPixmap pix;
      //Get the current QScreen (for XCB->Qt conversion)
      QList<QScreen*> scrnlist = QApplication::screens();
      //Try to grab the given window directly with Qt
      for(int i=0; i<scrnlist.length() && pix.isNull(); i++){
        pix = scrnlist[i]->grabWindow(win->id());
      }
      icon.addPixmap(pix);
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
    win->setProperty(NativeWindow::Icon, icon);
  } //end ICON property

  if(props.contains(NativeWindow::MinSize) || props.contains(NativeWindow::MaxSize)
	|| props.contains(NativeWindow::Size) || props.contains(NativeWindow::GlobalPos) ){
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
      bool initsize = win->property(NativeWindow::Size).isNull(); //initial window size
      if( (reply.flags&XCB_ICCCM_SIZE_HINT_US_POSITION)==XCB_ICCCM_SIZE_HINT_US_POSITION ){ win->setProperty(NativeWindow::GlobalPos, QPoint(reply.x,reply.y)); }
      if( (reply.flags&XCB_ICCCM_SIZE_HINT_US_SIZE)==XCB_ICCCM_SIZE_HINT_US_SIZE ){ win->setProperty(NativeWindow::Size, QSize(reply.width, reply.height)); }
      if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_POSITION)==XCB_ICCCM_SIZE_HINT_P_POSITION ){ win->setProperty(NativeWindow::GlobalPos, QPoint(reply.x,reply.y)); }
      if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_SIZE)==XCB_ICCCM_SIZE_HINT_P_SIZE ){ win->setProperty(NativeWindow::Size, QSize(reply.width, reply.height)); }
      if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_MIN_SIZE)==XCB_ICCCM_SIZE_HINT_P_MIN_SIZE ){ win->setProperty(NativeWindow::MinSize, QSize(reply.min_width, reply.min_height)); }
      if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_MAX_SIZE)==XCB_ICCCM_SIZE_HINT_P_MAX_SIZE ){ win->setProperty(NativeWindow::MaxSize, QSize(reply.max_width, reply.max_height)); }
      if( (reply.flags&XCB_ICCCM_SIZE_HINT_BASE_SIZE)==XCB_ICCCM_SIZE_HINT_BASE_SIZE && initsize ){ win->setProperty(NativeWindow::Size, QSize(reply.base_width, reply.base_height)); }
      //if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_RESIZE_INC)==XCB_ICCCM_SIZE_HINT_P_RESIZE_INC ){ hints.width_inc=reply.width_inc; hints.height_inc=reply.height_inc; }
      //if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_ASPECT)==XCB_ICCCM_SIZE_HINT_P_ASPECT ){ hints.min_aspect_num=reply.min_aspect_num; hints.min_aspect_den=reply.min_aspect_den; hints.max_aspect_num=reply.max_aspect_num; hints.max_aspect_den=reply.max_aspect_den;}
      //if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_WIN_GRAVITY)==XCB_ICCCM_SIZE_HINT_P_WIN_GRAVITY ){ hints.win_gravity=reply.win_gravity; }
    }
  } //end of geometry properties

  if(props.contains(NativeWindow::Name)){
    //Put the app/class name here (much more static than the "Title" properties
    xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_class_unchecked(QX11Info::connection(), win->id());
    xcb_icccm_get_wm_class_reply_t reply;
    if(1 == xcb_icccm_get_wm_class_reply(QX11Info::connection(), cookie, &reply, NULL) ){
      //Returns: "<instance name>::::<class name>"
      win->setProperty(NativeWindow::Name, ( QString::fromLocal8Bit(reply.instance_name)+"::::"+QString::fromLocal8Bit(reply.class_name) ));
      xcb_icccm_get_wm_class_reply_wipe(&reply);
    }
  } //end NAME property

  if(props.contains(NativeWindow::Workspace)){
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
    win->setProperty(NativeWindow::Workspace, wkspace);
  }
}


// === PUBLIC SLOTS ===
//These are the slots which are typically only used by the desktop system itself or the NativeEventFilter
void NativeWindowSystem::RegisterVirtualRoot(WId id){
  //Convert to XCB array
  xcb_window_t array[1];
  array[0] = id;
  //Set the property
  xcb_ewmh_set_virtual_roots(&obj->EWMH, QX11Info::appScreen(), 1, array);
}

void NativeWindowSystem::setRoot_supportedActions(){
//NET_WM standards (ICCCM implied - no standard way to list those)
  xcb_atom_t list[] = {obj->EWMH._NET_WM_NAME,
		obj->EWMH._NET_WM_ICON,
		obj->EWMH._NET_WM_ICON_NAME,
		obj->EWMH._NET_WM_DESKTOP,
		/*_NET_WINDOW_TYPE (and all the various types)*/
		obj->EWMH._NET_WM_WINDOW_TYPE, obj->EWMH._NET_WM_WINDOW_TYPE_DESKTOP, obj->EWMH._NET_WM_WINDOW_TYPE_DOCK,
		obj->EWMH._NET_WM_WINDOW_TYPE_TOOLBAR, obj->EWMH._NET_WM_WINDOW_TYPE_MENU, obj->EWMH._NET_WM_WINDOW_TYPE_UTILITY,
		obj->EWMH._NET_WM_WINDOW_TYPE_SPLASH, obj->EWMH._NET_WM_WINDOW_TYPE_DIALOG, obj->EWMH._NET_WM_WINDOW_TYPE_NORMAL,
		obj->EWMH._NET_WM_WINDOW_TYPE_DROPDOWN_MENU, obj->EWMH._NET_WM_WINDOW_TYPE_POPUP_MENU, obj->EWMH._NET_WM_WINDOW_TYPE_TOOLTIP,
		obj->EWMH._NET_WM_WINDOW_TYPE_NOTIFICATION, obj->EWMH._NET_WM_WINDOW_TYPE_COMBO, obj->EWMH._NET_WM_WINDOW_TYPE_DND,
		};
  xcb_ewmh_set_supported(&obj->EWMH, QX11Info::appScreen(), 19,list);
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
  xcb_ewmh_set_active_window(&obj->EWMH, QX11Info::appScreen(), win);
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
    xcb_flush(QX11Info::connection());
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
  if(findWindow(id) != 0){ return; } //already managed
  xcb_get_window_attributes_cookie_t cookie = xcb_get_window_attributes(QX11Info::connection(), id);
  xcb_get_window_attributes_reply_t *attr = xcb_get_window_attributes_reply(QX11Info::connection(), cookie, NULL);
  if(attr == 0){ return; } //could not get attributes of window
  if(attr->override_redirect){ free(attr); return; } //window has override redirect set (do not manage)
  free(attr);
  //Register for events from this window
  #define NORMAL_WIN_EVENT_MASK (XCB_EVENT_MASK_BUTTON_PRESS | 	\
                          XCB_EVENT_MASK_BUTTON_RELEASE | 	\
                          XCB_EVENT_MASK_POINTER_MOTION |	\
			  XCB_EVENT_MASK_BUTTON_MOTION |	\
                          XCB_EVENT_MASK_EXPOSURE |		\
                          XCB_EVENT_MASK_STRUCTURE_NOTIFY |	\
                          XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |	\
                          XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |	\
                          XCB_EVENT_MASK_ENTER_WINDOW)

  uint32_t value_list[1] = {NORMAL_WIN_EVENT_MASK};
  xcb_change_window_attributes(QX11Info::connection(), id, XCB_CW_EVENT_MASK, value_list);
  //Now go ahead and create/populate the container for this window
  NativeWindow *win = new NativeWindow(id);
  NWindows << win;
  UpdateWindowProperties(win, NativeWindow::allProperties());
  emit NewWindowAvailable(win);
}

void NativeWindowSystem::NewTrayWindowDetected(WId id){
  //Make sure this can be managed first
  if(findTrayWindow(id) != 0){ return; } //already managed
  xcb_get_window_attributes_cookie_t cookie = xcb_get_window_attributes(QX11Info::connection(), id);
  xcb_get_window_attributes_reply_t *attr = xcb_get_window_attributes_reply(QX11Info::connection(), cookie, NULL);
  if(attr == 0){ return; } //could not get attributes of window
  if(attr->override_redirect){ free(attr); return; } //window has override redirect set (do not manage)
  free(attr);
  //Register for events from this window
  #define TRAY_WIN_EVENT_MASK (XCB_EVENT_MASK_BUTTON_PRESS | 	\
                          XCB_EVENT_MASK_BUTTON_RELEASE | 	\
                          XCB_EVENT_MASK_POINTER_MOTION |	\
			  XCB_EVENT_MASK_BUTTON_MOTION |	\
                          XCB_EVENT_MASK_EXPOSURE |		\
                          XCB_EVENT_MASK_STRUCTURE_NOTIFY |	\
                          XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |	\
                          XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |	\
                          XCB_EVENT_MASK_ENTER_WINDOW)

  uint32_t value_list[1] = {TRAY_WIN_EVENT_MASK};
  xcb_change_window_attributes(QX11Info::connection(), id, XCB_CW_EVENT_MASK, value_list);
  //Now go ahead and create/populate the container for this window
  NativeWindow *win = new NativeWindow(id);
  TWindows << win;
  UpdateWindowProperties(win, NativeWindow::allProperties());
  emit NewTrayWindowAvailable(win);
}

void NativeWindowSystem::WindowCloseDetected(WId id){
  NativeWindow *win = findWindow(id);
  qDebug() << "Got Window Closed" << id << win;
  if(win!=0){
    NWindows.removeAll(win);
    win->emit WindowClosed(id);
    win->deleteLater();
  }else{
    win = findTrayWindow(id);
    if(win!=0){
      TWindows.removeAll(win);
      win->emit WindowClosed(id);
      win->deleteLater();
    }
  }
}

void NativeWindowSystem::WindowPropertyChanged(WId id, NativeWindow::Property prop){
  //NOTE: This is triggered by the NativeEventFilter - not by changes to the NativeWindow objects themselves
  NativeWindow *win = findWindow(id);
  if(win==0){ win = findTrayWindow(id); }
  if(win!=0){
    UpdateWindowProperties(win, QList<NativeWindow::Property>() << prop);
  }
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
  emit KeyPressDetected(win, keycode);
}

void NativeWindowSystem::NewKeyRelease(int keycode, WId win){
  emit NewInputEvent();
  if(screenLocked){ return; }
  emit KeyReleaseDetected(win, keycode);
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
  NativeWindow *WIN = findTrayWindow(win);
  if(WIN!=0){
    UpdateWindowProperties(WIN, QList<NativeWindow::Property>() << NativeWindow::Icon);
  }
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
  // TODO
  qDebug() << "Request Properties Changed:" << props << vals;
  qDebug() << " - Not implemented yet";
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
