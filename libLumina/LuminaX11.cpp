//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LuminaX11.h"

#include <QString>
#include <QByteArray>
#include <QFile>
#include <QObject>
#include <QImage>
#include <QApplication>
#include <QDesktopWidget>
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

//XLib includes
#include <X11/extensions/Xdamage.h>

#define DEBUG 0

//===============================
//===============================
// XCB LIBRARY FUNCTIONS
//===============================
//===============================
LXCB::LXCB(){
   xcb_intern_atom_cookie_t *cookie = xcb_ewmh_init_atoms(QX11Info::connection(), &EWMH);
   if(!xcb_ewmh_init_atoms_replies(&EWMH, cookie, NULL) ){
     qDebug() << "Error with XCB atom initializations";
   }else{
     qDebug() << "Number of XCB screens:" << EWMH.nb_screens;
   }
}
LXCB::~LXCB(){
  xcb_ewmh_connection_wipe(&EWMH);
}

// === WindowList() ===
QList<WId> LXCB::WindowList(bool rawlist){
  if(DEBUG){ qDebug() << "XCB: WindowList()" << rawlist; }
  QList<WId> output;
  //qDebug() << "Get Client list cookie";
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_client_list_unchecked( &EWMH, 0);
  xcb_ewmh_get_windows_reply_t winlist;
  //qDebug() << "Get client list";
  if( 1 == xcb_ewmh_get_client_list_reply( &EWMH, cookie, &winlist, NULL) ){
    //qDebug() << " - Loop over items";
    unsigned int wkspace = CurrentWorkspace();
    for(unsigned int i=0; i<winlist.windows_len; i++){ 
      //Filter out the Lumina Desktop windows
      if(WindowClass(winlist.windows[i]) == "Lumina Desktop Environment"){ continue; }
      //Also filter out windows not on the active workspace
      else if( (WindowWorkspace(winlist.windows[i])!=wkspace) && !rawlist ){ continue; }
      else{
        output << winlist.windows[i]; 
      }
    }
  }
  return output;
}

// === CurrentWorkspace() ===
unsigned int LXCB::CurrentWorkspace(){
  if(DEBUG){ qDebug() << "XCB: CurrentWorkspace()"; }
  //qDebug() << "Get Current Workspace";
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_current_desktop_unchecked(&EWMH, 0);
  uint32_t wkspace = 0;
  xcb_ewmh_get_current_desktop_reply(&EWMH, cookie, &wkspace, NULL);
  //qDebug() << " - done:" << wkspace;
  return wkspace;
}

unsigned int LXCB::NumberOfWorkspaces(){
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_number_of_desktops_unchecked(&EWMH, 0);
  uint32_t number;
  if(1==xcb_ewmh_get_number_of_desktops_reply(&EWMH, cookie, &number, NULL) ){
    return number;
  }else{
    return 0; //unable to get this property
  }
}

// === ActiveWindow() ===
WId LXCB::ActiveWindow(){
  if(DEBUG){ qDebug() << "XCB: ActiveWindow()"; }
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_active_window_unchecked(&EWMH, 0);
  xcb_window_t actwin;
  if(1 == xcb_ewmh_get_active_window_reply(&EWMH, cookie, &actwin, NULL) ){
    return actwin;
  }else{
    return 0; //invalid ID/failure
  }
}

// === CheckDisableXinerama() ===
bool LXCB::CheckDisableXinerama(){ 
  //returns true if Xinerama was initially set but now disabled
  return false;
  // TO-DO - not complete yet

  /*xcb_query_extension_cookie_t cookie = xcb_query_extension_unchecked(QX11Info::connection(), 8, "Xinerama");
  xcb_query_extension_reply_t *reply = xcb_query_extension_reply(QX11Info::connection(), cookie, NULL);
	
  if(reply!=0){
  
    free(reply);
  }
  */
}

// === RegisterVirtualRoots() ===
void LXCB::RegisterVirtualRoots(QList<WId> roots){
  if(DEBUG){ qDebug() << "XCB: RegisterVirtualRoots()"; }
  //First convert the QList into the proper format
  xcb_window_t *list = new xcb_window_t[ roots.length() ];
  for(int i=0; i<roots.length(); i++){
    list[i] = roots[i]; //move from the QList to the array
  }
  //Now set the property
  xcb_ewmh_set_virtual_roots(&EWMH, 0, roots.length(), list);
  //Now delete the temporary array from memory
  delete list;
}

// ===== SetCurrentWorkspace() =====
void LXCB::SetCurrentWorkspace(int number){
  //Need to send a client message event for the window so the WM picks it up
  xcb_client_message_event_t event;
    event.response_type = XCB_CLIENT_MESSAGE;
    event.format = 32;
    event.window = QX11Info::appRootWindow();
    event.type = EWMH._NET_CURRENT_DESKTOP;
    event.data.data32[0] = number; //set to enabled
    event.data.data32[1] = XCB_TIME_CURRENT_TIME;
    event.data.data32[2] = 0;
    event.data.data32[3] = 0;
    event.data.data32[4] = 0;

  xcb_send_event(QX11Info::connection(), 0, QX11Info::appRootWindow(),  XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT, (const char *) &event);
	
  //EWMH function (does not seem to be recognized by Fluxbox)
  xcb_ewmh_request_change_showing_desktop(&EWMH, QX11Info::appScreen(), number);
}

// === WindowClass() ===
QString LXCB::WindowClass(WId win){
  if(DEBUG){ qDebug() << "XCB: WindowClass()" << win; }
  QString out;
  if(win==0){ return ""; }
  xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_class_unchecked(QX11Info::connection(), win);
  if(cookie.sequence == 0){ return out; } 
  xcb_icccm_get_wm_class_reply_t value;
  if( 1== xcb_icccm_get_wm_class_reply( QX11Info::connection(), cookie, &value, NULL) ){
    out = QString::fromUtf8(value.class_name);
    xcb_icccm_get_wm_class_reply_wipe(&value);
  }
  return out;
}

// === WindowWorkspace() ===
unsigned int LXCB::WindowWorkspace(WId win){
  if(DEBUG){ qDebug() << "XCB: WindowWorkspace()" << win; }
  //qDebug() << "Get Window Workspace";
  if(win==0){ return 0; }
  uint32_t wkspace = 0;
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_desktop_unchecked(&EWMH, win);
  if(cookie.sequence == 0){ return wkspace; } 
  xcb_ewmh_get_wm_desktop_reply(&EWMH, cookie, &wkspace, NULL);
  //qDebug() << " - done: " << wkspace;
  return wkspace;	
}

// === WindowGeometry() ===
QRect LXCB::WindowGeometry(WId win, bool includeFrame){
  if(DEBUG){ qDebug() << "XCB: WindowGeometry()"; }
  QRect geom;
  if(win==0){ return geom; }
  xcb_get_geometry_cookie_t cookie = xcb_get_geometry(QX11Info::connection(), win);
  xcb_get_geometry_reply_t *reply = xcb_get_geometry_reply(QX11Info::connection(), cookie, NULL);
  //qDebug() << "Get Window Geometry:" << reply;
  if(reply != 0){
    geom = QRect(0, 0, reply->width, reply->height); //make sure to use the origin point for the window
    //qDebug() << " - "<<reply->x << reply->y << reply->width << reply->height;
    free(reply);
    if(includeFrame){
      //Need to add/include the frame extents as well (assuming the frame info is available)
      xcb_get_property_cookie_t cookie = xcb_ewmh_get_frame_extents_unchecked(&EWMH, win);
      if(cookie.sequence != 0){
	xcb_ewmh_get_extents_reply_t frame;
        if(1== xcb_ewmh_get_frame_extents_reply(&EWMH, cookie, &frame, NULL) ){
	    //adjust the origin point to account for the frame
	    geom.translate(-frame.left, -frame.top); //move to the orign point for the frame
	    //adjust the size (include the frame sizes)
	    geom.setWidth( geom.width() + frame.left + frame.right );
	    geom.setHeight( geom.height() + frame.top + frame.bottom );
	}
	//qDebug() << " - Frame:" << frame.left << frame.right << frame.top << frame.bottom;
	//qDebug() << " - Modified with Frame:" << geom.x() << geom.y() << geom.width() << geom.height();
      }
    }
    //Now need to convert this to absolute coordinates (not parent-relavitve)
      xcb_translate_coordinates_cookie_t tcookie = xcb_translate_coordinates(QX11Info::connection(), win, QX11Info::appRootWindow(), geom.x(), geom.y());
      xcb_translate_coordinates_reply_t *trans = xcb_translate_coordinates_reply(QX11Info::connection(), tcookie, NULL);
      if(trans!=0){
	//qDebug() << " - Got Translation:" << trans->dst_x << trans->dst_y;
	//Replace the origin point with the global position (sizing remains the same)
        geom.moveLeft(trans->dst_x); //adjust X coordinate (no size change)
	geom.moveTop(trans->dst_y); //adjust Y coordinate (no size change)
	free(trans);
      }
  }else{
    //Need to do another catch for this situation (probably not mapped yet)
  }
  return geom;
}

// === WindowFrameGeometry() ===
QList<int> LXCB::WindowFrameGeometry(WId win){
  if(DEBUG){ qDebug() << "XCB: WindowFrameGeometry()"; }
  //Returns: [top, bottom, left, right] sizes for the frame
  QList<int> geom;
  if(win!=0){
    xcb_get_property_cookie_t cookie = xcb_ewmh_get_frame_extents_unchecked(&EWMH, win);
    if(cookie.sequence != 0){
      xcb_ewmh_get_extents_reply_t frame;
      if(1== xcb_ewmh_get_frame_extents_reply(&EWMH, cookie, &frame, NULL) ){
        //adjust the origin point to account for the frame
        geom << frame.top << frame.bottom << frame.left << frame.right;
      }
    }
  }
  if(geom.isEmpty()){ geom << 0 << 0 << 0 << 0; }
  return geom;
}

// === WindowState() ===
LXCB::WINDOWSTATE LXCB::WindowState(WId win){
  if(DEBUG){ qDebug() << "XCB: WindowState()"; }
  if(win==0){ return IGNORE; }
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_state_unchecked(&EWMH, win);
  if(cookie.sequence == 0){ return IGNORE; } 
  xcb_ewmh_get_atoms_reply_t states;
  WINDOWSTATE cstate = IGNORE;
  //First Check for special states (ATTENTION in particular);
  if( 1 == xcb_ewmh_get_wm_state_reply(&EWMH, cookie, &states, NULL) ){
    for(unsigned int i=0; i<states.atoms_len; i++){
      if(states.atoms[i] == EWMH._NET_WM_STATE_DEMANDS_ATTENTION){ cstate = ATTENTION; break; } //nothing more urgent - stop here
      else if(states.atoms[i] == EWMH._NET_WM_STATE_HIDDEN){ cstate = INVISIBLE; }
    }
  }
  //Now check to see if the window is the active one
  if(cstate == IGNORE){
    xcb_get_property_cookie_t cookie = xcb_ewmh_get_active_window_unchecked(&EWMH, 0);
    xcb_window_t actwin;
    if(1 == xcb_ewmh_get_active_window_reply(&EWMH, cookie, &actwin, NULL) ){
      if(actwin == win){ cstate = ACTIVE; }
    }
  }
  //Now check for ICCCM Urgency hint (not sure if this is still valid with EWMH instead)
  /*if(cstate == IGNORE){
    xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_hints_unchecked(QX11Info::connection(), win);
    xcb_icccm_wm_hints_t hints;
    if( 1== xcb_icccm_get_wm_hints_reply(QX11Info::connection(), cookie, &hints, NULL) ){
      if(xcb_icccm_wm_hints_get_urgency(hints) ){ cstate = ATTENTION; };
    }
  }*/
  //Now check for standard visible/invisible attribute (current mapping state)
  if(cstate == IGNORE){
    xcb_get_window_attributes_cookie_t cookie = xcb_get_window_attributes(QX11Info::connection(), win);
    xcb_get_window_attributes_reply_t *attr = xcb_get_window_attributes_reply(QX11Info::connection(), cookie, NULL);
    if(attr!=0){
      if(attr->map_state==XCB_MAP_STATE_VIEWABLE){ cstate = VISIBLE; }
      else{ cstate = INVISIBLE; }
      free(attr);	    
    }
  }
  return cstate;
}

// === WindowVisibleIconName() ===
QString LXCB::WindowVisibleIconName(WId win){ //_NET_WM_VISIBLE_ICON_NAME
  if(DEBUG){ qDebug() << "XCB: WindowVisibleIconName()"; }
  if(win==0){ return ""; }
  QString out;
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_visible_icon_name_unchecked(&EWMH, win);
  if(cookie.sequence == 0){ return out; } 
  xcb_ewmh_get_utf8_strings_reply_t data;
  if( 1 == xcb_ewmh_get_wm_visible_icon_name_reply(&EWMH, cookie, &data, NULL) ){
      out = QString::fromUtf8(data.strings, data.strings_len);
  }
  return out;
}

// === WindowIconName() ===
QString LXCB::WindowIconName(WId win){ //_NET_WM_ICON_NAME
  if(DEBUG){ qDebug() << "XCB: WindowIconName()"; }
  if(win==0){ return ""; }
  QString out;
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_icon_name_unchecked(&EWMH, win);
  if(cookie.sequence == 0){ return out; } 
  xcb_ewmh_get_utf8_strings_reply_t data;
  if( 1 == xcb_ewmh_get_wm_icon_name_reply(&EWMH, cookie, &data, NULL) ){
      out = QString::fromUtf8(data.strings, data.strings_len);
  }
  return out;
}

// === WindowVisibleName() ===
QString LXCB::WindowVisibleName(WId win){ //_NET_WM_VISIBLE_NAME
  if(DEBUG){ qDebug() << "XCB: WindowVisibleName()"; }
  if(win==0){ return ""; }
  QString out;
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_visible_name_unchecked(&EWMH, win);
  if(cookie.sequence == 0){ return out; } 
  xcb_ewmh_get_utf8_strings_reply_t data;
  if( 1 == xcb_ewmh_get_wm_visible_name_reply(&EWMH, cookie, &data, NULL) ){
      out = QString::fromUtf8(data.strings, data.strings_len);
  }
  return out;
}

// === WindowName() ===
QString LXCB::WindowName(WId win){ //_NET_WM_NAME
  if(DEBUG){ qDebug() << "XCB: WindowName()"; }
  if(win==0){ return ""; }
  QString out;
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_name_unchecked(&EWMH, win);
  if(cookie.sequence == 0){ return out; } 
  xcb_ewmh_get_utf8_strings_reply_t data;
  if( 1 == xcb_ewmh_get_wm_name_reply(&EWMH, cookie, &data, NULL) ){
      out = QString::fromUtf8(data.strings, data.strings_len);
  }
  return out;
}

// === OldWindowName() ===
QString LXCB::OldWindowName(WId win){ //WM_NAME (old standard)
  if(DEBUG){ qDebug() << "XCB: OldWindowName()"; }
  if(win==0){ return ""; }
  xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_name_unchecked(QX11Info::connection(), win);
  xcb_icccm_get_text_property_reply_t reply;
  if(1 == xcb_icccm_get_wm_name_reply(QX11Info::connection(), cookie, &reply, NULL) ){
    QString name = QString::fromLocal8Bit(reply.name);
    xcb_icccm_get_text_property_reply_wipe(&reply);
    return name;
  }else{
    return "";
  }	
}

// === OldWindowIconName() ===
QString LXCB::OldWindowIconName(WId win){ //WM_ICON_NAME (old standard)
  if(DEBUG){ qDebug() << "XCB: OldWindowIconName()"; }
  if(win==0){ return ""; }
  xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_icon_name_unchecked(QX11Info::connection(), win);
  xcb_icccm_get_text_property_reply_t reply;
  if(1 == xcb_icccm_get_wm_icon_name_reply(QX11Info::connection(), cookie, &reply, NULL) ){
    QString name = QString::fromLocal8Bit(reply.name);
    xcb_icccm_get_text_property_reply_wipe(&reply);
    return name;
  }else{
    return "";
  }	
}

// === WindowIsMaximized() ===
bool LXCB::WindowIsMaximized(WId win){
  if(DEBUG){ qDebug() << "XCB: WindowIsMaximized()"; }
  if(win==0){ return ""; }
  //See if the _NET_WM_STATE_MAXIMIZED_[VERT/HORZ] flags are set on the window
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_state_unchecked(&EWMH, win);
  if(cookie.sequence == 0){ return false; } 
  xcb_ewmh_get_atoms_reply_t states;
  if( 1 == xcb_ewmh_get_wm_state_reply(&EWMH, cookie, &states, NULL) ){
    //Loop over the states
    for(unsigned int i=0; i<states.atoms_len; i++){
      if(states.atoms[i] == EWMH._NET_WM_STATE_MAXIMIZED_HORZ \
	      || states.atoms[i] == EWMH._NET_WM_STATE_MAXIMIZED_VERT ){
	return true;
      }
    }
  }
  return false;
}

// === WindowIsFullscreen() ===
int LXCB::WindowIsFullscreen(WId win){
 if(DEBUG){ qDebug() << "XCB: WindowIsFullscreen()"; }
  if(win==0){ return -1; }	
  //bool fullS = false;
  //See if the _NET_WM_STATE_FULLSCREEN flag is set on the window
  /*xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_state_unchecked(&EWMH, win);
  if(cookie.sequence == 0){ return false; } 
  xcb_ewmh_get_atoms_reply_t states;
  if( 1 == xcb_ewmh_get_wm_state_reply(&EWMH, cookie, &states, NULL) ){
    //Loop over the states
    for(unsigned int i=0; i<states.atoms_len; i++){
      if(states.atoms[i] == EWMH._NET_WM_STATE_FULLSCREEN){
	fullS = true; 
	break;
      }
    }
  }*/
  //if(!fullS){
    //Fallback check for windows which are painted above everything else 
    // but don't have the FULLSCREEN flag set (even though they are technically full-screen)
    int fscreen = -1;
    //qDebug() << "FALLBACK FULLSCREEN CHECK:";
    QRect geom = LXCB::WindowGeometry(win, false);
    QDesktopWidget *desk = QApplication::desktop();
    for(int i=0; i<desk->screenCount(); i++){
      QRect sgeom = desk->screenGeometry(i);
      qDebug() << " -- Check Window Geom:" << sgeom << geom << this->WindowClass(win);
      if( sgeom.contains(geom.center()) ){
	//Allow a 1 pixel variation in "full-screen" detection
	qDebug() << " -- Found Screen:" << i;
	if( geom.width() >= (sgeom.width()-1) && geom.height()>=(sgeom.height()-1) ){
	  qDebug() << " -- Is Fullscreen!";
	  //fullS = true;
	  fscreen = i;
	}
	break; //found the screen which contains this window
      }
    }
  //}
  //return fullS;
  return fscreen;
}

// === WindowIcon() ===
QIcon LXCB::WindowIcon(WId win){
  //Fetch the _NET_WM_ICON for the window and return it as a QIcon
  if(DEBUG){ qDebug() << "XCB: WindowIcon()"; }
  QIcon icon;
  if(win==0){ return icon; }
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_icon_unchecked(&EWMH, win);
  xcb_ewmh_get_wm_icon_reply_t reply;
  if(1 == xcb_ewmh_get_wm_icon_reply(&EWMH, cookie, &reply, NULL)){
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
  return icon;
}

// === SelectInput() ===
void LXCB::SelectInput(WId win){
  uint32_t mask = XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |  XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_PROPERTY_CHANGE;
  xcb_change_window_attributes(QX11Info::connection(), win, XCB_CW_EVENT_MASK, &mask );
}

// === GenerateDamageID() ===
uint LXCB::GenerateDamageID(WId win){
  //Now create/register the damage handler
  xcb_damage_damage_t dmgID = xcb_generate_id(QX11Info::connection()); //This is a typedef for a 32-bit unsigned integer
  xcb_damage_create(QX11Info::connection(), dmgID, win, XCB_DAMAGE_REPORT_LEVEL_RAW_RECTANGLES);
  return ( (uint) dmgID );		
}


// === SetAsSticky() ===
void LXCB::SetAsSticky(WId win){
  if(DEBUG){ qDebug() << "XCB: SetAsSticky()"; }
  if(win==0){ return; }
  //Need to send a client message event for the window so the WM picks it up
  xcb_client_message_event_t event;
    event.response_type = XCB_CLIENT_MESSAGE;
    event.format = 32;
    event.window = win;
    event.type = EWMH._NET_WM_STATE;
    event.data.data32[0] = 1; //set to enabled
    event.data.data32[1] = EWMH._NET_WM_STATE_STICKY;
    event.data.data32[2] = 0;
    event.data.data32[3] = 0;
    event.data.data32[4] = 0;

  xcb_send_event(QX11Info::connection(), 0, QX11Info::appRootWindow(),  XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT, (const char *) &event);
	
  //This method changes the property on the window directly - the WM is not aware of it	
  /*xcb_change_property( QX11Info::connection(), XCB_PROP_MODE_APPEND, win, EWMH._NET_WM_STATE, XCB_ATOM_ATOM, 32, 1, &(EWMH._NET_WM_STATE_STICKY) );
  xcb_flush(QX11Info::connection()); //apply it right away*/
}

// === SetDisableWMActions() ===
void LXCB::SetDisableWMActions(WId win){
  if(DEBUG){ qDebug() << "XCB: SetDisableWMActions()"; }
  //This disables all the various control that a WM allows for the window (except for allowing the "Sticky" state)
  xcb_atom_t list[1];
    list[0] = EWMH._NET_WM_ACTION_STICK;
  xcb_ewmh_set_wm_allowed_actions(&EWMH, win, 1, list);
}

// === SetAsPanel() ===
void LXCB::SetAsPanel(WId win){
  if(DEBUG){ qDebug() << "XCB: SetAsPanel()"; }
  if(win==0){ return; }
  SetDisableWMActions(win); //also need to disable WM actions for this window
  //Disable Input focus (panel activation ruins task manager window detection routines)
  //  - Disable Input flag in WM_HINTS
  xcb_icccm_wm_hints_t hints;
  //qDebug() << " - Disable WM_HINTS input flag";
  xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_hints_unchecked(QX11Info::connection(), win);
  //qDebug() << " -- got cookie";
  if(1 == xcb_icccm_get_wm_hints_reply(QX11Info::connection(), cookie, &hints, NULL) ){
    //qDebug() << " -- Set no inputs flag";
     xcb_icccm_wm_hints_set_input(&hints, false); //set no input focus
     xcb_icccm_set_wm_hints(QX11Info::connection(), win, &hints); //save hints back to window
  }
  //  - Remove WM_TAKE_FOCUS from the WM_PROTOCOLS for the window
  //  - - Generate the necessary atoms
  //qDebug() << " - Generate WM_PROTOCOLS and WM_TAKE_FOCUS atoms";
  xcb_atom_t WM_PROTOCOLS, WM_TAKE_FOCUS; //the two atoms needed
  xcb_intern_atom_reply_t *preply = xcb_intern_atom_reply(QX11Info::connection(), \
			xcb_intern_atom(QX11Info::connection(), 0, 12, "WM_PROTOCOLS"), NULL);
  xcb_intern_atom_reply_t *freply = xcb_intern_atom_reply(QX11Info::connection(), \
			xcb_intern_atom(QX11Info::connection(), 0, 13, "WM_TAKE_FOCUS"), NULL); 
  bool gotatoms = false;
  if(preply && freply){
    WM_PROTOCOLS = preply->atom;
    WM_TAKE_FOCUS = freply->atom;
    free(preply);
    free(freply);
    gotatoms = true;
    //qDebug() << " -- success";
  }
  //  - - Now update the protocols for the window
  if(gotatoms){ //requires the atoms
    //qDebug() << " - Get WM_PROTOCOLS";
    xcb_icccm_get_wm_protocols_reply_t proto;
    if( 1 == xcb_icccm_get_wm_protocols_reply(QX11Info::connection(), \
			xcb_icccm_get_wm_protocols_unchecked(QX11Info::connection(), win, WM_PROTOCOLS), \
			&proto, NULL) ){
	
      //Found the current protocols, see if it has the focus atom set
			//remove the take focus atom and re-save them
      bool needremove = false;
      //Note: This first loop is required so that we can initialize the modified list with a valid size
      //qDebug() << " -- Check current protocols";
      for(unsigned int i=0; i<proto.atoms_len; i++){
        if(proto.atoms[i] == WM_TAKE_FOCUS){ needremove = true; break;}
      }
      if(needremove){
	//qDebug() << " -- Remove WM_TAKE_FOCUS protocol";
	xcb_atom_t *protolist = new xcb_atom_t[proto.atoms_len-1];
	int num = 0;
	for(unsigned int i=0; i<proto.atoms_len; i++){
	  if(proto.atoms[i] != WM_TAKE_FOCUS){
	    protolist[num] = proto.atoms[i];
	    num++;
	  }
	}
	//qDebug() << " -- Re-save modified protocols";
	xcb_icccm_set_wm_protocols(QX11Info::connection(), win, WM_PROTOCOLS, num, protolist);
      }
      //qDebug() << " -- Clear protocols reply";
      xcb_icccm_get_wm_protocols_reply_wipe(&proto);
    }//end of get protocols check
  } //end of gotatoms check
  //Make sure it has the "dock" window type
  //  - get the current window types (Not necessary, only 1 type of window needed)
  
  //  - set the adjusted window type(s)
  //qDebug() << " - Adjust window type";
  xcb_atom_t list[1]; 
    list[0] = EWMH._NET_WM_WINDOW_TYPE_DOCK;
  xcb_ewmh_set_wm_window_type(&EWMH, win, 1, list);
  
  //Make sure it is on all workspaces
  //qDebug() << " - Set window as sticky";
  SetAsSticky(win);
	
}

// === SetAsDesktop() ===
void LXCB::SetAsDesktop(WId win){
  if(DEBUG){ qDebug() << "XCB: SetAsDesktop()"; }
  if(win==0){ return; }
  SetDisableWMActions(win); //also need to disable WM actions for this window
  xcb_atom_t list[1];
    list[0] = EWMH._NET_WM_WINDOW_TYPE_DESKTOP;
  xcb_ewmh_set_wm_window_type(&EWMH, win, 1, list);
}

// === CloseWindow() ===
void LXCB::CloseWindow(WId win){
  if(DEBUG){ qDebug() << "XCB: CloseWindow()"; }
  if(win==0){ return; }
  //This will close the specified window (might not close the entire application)
  xcb_ewmh_request_close_window(&EWMH, 0, win, QX11Info::getTimestamp(), XCB_EWMH_CLIENT_SOURCE_TYPE_OTHER);
}

// === KillClient() ===
void LXCB::KillClient(WId win){
  if(DEBUG){ qDebug() << "XCB: KillClient()"; }
  if(win==0){ return; }
  //This will forcibly close the application which created WIN 
  xcb_kill_client(QX11Info::connection(), win);
}

// === MinimizeWindow() ===
void LXCB::MinimizeWindow(WId win){ //request that the window be unmapped/minimized
  if(DEBUG){ qDebug() << "XCB: MinimizeWindow()"; }
  if(win==0){ return; }
  //Note: Fluxbox completely removes this window from the open list if unmapped manually
 // xcb_unmap_window(QX11Info::connection(), win);
  //xcb_flush(QX11Info::connection()); //make sure the command is sent out right away
	
  //Need to send a client message event for the window so the WM picks it up
  xcb_client_message_event_t event;
    event.response_type = XCB_CLIENT_MESSAGE;
    event.format = 32;
    event.window = win;
    event.type = EWMH._NET_WM_STATE;
    event.data.data32[0] = 1; //set to toggle (switch back and forth)
    event.data.data32[1] = EWMH._NET_WM_STATE_HIDDEN;
    event.data.data32[2] = 0;
    event.data.data32[3] = 0;
    event.data.data32[4] = 0;

  xcb_send_event(QX11Info::connection(), 0, QX11Info::appRootWindow(),  XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT, (const char *) &event);
}

// === ActivateWindow() ===
void LXCB::ActivateWindow(WId win){ //request that the window become active
  if(DEBUG){ qDebug() << "XCB: ActivateWindow();"; }
  if(win==0){ return; }
  //First need to get the currently active window
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_active_window_unchecked(&EWMH, 0);
  xcb_window_t actwin;
  if(1 != xcb_ewmh_get_active_window_reply(&EWMH, cookie, &actwin, NULL) ){
    actwin = 0;
  }	
  if(actwin == win){ return; } //requested window is already active
  
//Need to send a client message event for the window so the WM picks it up
  xcb_client_message_event_t event;
    event.response_type = XCB_CLIENT_MESSAGE;
    event.format = 32;
    event.window = win; //window to activate
    event.type = EWMH._NET_ACTIVE_WINDOW;
    event.data.data32[0] = 2; //pager/direct user interaction
    event.data.data32[1] = QX11Info::getTimestamp(); //current timestamp
    event.data.data32[2] = actwin; //currently active window (0 if none)
    event.data.data32[3] = 0;
    event.data.data32[4] = 0;

  xcb_send_event(QX11Info::connection(), 0, QX11Info::appRootWindow(),  XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT, (const char *) &event);
	
}

// ===== RestoreWindow() =====
void LXCB::RestoreWindow(WId win){
    uint32_t val = XCB_STACK_MODE_ABOVE;
    xcb_configure_window(QX11Info::connection(),  win, XCB_CONFIG_WINDOW_STACK_MODE, &val); //raise it
    xcb_map_window(QX11Info::connection(), win); //map it
}

// === MaximizeWindow() ===
void LXCB::MaximizeWindow(WId win, bool flagsonly){ //request that the window become maximized
  if(DEBUG){ qDebug() << "XCB: MaximizeWindow()"; }
  if(win==0){ return; }	
  if(flagsonly){
    //Directly set the flags on the window (bypassing the WM)
    xcb_atom_t list[2];
      list[0] = EWMH._NET_WM_STATE_MAXIMIZED_VERT;
      list[1] = EWMH._NET_WM_STATE_MAXIMIZED_HORZ;
    xcb_ewmh_set_wm_state(&EWMH, win, 2, list);
	  
  }else{
    //Need to send a client message event for the window so the WM picks it up
    xcb_client_message_event_t event;
    event.response_type = XCB_CLIENT_MESSAGE;
    event.format = 32;
    event.window = win;
    event.type = EWMH._NET_WM_STATE;
    event.data.data32[0] = 2; //set to toggle (switch back and forth)
    event.data.data32[1] = EWMH._NET_WM_STATE_MAXIMIZED_VERT;
    event.data.data32[2] = EWMH._NET_WM_STATE_MAXIMIZED_HORZ;
    event.data.data32[3] = 0;
    event.data.data32[4] = 0;

    xcb_send_event(QX11Info::connection(), 0, QX11Info::appRootWindow(),  XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT, (const char *) &event);
  }
}

// === MoveResizeWindow() ===
void LXCB::MoveResizeWindow(WId win, QRect geom){
  if(DEBUG){ qDebug() << "XCB: MoveResizeWindow()"; }
  if(win==0){ return; }
  //NOTE: geom needs to be in root/absolute coordinates!
  //qDebug() << "MoveResize Window:" << geom.x() << geom.y() << geom.width() << geom.height();
 
  //Move the window
  /*xcb_ewmh_request_moveresize_window(&EWMH, 0, win, XCB_GRAVITY_STATIC, XCB_EWMH_CLIENT_SOURCE_TYPE_OTHER, \
		XCB_EWMH_MOVERESIZE_WINDOW_X | XCB_EWMH_MOVERESIZE_WINDOW_Y | XCB_MOVERESIZE_WINDOW_WIDTH | XCB_MOVERESIZE_WINDOW_HEIGHT, \
		geom.x(), geom.y(), geom.width(), geom.height());*/
  
  //Use the basic XCB functions instead of ewmh (Issues with combining the XCB_EWMH_MOVERESIZE _*flags)
  uint32_t values[4];
  values[0] = geom.x(); values[1] = geom.y();
  values[2] = geom.width(); values[3] = geom.height();
  xcb_configure_window(QX11Info::connection(), win, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
	
}

// ===== ResizeWindow() =====
void LXCB::ResizeWindow(WId win, int width, int height){
  //Use the basic XCB functions instead of ewmh
  uint32_t values[] = {width, height};
  xcb_configure_window(QX11Info::connection(), win, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
}

// === ReserveLocation ===
void LXCB::ReserveLocation(WId win, QRect geom, QString loc){
  loc = loc.toLower().simplified();
  //Put the values in the proper structures
  xcb_ewmh_wm_strut_partial_t LOC;
    //Initialize the structure to zeros
    LOC.left = LOC.right = LOC.top = LOC.bottom = 0; //initial setting
    LOC.left_start_y = LOC.left_end_y = LOC.right_start_y = LOC.right_end_y = 0;
    LOC.top_start_x = LOC.top_end_x = LOC.bottom_start_x = LOC.bottom_end_x = 0;
   //Now put the values into the structure based on location
   if(loc=="top"){
    //top of screen
    LOC.top = geom.height(); //top width
    LOC.top_start_x = geom.x(); //top x start
    LOC.top_end_x = geom.x()+geom.width(); //top x end
  }else if(loc=="bottom"){
    //bottom of screen
    LOC.bottom = geom.height(); //bottom width
    LOC.bottom_start_x = geom.x(); //bottom x start
    LOC.bottom_end_x = geom.x()+geom.width(); //bottom x end
  }else if(loc=="left"){
    LOC.left = geom.width();
    LOC.left_start_y = geom.y();
    LOC.left_end_y = geom.y()+geom.height();
  }else{ //right
    LOC.right = geom.width();
    LOC.right_start_y = geom.y();
    LOC.right_end_y = geom.y()+geom.height();
  }
	
  //Change the property
  xcb_ewmh_set_wm_strut_partial(&EWMH, win, LOC); //_NET_WM_STRUT_PARTIAL (not always used)
  xcb_ewmh_set_wm_strut(&EWMH, win, LOC.left, LOC.right, LOC.top, LOC.bottom); //_NET_WM_STRUT
}

/*void LXCB::SetWindowBackground(QWidget *parent, QRect area, WId client){
  //Copy the image from the parent onto the client (parent/child - for system tray apps)
  //First create the background graphics context
  //qDebug() << "Create graphics context";
  //xcb_screen_t *root_screen = xcb_aux_get_screen(QX11Info::connection(), QX11Info::appScreen());
  uint32_t val = XCB_GX_CLEAR;
  xcb_gcontext_t graphic_context = xcb_generate_id(QX11Info::connection());
	xcb_create_gc(QX11Info::connection(), graphic_context, client, XCB_GC_BACKGROUND | XCB_GC_FOREGROUND, &val); 
  //qDebug() << "Copy Background Area";
  //Now copy the image onto the client background
  xcb_copy_area(QX11Info::connection(),
          parent->winId(),
          client,
          graphic_context,
          area.x(), area.y(),
          0, 0,
          area.width(), area.height());
  //Now re-map the client so it paints on top of the new background
  //qDebug() << "Map Window";
  //xcb_map_window(QX11Info::connection(), client);
  //Clean up variables
  xcb_free_gc(QX11Info::connection(), graphic_context);
}*/

// === EmbedWindow() ===
uint LXCB::EmbedWindow(WId win, WId container){
  if(DEBUG){ qDebug() << "XCB: EmbedWindow()"; }
  //This returns the damage control ID number (or 0 for a failure)
  if(win==0 || container==0){ return 0; }
  //qDebug() << "Embed Window:" << win << container;

  //Initialize any atoms that will be needed
  xcb_intern_atom_cookie_t ecookie = xcb_intern_atom_unchecked(QX11Info::connection(), 0, 7, "_XEMBED");
  
  xcb_intern_atom_reply_t *ereply = xcb_intern_atom_reply(QX11Info::connection(), ecookie, NULL);
  if(ereply==0){ return 0; } //unable to initialize the atom
  xcb_atom_t emb = ereply->atom;
  free(ereply); //done with this structure
  
  //Reparent the window into the container
  xcb_reparent_window(QX11Info::connection(), win, container, 0, 0);
  xcb_map_window(QX11Info::connection(), win);
  
  //Now send the embed event to the app
  //qDebug() << " - send _XEMBED event";
  xcb_client_message_event_t event;
    event.response_type = XCB_CLIENT_MESSAGE;
    event.format = 32;
    event.window = win;
    event.type = emb; //_XEMBED
    event.data.data32[0] = XCB_TIME_CURRENT_TIME; //CurrentTime; 
    event.data.data32[1] = 0; //XEMBED_EMBEDDED_NOTIFY
    event.data.data32[2] = 0;
    event.data.data32[3] = container; //WID of the container
    event.data.data32[4] = 0;

    xcb_send_event(QX11Info::connection(), 0, win,  XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT, (const char *) &event);
  
  //Now setup any redirects and return
  this->SelectInput(win); //Notify of structure changes
  xcb_composite_redirect_window(QX11Info::connection(), win, XCB_COMPOSITE_REDIRECT_MANUAL); //XCB_COMPOSITE_REDIRECT_[MANUAL/AUTOMATIC]);

  //Now map the window (will be a transparent child of the container)
  xcb_map_window(QX11Info::connection(), win);
  
  //Now create/register the damage handler
  // -- XCB (Note: The XCB damage registration is completely broken at the moment - 9/15/15, Ken Moore)
  //xcb_damage_damage_t dmgID = xcb_generate_id(QX11Info::connection()); //This is a typedef for a 32-bit unsigned integer
  //xcb_damage_create(QX11Info::connection(), dmgID, win, XCB_DAMAGE_REPORT_LEVEL_RAW_RECTANGLES);
  // -- XLib (Note: This is only used because the XCB routine above does not work - needs to be fixed upstream in XCB itself).
  Damage dmgID = XDamageCreate(QX11Info::display(), win, XDamageReportRawRectangles);
  
  //qDebug() << " - Done";
  return ( (uint) dmgID );	
}

// === Unembed Window() ===
bool LXCB::UnembedWindow(WId win){
  if(DEBUG){ qDebug() << "XCB: UnembedWindow()"; }
  if(win==0){ return false; }
  //Remove redirects
  uint32_t val[] = {XCB_EVENT_MASK_NO_EVENT};	
  xcb_change_window_attributes(QX11Info::connection(), win, XCB_CW_EVENT_MASK, val);
  //Make sure it is invisible
  xcb_unmap_window(QX11Info::connection(), win);
  //Reparent the window back to the root window
  xcb_reparent_window(QX11Info::connection(), win, QX11Info::appRootWindow(), 0, 0);
  return true;	
}

// === TrayImage() ===
QPixmap LXCB::TrayImage(WId win){
  QPixmap pix;
	
  //Get the current QScreen (for XCB->Qt conversion)
  QList<QScreen*> scrnlist = QApplication::screens();
  if(scrnlist.isEmpty()){ return pix; }	

  //Try to grab the given window directly with Qt
  if(pix.isNull()){
      pix = scrnlist[0]->grabWindow(win);
  }
  return pix;	
  
  //NOTE: Code below here saved for reference later (as necessary)
  // -------------------------------
  /*//First get the pixmap from the XCB compositing layer (since the tray images are redirected there)
  xcb_pixmap_t pixmap = xcb_generate_id(QX11Info::connection());
  xcb_composite_name_window_pixmap(QX11Info::connection(), win, pixmap);
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
  free(Greply); //done with geom reply*/
  
  /* NOTE: Found these little bit in the Qt sources - not sure if it is needed, but keep it here for reference
        // we may have to swap the byte order based on system type
        uint8_t image_byte_order = connection->setup()->image_byte_order;
        if ((QSysInfo::ByteOrder == QSysInfo::LittleEndian && image_byte_order == XCB_IMAGE_ORDER_MSB_FIRST)
            || (QSysInfo::ByteOrder == QSysInfo::BigEndian && image_byte_order == XCB_IMAGE_ORDER_LSB_FIRST))
        {
            for (int i=0; i < image.height(); i++) {
                    uint *p = (uint*)image.scanLine(i);
                    uint *end = p + image.width();
                    while (p < end) {
                        *p = ((*p << 24) & 0xff000000) | ((*p << 8) & 0x00ff0000)
                            | ((*p >> 8) & 0x0000ff00) | ((*p >> 24) & 0x000000ff);
                        p++;
                    }
            }
        }*/

        // fix-up alpha channel
        /*if (image.format() == QImage::Format_RGB32) {
            QRgb *p = (QRgb *)image.bits();
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x)
                    p[x] |= 0xff000000;
                p += bytes_per_line / 4;
            }*/

  //Convert the QImage into a QPixmap and return it
  //return QPixmap::fromImage(image.copy());
}

// ===== startSystemTray() =====
WId LXCB::startSystemTray(int screen){
  qDebug() << "Starting System Tray:" << screen;
  //Setup the freedesktop standards compliance
  
  //Get the appropriate atom for this screen
  QString str = QString("_NET_SYSTEM_TRAY_S%1").arg(QString::number(screen));
  //qDebug() << "Default Screen Atom Name:" << str;
  xcb_intern_atom_reply_t *treply = xcb_intern_atom_reply(QX11Info::connection(), \
			xcb_intern_atom(QX11Info::connection(), 0, str.length(), str.toLocal8Bit()), NULL);
  xcb_intern_atom_reply_t *oreply = xcb_intern_atom_reply(QX11Info::connection(), \
			xcb_intern_atom(QX11Info::connection(), 0, 28, "_NET_SYSTEM_TRAY_ORIENTATION"), NULL);  
  xcb_intern_atom_reply_t *vreply = xcb_intern_atom_reply(QX11Info::connection(), \
			xcb_intern_atom(QX11Info::connection(), 0, 23, "_NET_SYSTEM_TRAY_VISUAL"), NULL);  
  if(treply==0){
    qDebug() << " - ERROR: Could not initialize _NET_SYSTEM_TRAY_S<num> atom";
    return 0;
  }
  if(oreply==0){
    qDebug() << " - ERROR: Could not initialize _NET_SYSTEM_TRAY_ORIENTATION atom";
    return 0;	  
  }
  if(vreply==0){
    qDebug() << " - ERROR: Could not initialize _NET_SYSTEM_TRAY_VISUAL atom";
    return 0;	  
  }
  xcb_atom_t _NET_SYSTEM_TRAY_S = treply->atom;
  xcb_atom_t _NET_SYSTEM_TRAY_ORIENTATION = oreply->atom;
  xcb_atom_t _NET_SYSTEM_TRAY_VISUAL = vreply->atom;
  free(treply); //done with atom generation
  free(oreply);
  free(vreply);
  
  //Make sure that there is no other system tray running
  xcb_get_selection_owner_reply_t *ownreply = xcb_get_selection_owner_reply(QX11Info::connection(), \
						xcb_get_selection_owner_unchecked(QX11Info::connection(), _NET_SYSTEM_TRAY_S), NULL);
  if(ownreply==0){
    qWarning() << " - Could not get owner selection reply";
    return 0;
  }
  if(ownreply->owner != 0){
    free(ownreply);
    qWarning() << " - An alternate system tray is currently in use";
    return 0;
  }
  free(ownreply);
  
  //Create a simple window to register as the tray (not visible - just off the screen)
  xcb_screen_t *root_screen = xcb_aux_get_screen(QX11Info::connection(), QX11Info::appScreen());
  uint32_t params[] = {1};
  WId LuminaSessionTrayID = xcb_generate_id(QX11Info::connection()); //need a new ID
    xcb_create_window(QX11Info::connection(), root_screen->root_depth, \
		LuminaSessionTrayID, root_screen->root, -1, -1, 1, 1, 0, \
		XCB_WINDOW_CLASS_INPUT_OUTPUT, root_screen->root_visual, \
		XCB_CW_OVERRIDE_REDIRECT, params);
		
  //Now register this widget as the system tray
  xcb_set_selection_owner(QX11Info::connection(), LuminaSessionTrayID, _NET_SYSTEM_TRAY_S, XCB_CURRENT_TIME);
  //Make sure that it was registered properly
  ownreply = xcb_get_selection_owner_reply(QX11Info::connection(), \
						xcb_get_selection_owner_unchecked(QX11Info::connection(), _NET_SYSTEM_TRAY_S), NULL);
  
  if(ownreply==0 || ownreply->owner != LuminaSessionTrayID){
    if(ownreply!=0){ free(ownreply); }
    qWarning() << " - Could not register the system tray";
    xcb_destroy_window(QX11Info::connection(), LuminaSessionTrayID);
    return 0;
  }
  free(ownreply); //done with structure
  
  //Now register the orientation of the system tray
  uint32_t orient = _NET_SYSTEM_TRAY_ORIENTATION_HORZ;
   xcb_change_property(QX11Info::connection(), XCB_PROP_MODE_REPLACE, LuminaSessionTrayID, \
			_NET_SYSTEM_TRAY_ORIENTATION, XCB_ATOM_CARDINAL, 32, 1, &orient);

  //Now set the visual ID for the system tray (same as the root window, but TrueColor)
    xcb_visualtype_t *type = xcb_aux_find_visual_by_attrs(root_screen, XCB_VISUAL_CLASS_TRUE_COLOR, 32);
    if(type!=0){
      xcb_change_property(QX11Info::connection(), XCB_PROP_MODE_REPLACE, LuminaSessionTrayID, \
	  _NET_SYSTEM_TRAY_VISUAL, XCB_ATOM_VISUALID, 32, 1, &type->visual_id);	    
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
    event.data.data32[2] = LuminaSessionTrayID;
    event.data.data32[3] = 0;
    event.data.data32[4] = 0;

    xcb_send_event(QX11Info::connection(), 0, root_screen->root,  XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT, (const char *) &event);
  
  //Success
  return LuminaSessionTrayID;
}

// ===== closeSystemTray() =====
void LXCB::closeSystemTray(WId trayID){
  xcb_destroy_window(QX11Info::connection(), trayID);
}

// === SetScreenWorkArea() ===
/*void LXCB::SetScreenWorkArea(unsigned int screen, QRect rect){
  //This is only useful because Fluxbox does not set the _NET_WORKAREA root atom
  // 	This needs to be better incorporated into the new window manager later

  //First get the current workarea array (for all monitors/screens)
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_workarea_unchecked(&EWMH, 0);
  xcb_ewmh_get_workarea_reply_t work;
  if(0==xcb_ewmh_get_workarea_reply(&EWMH, cookie, &work, NULL)){ return; } //Error: Could not retrieve current work areas
  //Save what we need only from the reply
  unsigned int desks = work.workarea_len;
  if(desks <= screen){ return; } //invalid screen to modify
  qDebug() << "Number of desktops/screens:" << desks;
  xcb_ewmh_geometry_t *dareas = work.workarea;
  //Adjust the work area for the input monitor/screen
  dareas[screen].x = rect.x();
  dareas[screen].y = rect.y();
  dareas[screen].width = rect.width();
  dareas[screen].height = rect.height();
  //Now save the array again
  xcb_ewmh_set_workarea(&EWMH, 0, desks, dareas); //_NET_WORKAREA
  //Make sure to clear that reply
  xcb_ewmh_get_workarea_reply_wipe(&work);
}*/

//============
// WM Functions (directly changing properties/settings)
//  - Using these directly may prevent the WM from seeing the change
//============
void LXCB::WM_CloseWindow(WId win){
  xcb_destroy_window(QX11Info::connection(), win);
}
	
