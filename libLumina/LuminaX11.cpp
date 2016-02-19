//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014-2015, Ken Moore
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

// private function
void LXCB::createWMAtoms(){
  ATOMS.clear();
  atoms.clear();
  //List the atoms needed by some WM functions
  atoms << "WM_TAKE_FOCUS" << "WM_DELETE_WINDOW" << "WM_PROTOCOLS"; //WM_PROTOCOLS

  //Create all the requests for the atoms
  QList<xcb_intern_atom_reply_t*> reply;
  for(int i=0; i<atoms.length(); i++){
      reply << xcb_intern_atom_reply(QX11Info::connection(), \
			xcb_intern_atom(QX11Info::connection(), 0, atoms[i].length(), atoms[i].toLocal8Bit()), NULL); 
  }
  //Now evaluate all the requests and save the atoms
  for(int i=0; i<reply.length(); i++){
    if(reply[i]!=0){
      ATOMS << reply[i]->atom;
      free(reply[i]); //done with this reply
    }else{
      //Invalid atom - could not be created
      atoms.removeAt(i);
      reply.removeAt(i);
      i--;
    }
  }
  
  
  
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
  delete [] list;
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
  xcb_get_property_cookie_t scookie = xcb_ewmh_get_wm_state_unchecked(&EWMH, win);
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_desktop_unchecked(&EWMH, win);
  if(cookie.sequence == 0){ return wkspace; } 
  xcb_ewmh_get_wm_desktop_reply(&EWMH, cookie, &wkspace, NULL);
  xcb_ewmh_get_atoms_reply_t reply;
  if(1==xcb_ewmh_get_wm_state_reply(&EWMH,scookie, &reply, NULL)){
    //Also check if this window is "sticky", in which case return the current workspace (on all of them)
    for(unsigned int i=0; i<reply.atoms_len; i++){
      if(reply.atoms[i]==EWMH._NET_WM_STATE_STICKY){ wkspace = LXCB::CurrentWorkspace(); break; }
    }
  }
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
LXCB::WINDOWVISIBILITY LXCB::WindowState(WId win){
  if(DEBUG){ qDebug() << "XCB: WindowState()"; }
  if(win==0){ return IGNORE; }
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_state_unchecked(&EWMH, win);
  if(cookie.sequence == 0){ return IGNORE; } 
  xcb_ewmh_get_atoms_reply_t states;
  WINDOWVISIBILITY cstate = IGNORE;
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
void LXCB::SelectInput(WId win, bool isEmbed){
  uint32_t mask;
  if(isEmbed){
    mask = XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |  XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_PROPERTY_CHANGE;
  }else{
    mask = XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_PROPERTY_CHANGE;
  }
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
  if(win==0 || container==0 || LXCB::WindowClass(win).isEmpty() ){ return 0; } //invalid window (destroyed before getting here?)
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
  this->SelectInput(win, true); //Notify of structure changes
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
void LXCB::WM_CloseWindow(WId win, bool force){

  if(!force){ // && WM_ICCCM_GetProtocols(win).testFlag(LXCB::DELETE_WINDOW)){
    //Send the window a WM_DELETE_WINDOW message
    if(atoms.isEmpty()){ createWMAtoms(); } //need these atoms
    xcb_client_message_event_t event;
    event.response_type = XCB_CLIENT_MESSAGE;
    event.format = 32;
    event.window = win;
    event.type = ATOMS[atoms.indexOf("WM_PROTOCOLS")];
    event.data.data32[0] = ATOMS[atoms.indexOf("WM_DELETE_WINDOW")];  
    event.data.data32[1] = XCB_TIME_CURRENT_TIME; //CurrentTime;
    event.data.data32[2] = 0;
    event.data.data32[3] = 0;
    event.data.data32[4] = 0;

    xcb_send_event(QX11Info::connection(), 0, win,  XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT, (const char *) &event);
    xcb_flush(QX11Info::connection());
  }else{ xcb_destroy_window(QX11Info::connection(), win); }

}

void LXCB::WM_ShowWindow(WId win){
  xcb_map_window(QX11Info::connection(), win);
}

void LXCB::WM_HideWindow(WId win){
  xcb_unmap_window(QX11Info::connection(), win);
}

QList<WId> LXCB::WM_RootWindows(){
  xcb_query_tree_cookie_t cookie = xcb_query_tree(QX11Info::connection(), QX11Info::appRootWindow());
  xcb_query_tree_reply_t *reply = 0;
  QList<WId> out;
  reply=xcb_query_tree_reply(QX11Info::connection(), cookie, NULL);
  if(reply!=0){
    int num = xcb_query_tree_children_length(reply);
    xcb_window_t *children = xcb_query_tree_children(reply);
    for(int i=0; i<num; i++){ 
      if(!out.contains(children[i])){ out << children[i]; }
    }
    free(reply);
  }
  return out;
}

WId LXCB::WM_CreateWindow(WId parent){
  if(parent ==0){ parent = QX11Info::appRootWindow(); }
  xcb_screen_t *root_screen = xcb_aux_get_screen(QX11Info::connection(), QX11Info::appScreen());
  uint32_t params[] = {1};
  WId win = xcb_generate_id(QX11Info::connection()); //need a new ID
    xcb_create_window(QX11Info::connection(), root_screen->root_depth, \
		win, parent, -1, -1, 1, 1, 0, \
		XCB_WINDOW_CLASS_INPUT_OUTPUT, root_screen->root_visual, \
		XCB_CW_OVERRIDE_REDIRECT, params);
  return win;
}

bool LXCB::WM_ManageWindow(WId win, bool needsmap){
#define CLIENT_WIN_EVENT_MASK (XCB_EVENT_MASK_PROPERTY_CHANGE |  \
                          XCB_EVENT_MASK_STRUCTURE_NOTIFY |	\
                          XCB_EVENT_MASK_FOCUS_CHANGE)
  //return whether the window is/should be managed
  if(WM_ICCCM_GetClass(win).isEmpty() ){ return false; }
  xcb_get_window_attributes_cookie_t cookie = xcb_get_window_attributes(QX11Info::connection(), win);
  xcb_get_window_attributes_reply_t *attr = xcb_get_window_attributes_reply(QX11Info::connection(), cookie, NULL);
  if(attr == 0){ return false; } //could not get attributes of window
  if(attr->override_redirect){ free(attr); return false; } //window has override redirect set (do not manage)
  if(!needsmap && attr->map_state != XCB_MAP_STATE_VIEWABLE){ 
    //window is never supposed to be visible (lots of these)
    //if( !WM_ICCCM_GetClass(win).contains("xterm") ){ //Some windows mis-set this flag
    qDebug() << " - Not Viewable.." << WM_ICCCM_GetClass(win);
    free(attr);  return false; 
    //}
  }
  //Setup event handling on the window
  uint32_t value_list[1] = {CLIENT_WIN_EVENT_MASK};
  if( xcb_request_check(QX11Info::connection(), \
	  xcb_change_window_attributes_checked(QX11Info::connection(), win, XCB_CW_EVENT_MASK, value_list ) ) ){
    //Could not change event mask - did the window get deleted already?
    free(attr);
    qDebug() << " - Could not change event mask";
    return false;		  
  }
  
  return true;
}

QRect LXCB::WM_Window_Geom(WId win){
  xcb_get_geometry_cookie_t cookie = xcb_get_geometry_unchecked(QX11Info::connection(), win);
  xcb_get_geometry_reply_t *reply = 0;
  QRect geom;
  reply = xcb_get_geometry_reply(QX11Info::connection(), cookie, NULL);
  if(reply!=0){
    geom = QRect(reply->x, reply->y, reply->width, reply->height);
    free(reply);
  }
  return geom;
}

void LXCB::setupEventsForFrame(WId frame){
  #define FRAME_WIN_EVENT_MASK (XCB_EVENT_MASK_BUTTON_PRESS | 	\
                          XCB_EVENT_MASK_BUTTON_RELEASE | 	\
                          XCB_EVENT_MASK_POINTER_MOTION |	\
			  XCB_EVENT_MASK_BUTTON_MOTION |	\
                          XCB_EVENT_MASK_EXPOSURE |		\
                          XCB_EVENT_MASK_STRUCTURE_NOTIFY |	\
                          XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |	\
                          XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |	\
                          XCB_EVENT_MASK_ENTER_WINDOW)
	
  uint32_t value_list[1] = {FRAME_WIN_EVENT_MASK};
  xcb_change_window_attributes(QX11Info::connection(), frame, XCB_CW_EVENT_MASK, value_list);
}

bool LXCB::setupEventsForRoot(WId root){
 #define ROOT_WIN_EVENT_MASK (XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |  \
                         XCB_EVENT_MASK_BUTTON_PRESS | 	\
                         XCB_EVENT_MASK_STRUCTURE_NOTIFY |	\
			 XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |	\
                         XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |	\
                         XCB_EVENT_MASK_POINTER_MOTION | 	\
                         XCB_EVENT_MASK_PROPERTY_CHANGE | 	\
			 XCB_EVENT_MASK_FOCUS_CHANGE |	\
                         XCB_EVENT_MASK_ENTER_WINDOW)
	
  if(root==0){ root = QX11Info::appRootWindow(); }
  uint32_t value_list[1] = {ROOT_WIN_EVENT_MASK};
  xcb_generic_error_t *status = xcb_request_check( QX11Info::connection(), xcb_change_window_attributes_checked(QX11Info::connection(), root, XCB_CW_EVENT_MASK, value_list)); 
  return (status==0);
}
// --------------------------------------------------
// ICCCM Standards (older standards)
// --------------------------------------------------
// -- WM_NAME
QString LXCB::WM_ICCCM_GetName(WId win){
  xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_name_unchecked(QX11Info::connection(), win);
  xcb_icccm_get_text_property_reply_t reply;
  if(1 != xcb_icccm_get_wm_name_reply(QX11Info::connection(), cookie, &reply, NULL) ){
    return ""; //error in fetching name
  }else{
    return QString::fromLocal8Bit(reply.name);
  }
}

void LXCB::WM_ICCCM_SetName(WId win, QString name){
  xcb_icccm_set_wm_name(QX11Info::connection(), win, XCB_ATOM_STRING, 8, name.length(), name.toLocal8Bit());
}

// -- WM_ICON_NAME
QString LXCB::WM_ICCCM_GetIconName(WId win){
  xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_icon_name_unchecked(QX11Info::connection(), win);
  xcb_icccm_get_text_property_reply_t reply;
  if(1 != xcb_icccm_get_wm_icon_name_reply(QX11Info::connection(), cookie, &reply, NULL) ){
    return ""; //error in fetching name
  }else{
    return QString::fromLocal8Bit(reply.name);
  }
}

void LXCB::WM_ICCCM_SetIconName(WId win, QString name){
  xcb_icccm_set_wm_icon_name(QX11Info::connection(), win, XCB_ATOM_STRING, 8, name.length(), name.toLocal8Bit());
}

// -- WM_CLIENT_MACHINE
QString LXCB::WM_ICCCM_GetClientMachine(WId win){
  xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_client_machine_unchecked(QX11Info::connection(), win);
  xcb_icccm_get_text_property_reply_t reply;
  if(1 != xcb_icccm_get_wm_client_machine_reply(QX11Info::connection(), cookie, &reply, NULL) ){
    return ""; //error in fetching name
  }else{
    return QString::fromLocal8Bit(reply.name);
  }
}

void LXCB::WM_ICCCM_SetClientMachine(WId win, QString name){
  xcb_icccm_set_wm_client_machine(QX11Info::connection(), win, XCB_ATOM_STRING, 8, name.length(), name.toLocal8Bit());
}

// -- WM_CLASS
QString LXCB::WM_ICCCM_GetClass(WId win){
  xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_class_unchecked(QX11Info::connection(), win);
  xcb_icccm_get_wm_class_reply_t reply;
  if(1 != xcb_icccm_get_wm_class_reply(QX11Info::connection(), cookie, &reply, NULL) ){
    return ""; //error in fetching name
  }else{
    //Returns: "<instance name>::::<class name>"
    return ( QString::fromLocal8Bit(reply.instance_name)+"::::"+QString::fromLocal8Bit(reply.class_name) );
  }
}

void LXCB::WM_ICCCM_SetClass(WId win, QString name){
  xcb_icccm_set_wm_class(QX11Info::connection(), win, name.length(), name.toLocal8Bit());
}

// -- WM_TRANSIENT_FOR
WId LXCB::WM_ICCCM_GetTransientFor(WId win){
  xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_transient_for_unchecked(QX11Info::connection(), win);
  xcb_window_t trans;
  if(1!= xcb_icccm_get_wm_transient_for_reply(QX11Info::connection(), cookie, &trans, NULL) ){
    return win; //error in fetching transient window ID (or none found)
  }else{
    return trans;
  }
}

void LXCB::WM_ICCCM_SetTransientFor(WId win, WId transient){
  xcb_icccm_set_wm_transient_for(QX11Info::connection(), win, transient);
}

// -- WM_SIZE_HINTS (older property?)
icccm_size_hints LXCB::WM_ICCCM_GetSizeHints(WId win){
  //most values in structure are -1 if not set
  icccm_size_hints hints;
  xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_size_hints_unchecked(QX11Info::connection(), win, XCB_ATOM_WM_SIZE_HINTS);
  xcb_size_hints_t reply;
  if(1==xcb_icccm_get_wm_size_hints_reply(QX11Info::connection(), cookie, &reply, NULL) ){
    //Now go though and move any data into the output struct
    if( (reply.flags&XCB_ICCCM_SIZE_HINT_US_POSITION)==XCB_ICCCM_SIZE_HINT_US_POSITION ){ hints.x=reply.x; hints.y=reply.y; }
    if( (reply.flags&XCB_ICCCM_SIZE_HINT_US_SIZE)==XCB_ICCCM_SIZE_HINT_US_SIZE ){ hints.width=reply.width; hints.height=reply.height; }
    if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_POSITION)==XCB_ICCCM_SIZE_HINT_P_POSITION ){ hints.x=reply.x; hints.y=reply.y; }
    if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_SIZE)==XCB_ICCCM_SIZE_HINT_P_SIZE ){ hints.width=reply.width; hints.height=reply.height; }
    if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_MIN_SIZE)==XCB_ICCCM_SIZE_HINT_P_MIN_SIZE ){ hints.min_width=reply.min_width; hints.min_height=reply.min_height; }
    if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_MAX_SIZE)==XCB_ICCCM_SIZE_HINT_P_MAX_SIZE ){ hints.max_width=reply.max_width; hints.max_height=reply.max_height; }
    if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_RESIZE_INC)==XCB_ICCCM_SIZE_HINT_P_RESIZE_INC ){ hints.width_inc=reply.width_inc; hints.height_inc=reply.height_inc; }
    if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_ASPECT)==XCB_ICCCM_SIZE_HINT_P_ASPECT ){ hints.min_aspect_num=reply.min_aspect_num; hints.min_aspect_den=reply.min_aspect_den; hints.max_aspect_num=reply.max_aspect_num; hints.max_aspect_den=reply.max_aspect_den;}
    if( (reply.flags&XCB_ICCCM_SIZE_HINT_BASE_SIZE)==XCB_ICCCM_SIZE_HINT_BASE_SIZE ){ hints.base_width=reply.base_width; hints.base_height=reply.base_height; }
    if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_WIN_GRAVITY)==XCB_ICCCM_SIZE_HINT_P_WIN_GRAVITY ){ hints.win_gravity=reply.win_gravity; }
    //free(reply);
  }
  return hints;
}

//void WM_ICCCM_SetSizeHints(WId win, icccm_size_hints hints);

// -- WM_NORMAL_HINTS (newer property? - check for this before falling back on WM_SIZE_HINTS)
icccm_size_hints LXCB::WM_ICCCM_GetNormalHints(WId win){
//most values in structure are -1 if not set
  //most values in structure are -1 if not set
  icccm_size_hints hints;
  xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_normal_hints_unchecked(QX11Info::connection(), win);
  xcb_size_hints_t reply;
  if(1==xcb_icccm_get_wm_normal_hints_reply(QX11Info::connection(), cookie, &reply, NULL) ){
    //Now go though and move any data into the output struct
    if( (reply.flags&XCB_ICCCM_SIZE_HINT_US_POSITION)==XCB_ICCCM_SIZE_HINT_US_POSITION ){ hints.x=reply.x; hints.y=reply.y; }
    if( (reply.flags&XCB_ICCCM_SIZE_HINT_US_SIZE)==XCB_ICCCM_SIZE_HINT_US_SIZE ){ hints.width=reply.width; hints.height=reply.height; }
    if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_POSITION)==XCB_ICCCM_SIZE_HINT_P_POSITION ){ hints.x=reply.x; hints.y=reply.y; }
    if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_SIZE)==XCB_ICCCM_SIZE_HINT_P_SIZE ){ hints.width=reply.width; hints.height=reply.height; }
    if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_MIN_SIZE)==XCB_ICCCM_SIZE_HINT_P_MIN_SIZE ){ hints.min_width=reply.min_width; hints.min_height=reply.min_height; }
    if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_MAX_SIZE)==XCB_ICCCM_SIZE_HINT_P_MAX_SIZE ){ hints.max_width=reply.max_width; hints.max_height=reply.max_height; }
    if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_RESIZE_INC)==XCB_ICCCM_SIZE_HINT_P_RESIZE_INC ){ hints.width_inc=reply.width_inc; hints.height_inc=reply.height_inc; }
    if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_ASPECT)==XCB_ICCCM_SIZE_HINT_P_ASPECT ){ hints.min_aspect_num=reply.min_aspect_num; hints.min_aspect_den=reply.min_aspect_den; hints.max_aspect_num=reply.max_aspect_num; hints.max_aspect_den=reply.max_aspect_den;}
    if( (reply.flags&XCB_ICCCM_SIZE_HINT_BASE_SIZE)==XCB_ICCCM_SIZE_HINT_BASE_SIZE ){ hints.base_width=reply.base_width; hints.base_height=reply.base_height; }
    if( (reply.flags&XCB_ICCCM_SIZE_HINT_P_WIN_GRAVITY)==XCB_ICCCM_SIZE_HINT_P_WIN_GRAVITY ){ hints.win_gravity=reply.win_gravity; }
    //free(reply);
  }
  return hints;	
}

/*void LXCB::WM_ICCCM_SetNormalHints(WId win, icccm_size_hints hints){
  //Convert the data structure into the proper format
  xcb_size_hints_t xhints;
  if(hints.x>=0 || hints.y>=0){ xcb_icccm_size_hints_set_position(&xhints, 1, hints.x, hints.y); }
  //if(hints.width>=0
  
  xcb_icccm_set_wm_normal_hints(QX11Info::connection(), win, &xhints);
}*/

// -- WM_HINTS
	
// -- WM_PROTOCOLS
LXCB::ICCCM_PROTOCOLS LXCB::WM_ICCCM_GetProtocols(WId win){
  if(atoms.isEmpty()){ createWMAtoms(); }
  xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_protocols(QX11Info::connection(), win, EWMH.WM_PROTOCOLS);
  xcb_icccm_get_wm_protocols_reply_t reply;
  LXCB::ICCCM_PROTOCOLS flags;
  if(1==xcb_icccm_get_wm_protocols_reply(QX11Info::connection(), cookie, &reply, NULL) ){
    for(unsigned int i=0; i<reply.atoms_len; i++){
      if(reply.atoms[i]==ATOMS[atoms.indexOf("WM_TAKE_FOCUS")]){ flags = flags | TAKE_FOCUS; }
      else if(reply.atoms[i]==ATOMS[atoms.indexOf("WM_DELETE_WINDOW")]){ flags = flags | DELETE_WINDOW; }
    }
  }
  return flags;	
}

void LXCB::WM_ICCCM_SetProtocols(WId win, LXCB::ICCCM_PROTOCOLS flags){
  if(atoms.isEmpty()){ createWMAtoms(); }
  xcb_atom_t *list;
  int num;
  if(flags.testFlag(TAKE_FOCUS) && flags.testFlag(DELETE_WINDOW)){
    num = 2;
    list = new xcb_atom_t[2];
    list[0] = ATOMS[atoms.indexOf("WM_TAKE_FOCUS")];
    list[1] = ATOMS[atoms.indexOf("WM_DELETE_WINDOW")];
  }else if(flags.testFlag(TAKE_FOCUS)){
    num = 1;
    list = new xcb_atom_t[1];
    list[0] = ATOMS[atoms.indexOf("WM_TAKE_FOCUS")];
  }else if(flags.testFlag(DELETE_WINDOW)){
    num = 1;
    list = new xcb_atom_t[1];
    list[0] = ATOMS[atoms.indexOf("WM_DELETE_WINDOW")];
  }else{
    num = 0;
    list = new xcb_atom_t[0];
  }
  xcb_icccm_set_wm_protocols(QX11Info::connection(), win, EWMH.WM_PROTOCOLS, num, list);
  
}

// --------------------------------------------------------
// NET_WM Standards (newer standards)
// --------------------------------------------------------
// _NET_SUPPORTED (Root)
void LXCB::WM_Set_Root_Supported(){
  //NET_WM standards (ICCCM implied - no standard way to list those)
  xcb_atom_t list[] = {};
  xcb_ewmh_set_supported(&EWMH, QX11Info::appScreen(), 0,list);
}

// _NET_CLIENT_LIST
QList<WId> LXCB::WM_Get_Client_List(bool stacking){
  QList<WId> out;
  if(stacking){
    xcb_get_property_cookie_t cookie = xcb_ewmh_get_client_list_stacking(&EWMH, QX11Info::appScreen());
    xcb_ewmh_get_windows_reply_t reply;
    if(1==xcb_ewmh_get_client_list_stacking_reply(&EWMH, cookie, &reply, NULL) ){
      for(unsigned int i=0; i<reply.windows_len; i++){
        out << reply.windows[i];
      }
    }
  }else{
    xcb_get_property_cookie_t cookie = xcb_ewmh_get_client_list(&EWMH, QX11Info::appScreen());
    xcb_ewmh_get_windows_reply_t reply;
    if(1==xcb_ewmh_get_client_list_reply(&EWMH, cookie, &reply, NULL) ){
      for(unsigned int i=0; i<reply.windows_len; i++){
        out << reply.windows[i];
      }
    }
  }
  return out;
}

void LXCB::WM_Set_Client_List(QList<WId> list, bool stacking){
  //convert the QList into a generic array
  xcb_window_t array[list.length()];
  for(int i=0; i<list.length(); i++){ array[i] = list[i]; }
  if(stacking){
    xcb_ewmh_set_client_list_stacking(&EWMH, QX11Info::appScreen(), list.length(), array);
  }else{
    xcb_ewmh_set_client_list(&EWMH, QX11Info::appScreen(), list.length(), array);
  }	  

}

// _NET_NUMBER_OF_DESKTOPS
unsigned int LXCB::WM_Get_Number_Desktops(){
  //return value equals 0 for errors
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_number_of_desktops_unchecked(&EWMH, QX11Info::appScreen());
  uint32_t number = 0;
  xcb_ewmh_get_number_of_desktops_reply(&EWMH, cookie, &number, NULL);
  return number;
}

void LXCB::WM_SetNumber_Desktops(unsigned int number){
  //NOTE: number should be at least 1	
  xcb_ewmh_set_number_of_desktops(&EWMH, QX11Info::appScreen(), number);
}

// _NET_DESKTOP_GEOMETRY
QSize LXCB::WM_Get_Desktop_Geometry(){
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_desktop_geometry(&EWMH, QX11Info::appScreen());
  uint32_t wid, hi;
  wid = hi = 0;
  xcb_ewmh_get_desktop_geometry_reply(&EWMH, cookie, &wid, &hi, NULL);
  return QSize(wid,hi);
}

void LXCB::WM_Set_Desktop_Geometry(QSize size){
  xcb_ewmh_set_desktop_geometry(&EWMH, QX11Info::appScreen(), size.width(), size.height());
}

// _NET_DESKTOP_VIEWPORT
QList<QPoint> LXCB::WM_Get_Desktop_Viewport(){
  QList<QPoint> out;
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_desktop_viewport_unchecked(&EWMH, QX11Info::appScreen());
  xcb_ewmh_get_desktop_viewport_reply_t reply;
  if(1==xcb_ewmh_get_desktop_viewport_reply(&EWMH, cookie, &reply, NULL) ){
    for(unsigned int i=0; i<reply.desktop_viewport_len; i++){
      out << QPoint( reply.desktop_viewport[i].x, reply.desktop_viewport[i].y );
    }
    xcb_ewmh_get_desktop_viewport_reply_wipe(&reply); //clean up the reply structure first
  }	  
  return out;
}

void LXCB::WM_Set_Desktop_Viewport(QList<QPoint> list){
  //Turn the QList into xcb_ewmh_coordinates_t*
  xcb_ewmh_coordinates_t array[list.length()];
  for(int i=0; i<list.length(); i++){ array[i].x=list[i].x(); array[i].y=list[i].y(); }
  //Now set the property
  xcb_ewmh_set_desktop_viewport(&EWMH, QX11Info::appScreen(), list.length(), array);
}

// _NET_CURRENT_DESKTOP
int LXCB::WM_Get_Current_Desktop(){
  //Returns -1 for errors
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_current_desktop_unchecked(&EWMH, QX11Info::appScreen());
  uint32_t num = 0;
  if(1==xcb_ewmh_get_current_desktop_reply(&EWMH, cookie, &num, NULL) ){
    return num;
  }else{
    return -1;
  }
}

void LXCB::WM_Set_Current_Desktop(unsigned int num){
  xcb_ewmh_set_current_desktop(&EWMH, QX11Info::appScreen(), num);
}

// _NET_DESKTOP_NAMES
QStringList LXCB::WM_Get_Desktop_Names(){
  QStringList out;
  // ** ISSUES with the XCB_EWMH strings reply structure - 11/11/15  (skip for now)
  // (Appears to be a char* instead of char** in the class definitions)
  /*xcb_get_property_cookie_t cookie = xcb_ewmh_get_desktop_names_unchecked(&EWMH, QX11Info::appScreen());
  xcb_ewmh_get_utf8_strings_reply_t reply;
  if(1==xcb_ewmh_get_desktop_names_reply(&EWMH, cookie, &reply, NULL) ){
    for(unsigned int i=0; i<reply.strings_len; i++){
      out << QString::fromUtf8( QByteArray(reply.strings[i]) );
    }
  }*/
  return out;
}

void LXCB::WM_Set_Desktop_Names(QStringList){// list){
  // ** ISSUES with the XCB_EWMH strings input structure - 11/11/15  (skip for now)
  // (Appears to be a char* instead of char** in the class definitions)
  /*//Convert to an array of char arrays
  char *array[ list.length() ];
  for(int i=0; i<list.length(); i++){array[i] = list[i].toUtf8().data(); }
  //Now set the property
  xcb_ewmh_set_desktop_names(&EWMH, QX11Info::appScreen(), list.length(), array);
  */
}

// _NET_ACTIVE_WINDOW
WId LXCB::WM_Get_Active_Window(){
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_active_window_unchecked(&EWMH, QX11Info::appScreen());
  xcb_window_t win = 0;
  xcb_ewmh_get_active_window_reply(&EWMH, cookie, &win, NULL);
  return win;
}

void LXCB::WM_Set_Active_Window(WId win){
  xcb_ewmh_set_active_window(&EWMH, QX11Info::appScreen(), win);	
}

// _NET_WORKAREA
QList<QRect> LXCB::WM_Get_Workarea(){
  QList<QRect> out;
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_workarea_unchecked(&EWMH, QX11Info::appScreen());
  xcb_ewmh_get_workarea_reply_t reply;
  if(1==xcb_ewmh_get_workarea_reply(&EWMH, cookie, &reply, NULL) ){
    for(unsigned int i=0; i<reply.workarea_len ;i++){
      out << QRect( reply.workarea[i].x, reply.workarea[i].y, reply.workarea[i].width, reply.workarea[i].height);
    }
    xcb_ewmh_get_workarea_reply_wipe(&reply);
  }
  return out;
}

void LXCB::WM_Set_Workarea(QList<QRect> list){
  //Convert to the XCB/EWMH data structures
  xcb_ewmh_geometry_t array[list.length()];
  for(int i=0; i<list.length(); i++){
    array[i].x = list[i].x(); array[i].y = list[i].y();
    array[i].width = list[i].width(); array[i].height = list[i].height();
  }
  //Now set the property
  xcb_ewmh_set_workarea(&EWMH, QX11Info::appScreen(), list.length(), array);
}

// _NET_SUPPORTING_WM_CHECK
WId LXCB::WM_Get_Supporting_WM(WId win){
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_supporting_wm_check_unchecked(&EWMH, win);
  xcb_window_t out = 0;
  xcb_ewmh_get_supporting_wm_check_reply(&EWMH, cookie, &out, NULL);
  return win;
}

void LXCB::WM_Set_Supporting_WM(WId child){
  //Set this property on the root window first
  xcb_ewmh_set_supporting_wm_check(&EWMH, QX11Info::appRootWindow(), child);
  //Also set this property on the child window (pointing to itself)
  xcb_ewmh_set_supporting_wm_check(&EWMH, child, child);
}

// _NET_VIRTUAL_ROOTS
QList<WId> LXCB::WM_Get_Virtual_Roots(){
  QList<WId> out;
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_virtual_roots_unchecked(&EWMH, QX11Info::appScreen());
  xcb_ewmh_get_windows_reply_t reply;
  if(1==xcb_ewmh_get_virtual_roots_reply(&EWMH, cookie, &reply, NULL) ){
    for(unsigned int i=0; i<reply.windows_len; i++){
      out << reply.windows[i];
    }
  }
  return out;
}

void LXCB::WM_Set_Virtual_Roots(QList<WId> list){
  //Convert to XCB array
  xcb_window_t array[list.length()];
  for(int i=0; i<list.length(); i++){ array[i] = list[i]; }
  //Set the property
  xcb_ewmh_set_virtual_roots(&EWMH, QX11Info::appScreen(), list.length(), array);
}

// _NET_DESKTOP_LAYOUT
//  -- skipped for now - see note in LuminaX11.h

// _NET_SHOWING_DESKTOP
bool LXCB::WM_Get_Showing_Desktop(){
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_showing_desktop_unchecked(&EWMH, QX11Info::appScreen());
  uint32_t reply = 0;
  xcb_ewmh_get_showing_desktop_reply(&EWMH, cookie, &reply, NULL);
  return (reply==1);
}

void LXCB::WM_Set_Showing_Desktop(bool show){
  xcb_ewmh_set_showing_desktop(&EWMH, QX11Info::appScreen(), (show ? 1 : 0) );
}
	
// -- ROOT WINDOW MESSAGES/REQUESTS
// _NET_CLOSE_WINDOW
void LXCB::WM_Request_Close_Window(WId win){
  xcb_ewmh_request_close_window(&EWMH, QX11Info::appScreen(), win, XCB_TIME_CURRENT_TIME, XCB_EWMH_CLIENT_SOURCE_TYPE_OTHER); //user choice to close the window
}

// _NET_MOVERESIZE_WINDOW
void LXCB::WM_Request_MoveResize_Window(WId win, QRect geom, bool fromuser,  LXCB::GRAVITY grav, LXCB::MOVERESIZE_WINDOW_FLAGS flags){
  //Note: The LXCB::GRAVITY enum exactly matches the XCB values (just different names)
  //Convert the flags into the XCB type
  int eflags = 0; //xcb_ewmh_moveresize_window_opt_flags_t
  if(flags.testFlag(LXCB::X)){ eflags = eflags | XCB_EWMH_MOVERESIZE_WINDOW_X; }
  if(flags.testFlag(LXCB::Y)){ eflags = eflags | XCB_EWMH_MOVERESIZE_WINDOW_Y; }
  if(flags.testFlag(LXCB::WIDTH)){ eflags = eflags | XCB_EWMH_MOVERESIZE_WINDOW_WIDTH; }
  if(flags.testFlag(LXCB::HEIGHT)){ eflags = eflags | XCB_EWMH_MOVERESIZE_WINDOW_HEIGHT; }
  
  xcb_ewmh_request_moveresize_window(&EWMH, QX11Info::appScreen(), win, (xcb_gravity_t) grav, \
		(fromuser ? XCB_EWMH_CLIENT_SOURCE_TYPE_OTHER : XCB_EWMH_CLIENT_SOURCE_TYPE_NORMAL), \
		(xcb_ewmh_moveresize_window_opt_flags_t) eflags, geom.x(), geom.y(), geom.width(), geom.height() );
}

// _NET_WM_MOVERESIZE
//  -- skipped for now - see note in LuminaX11.h

// _NET_RESTACK_WINDOW
void LXCB::WM_Request_Restack_Window(WId win, WId sibling, LXCB::STACK_FLAG flag){
  //Note: The STACK_FLAG enum matches the xcb_stack_mode_t enum exactly (just different names)
  xcb_ewmh_request_restack_window(&EWMH, QX11Info::appScreen(), win, sibling, (xcb_stack_mode_t) flag);
}

// _NET_REQUEST_FRAME_EXTENTS
void LXCB::WM_Request_Frame_Extents(WId win){
  xcb_ewmh_request_frame_extents(&EWMH, QX11Info::appScreen(), win);
}
	
// === WINDOW PROPERTIES ===
// _NET_SUPPORTED (Window)
void LXCB::WM_Set_Window_Supported(WId win){
  //NET_WM standards (ICCCM implied - no standard way to list those)
  xcb_atom_t list[] = {};
  xcb_ewmh_set_wm_allowed_actions(&EWMH, win, 0, list);
}

// _NET_WM_NAME
QString LXCB::WM_Get_Name(WId win){
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_name_unchecked(&EWMH, win);
  xcb_ewmh_get_utf8_strings_reply_t reply;
  QString out;
  if(1==xcb_ewmh_get_wm_name_reply(&EWMH, cookie,&reply, NULL) ){
    out = QString::fromUtf8(reply.strings);
  }
  return out;
}
void LXCB::WM_Set_Name(WId win, QString txt){
  xcb_ewmh_set_wm_name(&EWMH, win, txt.length(), txt.toUtf8().data());
}
	
// _NET_WM_VISIBLE_NAME
QString LXCB::WM_Get_Visible_Name(WId win){
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_visible_name_unchecked(&EWMH, win);
  xcb_ewmh_get_utf8_strings_reply_t reply;
  QString out;
  if(1==xcb_ewmh_get_wm_visible_name_reply(&EWMH, cookie,&reply, NULL) ){
    out = QString::fromUtf8(reply.strings);
  }
  return out;	
}
void LXCB::WM_Set_Visible_Name(WId win, QString txt){
  xcb_ewmh_set_wm_visible_name(&EWMH, win, txt.length(), txt.toUtf8().data());
}
	
// _NET_WM_ICON_NAME
QString LXCB::WM_Get_Icon_Name(WId win){
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_icon_name_unchecked(&EWMH, win);
  xcb_ewmh_get_utf8_strings_reply_t reply;
  QString out;
  if(1==xcb_ewmh_get_wm_icon_name_reply(&EWMH, cookie,&reply, NULL) ){
    out = QString::fromUtf8(reply.strings);
  }
  return out;
}
void LXCB::WM_Set_Icon_Name(WId win, QString txt){
  xcb_ewmh_set_wm_icon_name(&EWMH, win, txt.length(), txt.toUtf8().data());
}
	
// _NET_WM_VISIBLE_ICON_NAME
QString LXCB::WM_Get_Visible_Icon_Name(WId win){
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_visible_icon_name_unchecked(&EWMH, win);
  xcb_ewmh_get_utf8_strings_reply_t reply;
  QString out;
  if(1==xcb_ewmh_get_wm_visible_icon_name_reply(&EWMH, cookie,&reply, NULL) ){
    out = QString::fromUtf8(reply.strings);
  }
  return out;
}
void LXCB::WM_Set_Visible_Icon_Name(WId win, QString txt){
  xcb_ewmh_set_wm_visible_icon_name(&EWMH, win, txt.length(), txt.toUtf8().data());	
}
	
// _NET_WM_DESKTOP
int LXCB::WM_Get_Desktop(WId win){
  //returns -1 if window on all desktops
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_desktop_unchecked(&EWMH, win);
  uint32_t num = 0;
  int out = -1;
  if(1==xcb_ewmh_get_wm_desktop_reply(&EWMH, cookie, &num, NULL) ){
    if(num!=0xFFFFFFFF){ out = num; }
  }else{
    //Error in fetching property (not set?)
    // - put it on the current screen
    out = WM_Get_Current_Desktop();
  }
  return out;
}

void LXCB::WM_Set_Desktop(WId win, int num){
  //use -1 to set it for all desktops
  xcb_ewmh_set_wm_desktop(&EWMH, win, (num<0 ? 0xFFFFFFFF : qAbs(num) ) );
}

// _NET_WM_WINDOW_TYPE
QList<LXCB::WINDOWTYPE> LXCB::WM_Get_Window_Type(WId win){
  // Note: This will silently discard any unknown/non-standard window type flags
  // The client should ensure to set at least one standardized type flag per the specifications.
  QList<LXCB::WINDOWTYPE> out;
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_window_type_unchecked(&EWMH, win);
  xcb_ewmh_get_atoms_reply_t reply;
  if(1==xcb_ewmh_get_wm_window_type_reply(&EWMH, cookie, &reply, NULL) ){
    for(unsigned int i=0; i<reply.atoms_len; i++){
      if(reply.atoms[i]==EWMH._NET_WM_WINDOW_TYPE_DESKTOP){ out << LXCB::T_DESKTOP; }
      else if(reply.atoms[i]==EWMH._NET_WM_WINDOW_TYPE_DOCK){ out << LXCB::T_DOCK; }
      else if(reply.atoms[i]==EWMH._NET_WM_WINDOW_TYPE_TOOLBAR){ out << LXCB::T_TOOLBAR; }
      else if(reply.atoms[i]==EWMH._NET_WM_WINDOW_TYPE_MENU){ out << LXCB::T_MENU; }
      else if(reply.atoms[i]==EWMH._NET_WM_WINDOW_TYPE_UTILITY){ out << LXCB::T_UTILITY; }
      else if(reply.atoms[i]==EWMH._NET_WM_WINDOW_TYPE_SPLASH){ out << LXCB::T_SPLASH; }
      else if(reply.atoms[i]==EWMH._NET_WM_WINDOW_TYPE_DIALOG){ out << LXCB::T_DIALOG; }
      else if(reply.atoms[i]==EWMH._NET_WM_WINDOW_TYPE_DROPDOWN_MENU){ out << LXCB::T_DROPDOWN_MENU; }
      else if(reply.atoms[i]==EWMH._NET_WM_WINDOW_TYPE_POPUP_MENU){ out << LXCB::T_POPUP_MENU; }
      else if(reply.atoms[i]==EWMH._NET_WM_WINDOW_TYPE_TOOLTIP){ out << LXCB::T_TOOLTIP; }
      else if(reply.atoms[i]==EWMH._NET_WM_WINDOW_TYPE_NOTIFICATION){ out << LXCB::T_NOTIFICATION; }
      else if(reply.atoms[i]==EWMH._NET_WM_WINDOW_TYPE_COMBO){ out << LXCB::T_COMBO; }
      else if(reply.atoms[i]==EWMH._NET_WM_WINDOW_TYPE_DND){ out << LXCB::T_DND; }
      else if(reply.atoms[i]==EWMH._NET_WM_WINDOW_TYPE_NORMAL){ out << LXCB::T_NORMAL; }
    }
  }
  return out;
}

void LXCB::WM_Set_Window_Type(WId win, QList<LXCB::WINDOWTYPE> list){
  //Convert to the XCB format
  xcb_atom_t array[list.length()];
  for(int i=0; i<list.length(); i++){
    switch(list[i]){
      case LXCB::T_DESKTOP:
	array[i] = EWMH._NET_WM_WINDOW_TYPE_DESKTOP; break;
      case LXCB::T_DOCK:
	array[i] = EWMH._NET_WM_WINDOW_TYPE_DOCK; break;
      case LXCB::T_TOOLBAR:
	array[i] = EWMH._NET_WM_WINDOW_TYPE_TOOLBAR; break;
      case LXCB::T_MENU:
	array[i] = EWMH._NET_WM_WINDOW_TYPE_MENU; break;
      case LXCB::T_UTILITY:
	array[i] = EWMH._NET_WM_WINDOW_TYPE_UTILITY; break;
      case LXCB::T_SPLASH:
	array[i] = EWMH._NET_WM_WINDOW_TYPE_SPLASH; break;
      case LXCB::T_DIALOG:
	array[i] = EWMH._NET_WM_WINDOW_TYPE_DIALOG; break;
      case LXCB::T_DROPDOWN_MENU:
	array[i] = EWMH._NET_WM_WINDOW_TYPE_DROPDOWN_MENU; break;
      case LXCB::T_POPUP_MENU:
	array[i] = EWMH._NET_WM_WINDOW_TYPE_POPUP_MENU; break;
      case LXCB::T_TOOLTIP:
	array[i] = EWMH._NET_WM_WINDOW_TYPE_TOOLTIP; break;
      case LXCB::T_NOTIFICATION:
	array[i] = EWMH._NET_WM_WINDOW_TYPE_NOTIFICATION; break;
      case LXCB::T_COMBO:
	array[i] = EWMH._NET_WM_WINDOW_TYPE_COMBO; break;
      case LXCB::T_DND:
	array[i] = EWMH._NET_WM_WINDOW_TYPE_DND; break;
      default:
	array[i] = EWMH._NET_WM_WINDOW_TYPE_NORMAL;
    }
  }
  //Now set the property
  xcb_ewmh_set_wm_window_type(&EWMH, win, list.length(), array);
}

// _NET_WM_STATE
QList<LXCB::WINDOWSTATE> LXCB::WM_Get_Window_States(WId win){
  QList<LXCB::WINDOWSTATE> out;
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_state_unchecked(&EWMH, win);
  xcb_ewmh_get_atoms_reply_t reply;
  if(1==xcb_ewmh_get_wm_state_reply(&EWMH, cookie, &reply, NULL) ){
    for(unsigned int i=0; i<reply.atoms_len; i++){
      if(reply.atoms[i]==EWMH._NET_WM_STATE_MODAL){ out << LXCB::S_MODAL; }
      else if(reply.atoms[i]==EWMH._NET_WM_STATE_STICKY){ out << LXCB::S_STICKY; }
      else if(reply.atoms[i]==EWMH._NET_WM_STATE_MAXIMIZED_VERT){ out << LXCB::S_MAX_VERT; }
      else if(reply.atoms[i]==EWMH._NET_WM_STATE_MAXIMIZED_HORZ){ out << LXCB::S_MAX_HORZ; }
      else if(reply.atoms[i]==EWMH._NET_WM_STATE_SHADED){ out << LXCB::S_SHADED; }
      else if(reply.atoms[i]==EWMH._NET_WM_STATE_SKIP_TASKBAR){ out << LXCB::S_SKIP_TASKBAR; }
      else if(reply.atoms[i]==EWMH._NET_WM_STATE_SKIP_PAGER){ out << LXCB::S_SKIP_PAGER; }
      else if(reply.atoms[i]==EWMH._NET_WM_STATE_HIDDEN){ out << LXCB::S_HIDDEN; }
      else if(reply.atoms[i]==EWMH._NET_WM_STATE_FULLSCREEN){ out << LXCB::S_FULLSCREEN; }
      else if(reply.atoms[i]==EWMH._NET_WM_STATE_ABOVE){ out << LXCB::S_ABOVE; }
      else if(reply.atoms[i]==EWMH._NET_WM_STATE_BELOW){ out << LXCB::S_BELOW; }
      else if(reply.atoms[i]==EWMH._NET_WM_STATE_DEMANDS_ATTENTION){ out << LXCB::S_ATTENTION; }
      //else if(reply.atoms[i]==EWMH._NET_WM_STATE_FOCUSED){ out << LXCB::FOCUSED; }
    }
  }
  return out;
}

void LXCB::WM_Set_Window_States(WId win, QList<LXCB::WINDOWSTATE> list){
  //Convert to the XCB format
  xcb_atom_t array[list.length()];
  for(int i=0; i<list.length(); i++){
    switch(list[i]){
      case LXCB::S_MODAL:
	array[i] = EWMH._NET_WM_STATE_MODAL; break;
      case LXCB::S_STICKY:
	array[i] = EWMH._NET_WM_STATE_STICKY; break;
      case LXCB::S_MAX_VERT:
	array[i] = EWMH._NET_WM_STATE_MAXIMIZED_VERT; break;
      case LXCB::S_MAX_HORZ:
	array[i] = EWMH._NET_WM_STATE_MAXIMIZED_HORZ; break;
      case LXCB::S_SHADED:
	array[i] = EWMH._NET_WM_STATE_SHADED; break;
      case LXCB::S_SKIP_TASKBAR:
	array[i] = EWMH._NET_WM_STATE_SKIP_TASKBAR; break;
      case LXCB::S_SKIP_PAGER:
	array[i] = EWMH._NET_WM_STATE_SKIP_PAGER; break;
      case LXCB::S_HIDDEN:
	array[i] = EWMH._NET_WM_STATE_HIDDEN; break;
      case LXCB::S_FULLSCREEN:
	array[i] = EWMH._NET_WM_STATE_FULLSCREEN; break;
      case LXCB::S_ABOVE:
	array[i] = EWMH._NET_WM_STATE_ABOVE; break;
      case LXCB::S_BELOW:
	array[i] = EWMH._NET_WM_STATE_BELOW; break;
      case LXCB::S_ATTENTION:
	array[i] = EWMH._NET_WM_STATE_DEMANDS_ATTENTION; break;
      //case LXCB::FOCUSED:
	//array[i] = EWMH._NET_WM_STATE_FOCUSED; break;
    }
  }
  //Now set the property
  xcb_ewmh_set_wm_state(&EWMH, win, list.length(), array);	
}

// _NET_WM_ALLOWED_ACTIONS
QList<LXCB::WINDOWACTION> LXCB::WM_Get_Window_Actions(WId win){
  QList<LXCB::WINDOWACTION> out;
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_allowed_actions_unchecked(&EWMH, win);
  xcb_ewmh_get_atoms_reply_t reply;
  if(1==xcb_ewmh_get_wm_allowed_actions_reply(&EWMH, cookie, &reply, NULL) ){
    for(unsigned int i=0; i<reply.atoms_len; i++){
      if(reply.atoms[i]==EWMH._NET_WM_ACTION_MOVE){ out << LXCB::A_MOVE; }
      else if(reply.atoms[i]==EWMH._NET_WM_ACTION_RESIZE){ out << LXCB::A_RESIZE; }
      else if(reply.atoms[i]==EWMH._NET_WM_ACTION_MINIMIZE){ out << LXCB::A_MINIMIZE; }
      else if(reply.atoms[i]==EWMH._NET_WM_ACTION_SHADE){ out << LXCB::A_SHADE; }
      else if(reply.atoms[i]==EWMH._NET_WM_ACTION_STICK){ out << LXCB::A_STICK; }
      else if(reply.atoms[i]==EWMH._NET_WM_ACTION_MAXIMIZE_HORZ){ out << LXCB::A_MAX_HORZ; }
      else if(reply.atoms[i]==EWMH._NET_WM_ACTION_MAXIMIZE_VERT){ out << LXCB::A_MAX_VERT; }
      else if(reply.atoms[i]==EWMH._NET_WM_ACTION_FULLSCREEN){ out << LXCB::A_FULLSCREEN; }
      else if(reply.atoms[i]==EWMH._NET_WM_ACTION_CHANGE_DESKTOP){ out << LXCB::A_CHANGE_DESKTOP; }
      else if(reply.atoms[i]==EWMH._NET_WM_ACTION_CLOSE){ out << LXCB::A_CLOSE; }
      else if(reply.atoms[i]==EWMH._NET_WM_ACTION_ABOVE){ out << LXCB::A_ABOVE; }
      else if(reply.atoms[i]==EWMH._NET_WM_ACTION_BELOW){ out << LXCB::A_BELOW; }
    }
  }
  return out;	
}

void LXCB::WM_Set_Window_Actions(WId win, QList<LXCB::WINDOWACTION> list){
  //Convert to the XCB format
  xcb_atom_t array[list.length()];
  for(int i=0; i<list.length(); i++){
    switch(list[i]){
      case LXCB::A_MOVE:
	array[i] = EWMH._NET_WM_ACTION_MOVE; break;
      case LXCB::A_RESIZE:
	array[i] = EWMH._NET_WM_ACTION_RESIZE; break;
      case LXCB::A_MINIMIZE:
	array[i] = EWMH._NET_WM_ACTION_MINIMIZE; break;
      case LXCB::A_SHADE:
	array[i] = EWMH._NET_WM_ACTION_SHADE; break;
      case LXCB::A_STICK:
	array[i] = EWMH._NET_WM_ACTION_STICK; break;
      case LXCB::A_MAX_HORZ:
	array[i] = EWMH._NET_WM_ACTION_MAXIMIZE_HORZ; break;
      case LXCB::A_MAX_VERT:
	array[i] = EWMH._NET_WM_ACTION_MAXIMIZE_VERT; break;
      case LXCB::A_FULLSCREEN:
	array[i] = EWMH._NET_WM_ACTION_FULLSCREEN; break;
      case LXCB::A_CHANGE_DESKTOP:
	array[i] = EWMH._NET_WM_ACTION_CHANGE_DESKTOP; break;
      case LXCB::A_CLOSE:
	array[i] = EWMH._NET_WM_ACTION_CLOSE; break;
      case LXCB::A_ABOVE:
	array[i] = EWMH._NET_WM_ACTION_ABOVE; break;
      case LXCB::A_BELOW:
	array[i] = EWMH._NET_WM_ACTION_BELOW; break;
    }
  }
  //Now set the property
  xcb_ewmh_set_wm_allowed_actions(&EWMH, win, list.length(), array);
}

// _NET_WM_STRUT
QList<unsigned int> LXCB::WM_Get_Window_Strut(WId win){
  //Returns: [left,right,top,bottom] margins in pixels (always length 4)
  QList<unsigned int> out; out << 0 << 0 << 0 << 0; //init the output list
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_strut_unchecked(&EWMH, win);
  xcb_ewmh_get_extents_reply_t reply;
  if(1==xcb_ewmh_get_wm_strut_reply(&EWMH, cookie, &reply, NULL) ){
    out[0] = reply.left;
    out[1] = reply.right;
    out[2] = reply.top;
    out[3] = reply.bottom;
  }
  return out;
}

void LXCB::WM_Set_Window_Strut(WId win, QList<unsigned int> margins){
  //Input: [left, right, top, bottom] - must be length 4
  while(margins.length()<4){ margins << 0; }
  xcb_ewmh_set_wm_strut(&EWMH, win, margins[0], margins[1], margins[2], margins[3]);
}

// _NET_WM_STRUT_PARTIAL
QList<strut_geom> LXCB::WM_Get_Window_Strut_Partial(WId win){
  //Returns: [left,right,top,bottom] struts
  QList<strut_geom> out; out << strut_geom() << strut_geom() << strut_geom() << strut_geom();
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_strut_partial_unchecked(&EWMH, win);
  xcb_ewmh_wm_strut_partial_t reply;
  if(1==xcb_ewmh_get_wm_strut_partial_reply(&EWMH, cookie, &reply, NULL) ){
    if(reply.left>0){
      out[0].start = reply.left_start_y; out[0].end = reply.left_end_y; out[0].thick = reply.left;
    }
    if(reply.right>0){
      out[1].start = reply.right_start_y; out[1].end = reply.right_end_y; out[1].thick = reply.right;
    }
    if(reply.top>0){
      out[2].start = reply.top_start_x; out[2].end = reply.top_end_x; out[2].thick = reply.top;
    }
    if(reply.bottom>0){
      out[3].start = reply.bottom_start_x; out[3].end = reply.bottom_end_x; out[3].thick = reply.bottom;
    }
  }
  return out;
}

void LXCB::WM_Set_Window_Strut_Partial(WId win, QList<strut_geom> struts){
  //Input: [left,right,top,bottom] - must be length 4
  while(struts.length() < 4){ struts << strut_geom(); }
  //Convert to the XCB input format
  xcb_ewmh_wm_strut_partial_t input;
  input.left=struts[0].thick; input.left_start_y=struts[0].start; input.left_end_y=struts[0].end;
  input.right=struts[1].thick; input.right_start_y=struts[1].start; input.right_end_y=struts[1].end;
  input.top=struts[2].thick; input.top_start_x=struts[2].start; input.top_end_x=struts[2].end;
  input.bottom=struts[3].thick; input.bottom_start_x=struts[3].start; input.bottom_end_x=struts[3].end;
  //Now set the property
  xcb_ewmh_set_wm_strut_partial(&EWMH, win, input);
}

// _NET_WM_ICON_GEOMETRY
QRect LXCB::WM_Get_Icon_Geometry(WId win){
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_icon_geometry_unchecked(&EWMH, win);
  xcb_ewmh_geometry_t reply;
  QRect out;
  if(1==xcb_ewmh_get_wm_icon_geometry_reply(&EWMH, cookie, &reply, NULL) ){
    out = QRect(reply.x, reply.y, reply.width, reply.height);
  }
  return out;
}

void LXCB::WM_Set_Icon_Geometry(WId win, QRect geom){
  //Note - 11/12/15: xcb_ewmh.h lists the inputs as "left/right/top/bottom"
  //  but this might be an error and the real inputs are "x/y/width/height"
  //  as in the other geometry get/set routines (and as returned by the xcb_ewmh_get_wm_icon_geometry() routine)
  xcb_ewmh_set_wm_icon_geometry(&EWMH, win, geom.x(), geom.x()+geom.width(), geom.y(), geom.y()+geom.height());
  //xcb_ewmh_set_wm_icon_geometry(&EWMH, win, geom.x(), geom.y(), geom.width(), geom.height());
}

// _NET_WM_ICON
QIcon LXCB::WM_Get_Icon(WId win){
  //Note: The output is a QIcon because it allows for multiple varying-sized images to be loaded/used later as needed
  // For each pixmap found here, add it (in its native size) to the icon structure
  QIcon out;
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_icon_unchecked(&EWMH, win);
  xcb_ewmh_get_wm_icon_reply_t reply;
  if(1==xcb_ewmh_get_wm_icon_reply(&EWMH, cookie, &reply, NULL) ){
    //Now iterate over all the pixmaps and load them into the QIcon
    xcb_ewmh_wm_icon_iterator_t it = xcb_ewmh_get_wm_icon_iterator(&reply);
    while(it.index < reply.num_icons){
      QImage img( (const unsigned char *) it.data, it.width, it.height, QImage::Format_ARGB32);
      out.addPixmap( QPixmap::fromImage(img) );
      if(it.rem>0){ xcb_ewmh_get_wm_icon_next(&it); } //go to the next pixmap
      else{ break; } //just finished the last one - ensure this breaks out now (just in case)
    }
    //Clean up any background buffer for the reply
    xcb_ewmh_get_wm_icon_reply_wipe(&reply);
  }
  return out;
}

// _NET_WM_PID
unsigned int LXCB::WM_Get_Pid(WId win){
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_pid_unchecked(&EWMH, win);
  uint32_t pid = 0;
  xcb_ewmh_get_wm_pid_reply(&EWMH, cookie, &pid, NULL);
  return pid;
}

// _NET_WM_HANDLED_ICONS
bool LXCB::WM_Get_Handled_Icons(WId win){
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_handled_icons_unchecked(&EWMH, win);
  uint32_t num = 0;
  xcb_ewmh_get_wm_handled_icons_reply(&EWMH, cookie, &num, NULL);
  return (num!=0); //This flag is set on the window
}

void LXCB::WM_Set_Handled_Icons(WId win, bool set){
  xcb_ewmh_set_wm_handled_icons(&EWMH, win, (set ? 1 : 0));
}

// _NET_WM_USER_TIME
unsigned int LXCB::WM_Get_User_Time(WId win){
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_user_time_unchecked(&EWMH, win);
  uint32_t out = 0;
  xcb_ewmh_get_wm_user_time_reply(&EWMH, cookie, &out, NULL);
  return out;
}

void LXCB::WM_Set_User_Time(WId win, unsigned int xtime){
  xcb_ewmh_set_wm_user_time(&EWMH, win, xtime);
}

// _NET_WM_USER_TIME_WINDOW
/*
WId LXCB::WM_Get_User_Time_WIndow(WId win){
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_user_time_window_unchecked(&EWMH, win);
  xcb_window_t out;
  xcb_ewmh_get_user_time_window_reply(&EWMH, cookie, &out, NULL);
  return out;
}

void LXCB::WM_Set_User_Time_Window(WId win, WId utwin){
  xcb_ewmh_set_wm_user_time_window(&EWMH, win, utwin);
}*/

// _NET_FRAME_EXTENTS
QList<unsigned int> LXCB::WM_Get_Frame_Extents(WId win){
  //Returns: [left,right,top,bottom] margins in pixels (always length 4)
  QList<unsigned int> out; out << 0 << 0 << 0 << 0; //init the output list
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_frame_extents_unchecked(&EWMH, win);
  xcb_ewmh_get_extents_reply_t reply;
  if(1==xcb_ewmh_get_frame_extents_reply(&EWMH, cookie, &reply, NULL) ){
    out[0] = reply.left;
    out[1] = reply.right;
    out[2] = reply.top;
    out[3] = reply.bottom;
  }
  return out;
}

void LXCB::WM_Set_Frame_Extents(WId win, QList<unsigned int> margins){
  //Input: [left, right, top, bottom] - must be length 4
  while(margins.length()<4){ margins << 0; }
  xcb_ewmh_set_frame_extents(&EWMH, win, margins[0], margins[1], margins[2], margins[3]);
}

// _NET_WM_OPAQUE_REGION
	
// _NET_WM_BYPASS_COMPOSITOR
	
// === SPECIAL WM PROTOCOLS (EWMH) ===
// _NET_WM_PING
void LXCB::WM_Send_Ping(WId win){
  xcb_ewmh_send_wm_ping(&EWMH, win, XCB_TIME_CURRENT_TIME);
}

// _NET_WM_SYNC_REQUEST
uint64_t LXCB::WM_Get_Sync_Request_Counter(WId win){
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_sync_request_counter_unchecked(&EWMH, win);
  uint64_t count = 0;
  xcb_ewmh_get_wm_sync_request_counter_reply(&EWMH, cookie, &count, NULL);
  return count;
}

/*void LXCB::WM_Set_Sync_Request_Counter(WId win, uint64_t count){
  
}*/

// _NET_WM_FULLSCREEN_MONITORS
QList<unsigned int> LXCB::WM_Get_Fullscreen_Monitors(WId win){
  //Returns: [top,bottom,left,right] monitor numbers for window to use when fullscreen
  QList<unsigned int> out; out << 0 << 0 << 0 << 0; //init the output array
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_fullscreen_monitors_unchecked(&EWMH, win);
  xcb_ewmh_get_wm_fullscreen_monitors_reply_t reply;
  if(1==xcb_ewmh_get_wm_fullscreen_monitors_reply(&EWMH, cookie, &reply, NULL) ){
    out[0] = reply.top; out[1] = reply.bottom;
    out[2] = reply.left; out[3] = reply.right;
  }
  return out;
}

void LXCB::WM_Set_Fullscreen_Montors(WId win, QList<unsigned int> list){
  //Input: [top,bottom,left,right] monitor numbers
  while(list.length()<4){ list << 0; }
  xcb_ewmh_set_wm_fullscreen_monitors(&EWMH, win, list[0], list[1], list[2], list[3]);
}

// _NET_WM_CM_S(n)
WId LXCB::WM_Get_CM_Owner(){
  xcb_get_selection_owner_cookie_t cookie = xcb_ewmh_get_wm_cm_owner_unchecked(&EWMH, QX11Info::appScreen());
  xcb_window_t owner = 0;
  xcb_ewmh_get_wm_cm_owner_reply(&EWMH, cookie, &owner, NULL);
  return owner;
}

void LXCB::WM_Set_CM_Owner(WId win){
  xcb_ewmh_set_wm_cm_owner(&EWMH, QX11Info::appScreen(), win, XCB_TIME_CURRENT_TIME,0,0);
}
