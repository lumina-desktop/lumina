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

//X includes (these need to be last due to Qt compile issues)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xcomposite.h>

//XCB Library includes
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/xproto.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_image.h>
#include <xcb/composite.h>
#include <xcb/damage.h>


//=====   WindowList() ========
QList<WId> LX11::WindowList(){
  QList<WId> output;
  output << LX11::GetClientList();

  
  //Validate windows
  int desk = LX11::GetCurrentDesktop();
  for(int i=0; i<output.length(); i++){
    bool remove=false;
    QString name = LX11::WindowClass(output[i]);
    if(output[i] == 0){ remove=true; }
    else if( desk >= 0 && LX11::WindowDesktop(output[i]) != desk){ remove = true; }
    else if( name=="Lumina Desktop Environment" ){ remove = true; }
    else if(name.startsWith("Lumina-")){
      //qDebug() << "Lumina Window:" << name << LX11::WindowName(output[i]);
      if(LX11::WindowName(output[i]).toLower()==name.toLower() ){ remove=true; }
    }
    /*else if( name.isEmpty() ){ 
      qDebug() << "Abnormal Window:" << output[i];
	qDebug() << " - Class:" << name;
	qDebug() << " - Text:" << LX11::WindowName(output[i]);
	qDebug() << " - Visible Name:" << LX11::WindowVisibleName(output[i]);
	qDebug() << " - Icon Name:" << LX11::WindowIconName(output[i]);
    }*/
    if(remove){
      //qDebug() << "Skip Window:" << output[i];
      output.removeAt(i);
      i--;
    }
  }
  //qDebug() << output;
  //Return the list
  return output;
}

// ===== GetClientList() =====
QList<WId> LX11::GetClientList(){
  QList<WId> output;
  //XCB Library
  /*qDebug() << "Get Client list cookie";
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_client_list_unchecked( LX11::EWMH_C(), 0);
  xcb_ewmh_get_windows_reply_t winlist;
  qDebug() << "Get client list";
  if( xcb_ewmh_get_client_list_reply( LX11::EWMH_C(), cookie, &winlist, NULL) ){
    qDebug() << " - Loop over items";
    for(unsigned int i=0; i<winlist.windows_len; i++){ output << winlist.windows[i]; }
  }*/
	
  //XLib
  Atom a = XInternAtom(QX11Info::display(), "_NET_CLIENT_LIST", true);
  Atom realType;
  int format;
  unsigned long num, bytes;
  unsigned char *data = 0;
  int status = XGetWindowProperty(QX11Info::display(), QX11Info::appRootWindow(), a, 0L, (~0L),
  	     false, AnyPropertyType, &realType, &format, &num, &bytes, &data);
  if( (status >= Success) && (num > 0) ){
    Window* array =  (Window*) data;
    for(unsigned int i=0; i<num; i++){
       output << array[i];
    }
    XFree(data);
  }
  return output;
}

// ===== GetClientStackingList() =====
QList<WId> LX11::GetClientStackingList(){
  QList<WId> output;
  Atom a = XInternAtom(QX11Info::display(), "_NET_CLIENT_LIST_STACKING", true);
  Atom realType;
  int format;
  unsigned long num, bytes;
  unsigned char *data = 0;
  int status = XGetWindowProperty(QX11Info::display(), QX11Info::appRootWindow(), a, 0L, (~0L),
  	     false, AnyPropertyType, &realType, &format, &num, &bytes, &data);
  if( (status >= Success) && (num > 0) ){
    Window* array =  (Window*) data;
    for(unsigned int i=0; i<num; i++){
       output << array[i];
    }
    XFree(data);
  }
  return output;
}

// ===== findChildren() =====
QList<WId> LX11::findChildren(WId parent, int levels){
  Window rootR, parentR;
  Window *childrenR;
  unsigned int num;
  int stat = XQueryTree(QX11Info::display(), parent, &rootR, &parentR, &childrenR, &num);
  QList<WId> output;
  if(stat != 0 && num > 0){
    for(int i=0; i<int(num); i++){
      output << childrenR[i];
      if(levels > 0){
       output << LX11::findChildren(childrenR[i], levels-1); //call this recursively
      }
    }
    XFree(childrenR);
  }	
  return output;
}

// ===== ActiveWindow() =====
WId LX11::ActiveWindow(){
  Display *disp = QX11Info::display();
  Atom SA = XInternAtom(disp, "_NET_ACTIVE_WINDOW", false);
  Atom type;
  int format;
  unsigned long num, bytes;
  unsigned char *data = 0;
  int status = XGetWindowProperty( disp, QX11Info::appRootWindow() , SA, 0, ~(0L), false, AnyPropertyType,
  	  			&type, &format, &num, &bytes, &data);
  WId window=0;
  if(status >= Success && data){
    Window *array = (Window*) data;
    window = array[0];
    XFree(data);
  }
  return window;  		
}

// ===== SetNumberOfDesktops() =====
void LX11::SetNumberOfDesktops(int number){
  //XCB Library
	
  //XLib
  Display *display = QX11Info::display();
  Window rootWindow = QX11Info::appRootWindow();

  Atom atom = XInternAtom(display, "_NET_NUMBER_OF_DESKTOPS", False);
  XEvent xevent;
  xevent.type                 = ClientMessage;
  xevent.xclient.type         = ClientMessage; 
  xevent.xclient.display      = display;
  xevent.xclient.window       = rootWindow;
  xevent.xclient.message_type = atom;
  xevent.xclient.format       = 32;
  xevent.xclient.data.l[0]    = number;
  xevent.xclient.data.l[1]    = CurrentTime;
  xevent.xclient.data.l[2]    = 0;
  xevent.xclient.data.l[3]    = 0;
  xevent.xclient.data.l[4]    = 0;
  XSendEvent(display, rootWindow, False, SubstructureNotifyMask | SubstructureRedirectMask, &xevent);

  XFlush(display);	
}

// ===== SetCurrentDesktop() =====
void LX11::SetCurrentDesktop(int number){
  Display *display = QX11Info::display();
  Window rootWindow = QX11Info::appRootWindow();

  Atom atom = XInternAtom(display, "_NET_CURRENT_DESKTOP", False);
  XEvent xevent;
  xevent.type                 = ClientMessage;
  xevent.xclient.type         = ClientMessage; 
  xevent.xclient.display      = display;
  xevent.xclient.window       = rootWindow;
  xevent.xclient.message_type = atom;
  xevent.xclient.format       = 32;
  xevent.xclient.data.l[0]    = number;
  xevent.xclient.data.l[1]    = CurrentTime;
  xevent.xclient.data.l[2]    = 0;
  xevent.xclient.data.l[3]    = 0;
  xevent.xclient.data.l[4]    = 0;
  XSendEvent(display, rootWindow, False, SubstructureNotifyMask | SubstructureRedirectMask, &xevent);

  XFlush(display);	
}

// ===== GetNumberOfDesktops() =====
int LX11::GetNumberOfDesktops(){
  int number = -1;
  Atom a = XInternAtom(QX11Info::display(), "_NET_NUMBER_OF_DESKTOPS", true);
  Atom realType;
  int format;
  unsigned long num, bytes;
  unsigned char *data = 0;
  int status = XGetWindowProperty(QX11Info::display(), QX11Info::appRootWindow(), a, 0L, (~0L),
             false, AnyPropertyType, &realType, &format, &num, &bytes, &data);
  if( (status >= Success) && (num > 0) ){
    number = *data;
    XFree(data);
  }
  return number;
}

// ===== GetCurrentDesktop =====
int LX11::GetCurrentDesktop(){
  int number = -1;
  Atom a = XInternAtom(QX11Info::display(), "_NET_CURRENT_DESKTOP", true);
  Atom realType;
  int format;
  unsigned long num, bytes;
  unsigned char *data = 0;
  int status = XGetWindowProperty(QX11Info::display(), QX11Info::appRootWindow(), a, 0L, (~0L),
             false, AnyPropertyType, &realType, &format, &num, &bytes, &data);
  if( (status >= Success) && (num > 0) ){
    number = data[0];
    XFree(data);
  }
  return number;	
}

// ===== ValidWindowEvent() =====
/*bool LX11::ValidWindowEvent(Atom evAtom){
	
  if(evAtom == XInternAtom(QX11Info::display(),"_NET_CLIENT_LIST",false) ){ return true; }
  else if( evAtom == XInternAtom(QX11Info::display(),"_NET_ACTIVE_WINDOW",false) ){ return true; }
  else if( evAtom == XInternAtom(QX11Info::display(),"_NET_WM_NAME",false) ){ return true; }
  else if( evAtom == XInternAtom(QX11Info::display(),"_NET_WM_VISIBLE_NAME",false) ){ return true; }
  else if( evAtom == XInternAtom(QX11Info::display(),"_NET_WM_ICON_NAME",false) ){ return true; }
  else if( evAtom == XInternAtom(QX11Info::display(),"_NET_WM_VISIBLE_ICON_NAME",false) ){ return true; }
  else{ return false; }
}*/

// ===== CloseWindow() =====
void LX11::CloseWindow(WId win){
  Display *display = QX11Info::display();
  Window rootWindow = QX11Info::appRootWindow();

  Atom atom = XInternAtom(display, "_NET_CLOSE_WINDOW", False);
  XEvent xevent;
  xevent.type                 = ClientMessage;
  xevent.xclient.type         = ClientMessage; 
  xevent.xclient.display      = display;
  xevent.xclient.window       = win;
  xevent.xclient.message_type = atom;
  xevent.xclient.format       = 32;
  xevent.xclient.data.l[0]    = CurrentTime;
  xevent.xclient.data.l[1]    = 2;
  xevent.xclient.data.l[2]    = 0;
  xevent.xclient.data.l[3]    = 0;
  xevent.xclient.data.l[4]    = 0;
  XSendEvent(display, rootWindow, False, SubstructureNotifyMask | SubstructureRedirectMask, &xevent);

  XFlush(display);
}

void LX11::KillWindow(WId win){
  XKillClient(QX11Info::display(),win);
}

// ===== IconifyWindow() =====
void LX11::IconifyWindow(WId win){
  XIconifyWindow(QX11Info::display(), win, QX11Info::appScreen());	
}

// ===== RestoreWindow() =====
void LX11::RestoreWindow(WId win){
  //XCB Library
    uint32_t val = XCB_STACK_MODE_ABOVE;
    xcb_configure_window(QX11Info::connection(),  win, XCB_CONFIG_WINDOW_STACK_MODE, &val); //raise it
    xcb_map_window(QX11Info::connection(), win); //map it
  //XLib
  //Display *disp = QX11Info::display();
    //XMapRaised(disp, win); //make it visible again and raise it to the top
}

// ===== ActivateWindow() =====
void LX11::ActivateWindow(WId win){
  Display *display = QX11Info::display();
  Window rootWindow = QX11Info::appRootWindow();

  Atom atom = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
  XEvent xevent;
  xevent.type                 = ClientMessage;
  xevent.xclient.type         = ClientMessage; 
  xevent.xclient.display      = display;
  xevent.xclient.window       = win;
  xevent.xclient.message_type = atom;
  xevent.xclient.format       = 32;
  xevent.xclient.data.l[0]    = 2;
  xevent.xclient.data.l[1]    = CurrentTime;
  xevent.xclient.data.l[2]    = LX11::ActiveWindow();
  xevent.xclient.data.l[3]    = 0;
  xevent.xclient.data.l[4]    = 0;
  XSendEvent(display, rootWindow, False, SubstructureNotifyMask | SubstructureRedirectMask, &xevent);

  XFlush(display);
}

// ===== ReservePanelLocation() =====
void LX11::ReservePanelLocation(WId win, int xstart, int ystart, int width, int height, QString loc){
  unsigned long strut[12];
  for(int i=0; i<12; i++){ strut[i] = 0; } //initialize it to all zeros
  if(loc=="top"){
    //top of screen
    strut[2] = height; //top width
    strut[8] = xstart; //top x start
    strut[9] = xstart+width; //top x end
  }else if(loc=="bottom"){
    //bottom of screen
    strut[3] = height; //bottom width
    strut[10] = xstart; //bottom x start
    strut[11] = xstart+width; //bottom x end
  }else if(loc=="left"){
    strut[0] = width;
    strut[4]=ystart;
    strut[5]=ystart+height;
  }else{ //right
    strut[1] = width;
    strut[6]=ystart;
    strut[7]=ystart+height;	  
  }
  Display *disp = QX11Info::display();
  Atom WTYPE = XInternAtom(disp, "_NET_WM_STRUT_PARTIAL", false);
  XChangeProperty( disp, win, WTYPE, XA_CARDINAL, 32, PropModeReplace, (unsigned char *) strut, 12);
  
  //Also set the _NET_WM_STRUT property, just in case the WM does not use the newer type
  WTYPE = XInternAtom(disp, "_NET_WM_STRUT", false);
  XChangeProperty( disp, win, WTYPE, XA_CARDINAL, 32, PropModeReplace, (unsigned char *) strut, 4);
}

// ===== SetAsSticky() =====
void LX11::SetAsSticky(WId win){
  //make this window "stick" to all virtual desktops
	
  //XCB Library
 // xcb_change_property(QX11Info::connection(), XCB_PROP_MODE_APPEND, win, _NET_WM_STATE, XCB_ATOM, 32, 1, _NET_WM_STATE_STICKY);
	
  //XLib
  Display *disp = QX11Info::display();
  Atom stick = XInternAtom(disp, "_NET_WM_STATE_STICKY",false);
  Atom state = XInternAtom(disp, "_NET_WM_STATE", false);
  
  XChangeProperty(disp, win, state, XA_ATOM, 32, PropModeAppend, (unsigned char*) &stick, 1L);
}

// ===== SetAsPanel() =====
void LX11::SetAsPanel(WId win){
  //Set this window as the "Dock" type (for showing on top of everthing else)
  long data[1];
  Display *disp = QX11Info::display();
  Atom WTYPE = XInternAtom(disp, "_NET_WM_WINDOW_TYPE", false);
  Atom DOCK = XInternAtom(disp, "_NET_WM_WINDOW_TYPE_DOCK",false);
  data[0] = DOCK;
  XChangeProperty( disp, win, WTYPE, XA_ATOM, 32, PropModeReplace, (unsigned char *) &data, 1L);
}

// ===== SetAsDesktop() =====
void LX11::SetAsDesktop(WId win){
  //Set this window as the "Desktop" type (for showing below everthing else)
  long data[1];
  Display *disp = QX11Info::display();
  Atom WTYPE = XInternAtom(disp, "_NET_WM_WINDOW_TYPE", false);
  Atom DOCK = XInternAtom(disp, "_NET_WM_WINDOW_TYPE_DESKTOP",false);
  data[0] = DOCK;
  XChangeProperty( disp, win, WTYPE, XA_ATOM, 32, PropModeReplace, (unsigned char *) &data, 1L);
}

// ===== MoveResizeWindow() =====
void LX11::MoveResizeWindow(WId win, QRect rect){
  //Note: rect needs to be in global coordinates!!
  XMoveResizeWindow(QX11Info::display(), win, rect.x(), rect.y(), rect.width(), rect.height());
}

// ===== ResizeWindow() =====
void LX11::ResizeWindow(WId win, int width, int height){
  XResizeWindow(QX11Info::display(), win, width, height);
}

// ===== CreateWindow() =====
WId LX11::CreateWindow(WId parent, QRect rect){
  if(parent==0){ parent = QX11Info::appRootWindow(); }
  XWindowAttributes patt;
  XSetWindowAttributes att;
    att.background_pixel=0;
    att.border_pixel=0;
  //Try to set the same attributes as the parent
  if( ! XGetWindowAttributes(QX11Info::display(), parent, &patt) ){
    //Use some simple defaults instead
    att.colormap = None;
  }else{
    att.colormap = patt.colormap;
  }
  return XCreateWindow( QX11Info::display(), parent, rect.x(), rect.y(), rect.width(), rect.height(), 0, 
		CopyFromParent, InputOutput, CopyFromParent, CWColormap|CWBackPixel|CWBorderPixel, &att);
}

// ===== DestroyWindow() =====
void LX11::DestroyWindow(WId win){
  XDestroyWindow( QX11Info::display(), win);
}

// ===== EmbedWindow() =====
bool LX11::EmbedWindow(WId win, WId container){
  Display *disp = QX11Info::display();
  if(win==0 || container==0){ return false; }
  //Reparent the window
  //XCompositeRedirectSubwindows(disp, container, CompositeRedirectAutomatic); //container/window should be aware of each other
  //qDebug() << "Embed Window:" << win << container;
  XReparentWindow(disp, win, container,0,0);
  XSync(disp, false);
  //Map the window
  XMapRaised(disp, win); //make it visible again and raise it to the top
  //Check that the window has _XEMBED_INFO
  //qDebug() << " - check for _XEMBED_INFO";
  Atom embinfo = XInternAtom(disp, "_XEMBED_INFO",false);
  uchar *data=0;
  ulong num, bytes;
  int fmt;
  Atom junk;
  if(Success != XGetWindowProperty(disp, win, embinfo, 0, 2, false, embinfo, &junk, &fmt, &num, &bytes, &data) ){
    return false; //Embedding error (no info?)
  }
  if(data){ XFree(data); } // clean up any data found

  //Now send the embed event to the app
  //qDebug() << " - send _XEMBED event";
  XEvent ev;
	ev.xclient.type=ClientMessage;
	ev.xclient.serial=0;
	ev.xclient.send_event=true;
	ev.xclient.message_type = XInternAtom(disp, "_XEMBED", false);
	ev.xclient.window = win;
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = CurrentTime;
	ev.xclient.data.l[1] = 0; //XEMBED_EMBEDDED_NOTIFY
	ev.xclient.data.l[2] = 0;
	ev.xclient.data.l[3] = container;
	ev.xclient.data.l[4] = 0;
  XSendEvent(disp, win, false, 0xFFFFFF, &ev);
  //Now setup any redirects and return
  //qDebug() << " - select Input";
  XSelectInput(disp, win, StructureNotifyMask); //Notify of structure changes
  //qDebug() << " - Composite Redirect";
  XCompositeRedirectWindow(disp, win, CompositeRedirectManual);
  //qDebug() << " - Done";
  return true;
}

// ===== UnembedWindow() =====
bool LX11::UnembedWindow(WId win){
  Display *disp = QX11Info::display();
  //Remove redirects
  XSelectInput(disp, win, NoEventMask);
  //Make sure it is invisible
  XUnmapWindow(disp, win);
  //Reparent the window back to the root window
  XReparentWindow(disp, win, QX11Info::appRootWindow(),0,0);
  XSync(disp, false);
  return true;
}

// ===== WindowClass() =====
QString LX11::WindowClass(WId win){
  XClassHint hint;
  QString clss;
  if(0 != XGetClassHint(QX11Info::display(), win, &hint) ){
    clss = QString(hint.res_class); //class is often capitalized properly, while name is not
    XFree(hint.res_name);
    XFree(hint.res_class);
  }
  return clss;
}

// ===== WindowName() =====
QString LX11::WindowName(WId win){
  QString nm = LX11::getNetWMProp(win, "_NET_WM_NAME");
  if(nm.isEmpty()){ 
    char *txt;
    if( XFetchName(QX11Info::display(), win, &txt) != 0){
      nm = QString(txt);
    }
    XFree(txt);
  }
  return nm;
}

// ===== WindowVisibleName() =====
QString LX11::WindowVisibleName(WId win){
  return LX11::getNetWMProp(win, "_NET_WM_VISIBLE_NAME");	
}

// ===== WindowIconName() =====
QString LX11::WindowIconName(WId win){
  return LX11::getNetWMProp(win, "_NET_WM_ICON_NAME");	
}

// ===== WindowVisibleIconName() =====
QString LX11::WindowVisibleIconName(WId win){
  return LX11::getNetWMProp(win, "_NET_WM_VISIBLE_ICON_NAME");	
}

// ===== WindowIcon() =====
/*QIcon LX11::WindowIcon(WId win){
  //Use the _NET_WM_ICON value instead of the WMHints pixmaps
	// - the pixmaps are very unstable and erratic
  QIcon icon;
  Display *disp = QX11Info::display();
  Atom type;
  Atom SA = XInternAtom(disp, "_NET_WM_ICON", false);
  int format;
  unsigned long num, bytes;
  unsigned long *data = 0;
  XGetWindowProperty( disp, win, SA, 0, LONG_MAX, False, AnyPropertyType,
  	  			&type, &format, &num, &bytes, (uchar**)&data);
  if(data != 0){
    //qDebug() << "Icon Data Found:" << win;
    ulong* dat = data;
    while(dat < data+num){ //consider the fact that there may be multiple graphical layers
    //Now convert it into a Qt image
    // - first 2 elements are width and height
    // - data in rows from left to right and top to bottom
      QImage image(dat[0], dat[1], QImage::Format_ARGB32); //initial setup
	dat+=2; //remember the first 2 element offset
	for(int i=0; i<image.byteCount()/4; ++i, ++dat){
	  ((uint*)image.bits())[i] = *dat; 
	}
      icon.addPixmap(QPixmap::fromImage(image)); //layer this pixmap onto the icon
    }
    XFree(data);
  }
  return icon;
}*/


// ===== WindowImage() =====
/*QPixmap LX11::WindowImage(WId win, bool useleader){
  QPixmap pix;
  Display *disp = QX11Info::display();
  WId leader = LX11::leaderWindow(win); //check for an alternate window that contains the image
  if(leader!=0 && useleader){ win = leader; } //use the leader window instead
  //First get the size of the window image (embedded in the window attributes)
  XWindowAttributes att; 
  if( 0 == XGetWindowAttributes(disp, win, &att) ){ return pix; } //invalid window attributes
  //Now extract the image
  XImage *xim = XGetImage(disp, win, 0,0, att.width, att.height, AllPlanes, ZPixmap);
  if(xim!=0){
    //Convert the X image to a Qt Image
    pix.convertFromImage( QImage( (const uchar*) xim->data, xim->width, xim->height, xim->bytes_per_line, QImage::Format_ARGB32_Premultiplied) );
    XDestroyImage(xim); //clean up
  }
  //Return the pixmap
  return pix;
}*/

// ===== GetNumberOfDesktops() =====
int LX11::WindowDesktop(WId win){
  int number = -1;
  Atom a = XInternAtom(QX11Info::display(), "_NET_WM_DESKTOP", true);
  Atom realType;
  int format;
  unsigned long num, bytes;
  unsigned char *data = 0;
  int status = XGetWindowProperty(QX11Info::display(), win, a, 0L, (~0L),
             false, AnyPropertyType, &realType, &format, &num, &bytes, &data);
  if( (status >= Success) && (num > 0) ){
    number = *data;
    XFree(data);
  }
  return number;
}

// ===== GetWindowState() =====
LX11::WINDOWSTATE LX11::GetWindowState(WId win){
  LX11::WINDOWSTATE state = LX11::VISIBLE;
	
  //XCB Library (TO DO)

  
  //XLib
  Display *disp = QX11Info::display(); 
  Atom SA = XInternAtom(disp, "_NET_WM_STATE", true);
  Atom ATTENTION = XInternAtom(disp, "_NET_WM_STATE_DEMANDS_ATTENTION", false);
  Atom SKIPP = XInternAtom(disp, "_NET_WM_STATE_SKIP_PAGER", false);
  Atom HIDDEN = XInternAtom(disp, "_NET_WM_STATE_HIDDEN", false);
  Atom SKIPT = XInternAtom(disp, "_NET_WM_STATE_SKIP_TASKBAR", false);
  //Atom MODAL = XInternAtom(disp, "_NET_WM_STATE_MODAL", false); 
  Atom type;
  int format;
  unsigned long num, bytes;
  unsigned long *data = 0;
  
  int status = XGetWindowProperty( disp, win, SA, 0, ~(0L), false, AnyPropertyType,
  	  			&type, &format, &num, &bytes, (unsigned char**) &data);

  if(status >= Success && data){
    for(unsigned int i=0; i<num; i++){
      if(data[i] == SKIPP || data[i]==SKIPT){
      	state = LX11::IGNORE;
	//qDebug() << "Ignore Window:" << win;
      	break;
      }else if(data[i]==HIDDEN){
	//qDebug() << "Hidden Window:" << win;
	state = LX11::INVISIBLE;
      }else if(data[i]==ATTENTION){
	//qDebug() << "Attention Window: " << win;
	state = LX11::ATTENTION;
	break; //This state has priority over others
      }
    }
    XFree(data);
  }
  
  //LX11::WINDOWSTATE state = LX11::VISIBLE;
  if(state==LX11::VISIBLE){
    XWindowAttributes attr;
    if( 0 != XGetWindowAttributes(disp, win, &attr) ){
      if(attr.map_state==IsUnmapped || attr.map_state==IsUnviewable){
	state = LX11::INVISIBLE;
      }
    }
  }
  //If visible, check whether it is the active window
  if(state == LX11::VISIBLE){
    if(win == LX11::ActiveWindow()){
      state = LX11::ACTIVE;
    }	    
  }
  //(ALTERNATE) Also check whether the window has the URGENT flag set (override all other states)
  if(state!= LX11::ATTENTION){
    XWMHints *hints = XGetWMHints(disp, win);
    if(hints!=0){
      if(hints->flags & URGENCYHINT){
        qDebug() <<  "Found Urgent Flag:";
        state = LX11::ATTENTION;
      }
      XFree(hints);
    }
  }
  
  return state;  	
}

WId LX11::leaderWindow(WId win){
  //Get the client leader for this window if it has one
  Display *disp = QX11Info::display();
  Atom SA = XInternAtom(disp, "WM_CLIENT_LEADER", false);
  Atom type;
  int format;
  unsigned long num, bytes;
  unsigned char *data = 0;
  WId leader = 0;
  int status = XGetWindowProperty( disp, win, SA, 0, ~(0L), false, AnyPropertyType,
  	  			&type, &format, &num, &bytes, &data);
  if(status >= Success && data){
    Window *array = reinterpret_cast<Window*> (data);
    if(array!=0){
      leader = array[0];
    }
    XFree(data);
  }
  return leader;
}

// ===== isNormalWindow() =====
bool LX11::isNormalWindow(WId win, bool includeDialogs){
  //Check to see if it is a "normal" window (as opposed to tooltips, popups, menus, etc)
  Display *disp = QX11Info::display();
  Atom SA = XInternAtom(disp, "_NET_WM_WINDOW_TYPE", false);
  Atom NORMAL = XInternAtom(disp, "_NET_WM_WINDOW_TYPE_NORMAL", false);
  Atom DIALOG = XInternAtom(disp, "_NET_WM_WINDOW_TYPE_DIALOG", false);
  Atom type;
  int format;
  unsigned long num, bytes;
  unsigned char *data = 0;
  int status = XGetWindowProperty( disp, win, SA, 0, ~(0L), false, AnyPropertyType,
  	  			&type, &format, &num, &bytes, &data);
	
  bool isNormal = true; //assume normal is true if unlisted (the standard use)
  if(status >= Success && data){
    for(unsigned int i=0; i<num; i++){
      if( data[i] == NORMAL ){ isNormal = true; break; }
      else if(data[i]==DIALOG && includeDialogs){ isNormal=true; break; }
      else{ isNormal = false; } //don't break here, might be explicitly listed next
    }
    XFree(data);
  }
  return isNormal;
}

// ===== startSystemTray() =====
WId LX11::startSystemTray(int screen){
  qDebug() << "Starting System Tray:" << screen;
  //Setup the freedesktop standards compliance
  Display *disp = QX11Info::display();
  Window root = QX11Info::appRootWindow();
  
  //Get the appropriate atom for this screen
  QString str = QString("_NET_SYSTEM_TRAY_S%1").arg(QString::number(screen));
  //qDebug() << "Default Screen Atom Name:" << str;
  Atom _NET_SYSTEM_TRAY_S = XInternAtom(disp,str.toLatin1(),false);
  //Make sure that there is no other system tray running
  if(XGetSelectionOwner(disp, _NET_SYSTEM_TRAY_S) != None){
    qWarning() << " - An alternate system tray is currently in use";
    return 0;
  }
  //Create a simple window to register as the tray (not visible - just off the screen)
  Window LuminaSessionTrayID = XCreateSimpleWindow(disp, root,-1,-1,1,1,0,0,0);
  //register this widget as the system tray
  XSetSelectionOwner(disp, _NET_SYSTEM_TRAY_S, LuminaSessionTrayID, CurrentTime);
  //Make sure that it was registered properly
  if(XGetSelectionOwner(disp, _NET_SYSTEM_TRAY_S) != LuminaSessionTrayID){
    qWarning() << " - Could not register the system tray";
    XDestroyWindow(disp, LuminaSessionTrayID);
    return 0;
  }
  //Now register the orientation of the system tray
  int horz = _NET_SYSTEM_TRAY_ORIENTATION_HORZ;
  XChangeProperty(disp, LuminaSessionTrayID, XInternAtom(disp,"_NET_SYSTEM_TRAY_ORIENTATION",False),
  	  	XA_CARDINAL, 32, PropModeReplace, (unsigned char*) &horz, 1);
  //Now get the visual ID for the system tray
  XVisualInfo *XVI = new XVisualInfo;
    XVI->screen = QX11Info::appScreen();
    XVI->depth = 32;
    XVI->c_class = TrueColor;
  int num;
  XVI = XGetVisualInfo(disp, VisualScreenMask | VisualDepthMask | VisualClassMask , XVI, &num);
  VisualID vis = 0;
  if(XVI != 0){
    XRenderPictFormat *fmt;
    for(int i=0; i<num; i++){
      fmt = XRenderFindVisualFormat(disp, XVI[i].visual);
      if( (fmt !=0) && (fmt->type == PictTypeDirect) && (fmt->direct.alphaMask!=0) ){
        vis = XVI[i].visualid;
        break;
      }
    }
  }
  XFree(XVI); //done with this - clean it up
  //Now register the visual ID
  if(vis!=0){
    XChangeProperty(disp, LuminaSessionTrayID, XInternAtom(disp,"_NET_SYSTEM_TRAY_VISUAL",true),
  	  	XA_VISUALID, 32, PropModeReplace, (unsigned char*) &vis, 1);	  
  }
  //Finally, send out an X event letting others know that the system tray is up and running
  XClientMessageEvent msg;
    msg.type = ClientMessage;
    msg.window = root;
    msg.message_type = XInternAtom(disp,"MANAGER",true);
    msg.format = 32;
    msg.data.l[0] = CurrentTime;
    msg.data.l[1] = _NET_SYSTEM_TRAY_S;
    msg.data.l[2] = LuminaSessionTrayID;
    msg.data.l[3] = 0;
    msg.data.l[4] = 0;
  XSendEvent(disp, root, False, StructureNotifyMask, (XEvent*)&msg);
  
  //Success
  return LuminaSessionTrayID;
}

// ===== closeSystemTray() =====
void LX11::closeSystemTray(WId trayID){
  XDestroyWindow(QX11Info::display(), trayID);
}

// ===== findOrphanTrayWindows() =====
QList<WId> LX11::findOrphanTrayWindows(){
  //Scan the first level of root windows and see if any of them
    // are tray apps waiting to be embedded
  Display *disp = QX11Info::display();
  QList<WId> wins = LX11::findChildren(QX11Info::appRootWindow(), 0); //only go one level deep
  Atom embinfo = XInternAtom(disp, "_XEMBED_INFO",false);	
  for(int i=0; i<wins.length(); i++){
    uchar *data=0;
    ulong num, bytes;
    int fmt;
    Atom junk;
    bool ok = (Success != XGetWindowProperty(disp, wins[i], embinfo, 0, 2, false, embinfo, &junk, &fmt, &num, &bytes, &data) );
    if(ok){ //successfully found info
      ok = (data!=0);
    }
    
    if(!ok){
      //no embed info - not a tray app
      qDebug() << "Remove non-xembed window:" << wins[i];
      wins.removeAt(i);
      i--;
    }
    if(data){ XFree(data); } // clean up any data found
  }
  return wins; //anything left in the list must be a tray app that is still unembedded (root window parent)
}

// ===== getNetWMProp() =====
QString LX11::getNetWMProp(WId win, QString prop){
  Display *disp = QX11Info::display();
  Atom NA = XInternAtom(disp, prop.toUtf8(), false);
  Atom utf = XInternAtom(disp, "UTF8_STRING", false);
  Atom type;
  int format;
  unsigned long num, bytes;
  unsigned char *data = 0;
  int status = XGetWindowProperty( disp, win, NA, 0, 65536, false, utf,
  	  			&type, &format, &num, &bytes, &data);
  QString property;
  if(status >= Success && data){
    property = QString::fromUtf8( (char *) data);
    XFree(data);
  }
  return property;
}

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
  //qDebug() << "Get Current Workspace";
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_current_desktop_unchecked(&EWMH, 0);
  uint32_t wkspace = 0;
  xcb_ewmh_get_current_desktop_reply(&EWMH, cookie, &wkspace, NULL);
  //qDebug() << " - done:" << wkspace;
  return wkspace;
}

// === ActiveWindow() ===
WId LXCB::ActiveWindow(){
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_active_window_unchecked(&EWMH, 0);
  xcb_window_t actwin;
  if(1 == xcb_ewmh_get_active_window_reply(&EWMH, cookie, &actwin, NULL) ){
    return actwin;
  }else{
    return 0; //invalid ID/failure
  }
}

// === RegisterVirtualRoots() ===
void LXCB::RegisterVirtualRoots(QList<WId> roots){
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

// === WindowClass() ===
QString LXCB::WindowClass(WId win){
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

QList<int> LXCB::WindowFrameGeometry(WId win){
  //Returns: [top, bottom, left, right] sizes for the frame
  QList<int> geom;
  xcb_get_property_cookie_t cookie = xcb_ewmh_get_frame_extents_unchecked(&EWMH, win);
  if(cookie.sequence != 0){
    xcb_ewmh_get_extents_reply_t frame;
    if(1== xcb_ewmh_get_frame_extents_reply(&EWMH, cookie, &frame, NULL) ){
      //adjust the origin point to account for the frame
      geom << frame.top << frame.bottom << frame.left << frame.right;
    }
  }
  if(geom.isEmpty()){ geom << 0 << 0 << 0 << 0; }
  return geom;
}

// === WindowState() ===
LXCB::WINDOWSTATE LXCB::WindowState(WId win){
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

// === WindowIcon() ===
QIcon LXCB::WindowIcon(WId win){
  //Fetch the _NET_WM_ICON for the window and return it as a QIcon
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

// === SetAsSticky() ===
void LXCB::SetAsSticky(WId win){
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
  //This disables all the various control that a WM allows for the window (except for allowing the "Sticky" state)
  xcb_atom_t list[1];
    list[0] = EWMH._NET_WM_ACTION_STICK;
  xcb_ewmh_set_wm_allowed_actions(&EWMH, win, 1, list);
}

// === SetAsPanel() ===
void LXCB::SetAsPanel(WId win){
  if(win==0){ return; }
  SetDisableWMActions(win); //also need to disable WM actions for this window
  //Disable Input focus (panel activation ruins task manager window detection routines)
  //  - Disable Input flag in WM_HINTS
  xcb_icccm_wm_hints_t hints;
  qDebug() << " - Disable WM_HINTS input flag";
  xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_hints_unchecked(QX11Info::connection(), win);
  qDebug() << " -- got cookie";
  if(1 == xcb_icccm_get_wm_hints_reply(QX11Info::connection(), cookie, &hints, NULL) ){
    qDebug() << " -- Set no inputs flag";
     xcb_icccm_wm_hints_set_input(&hints, False); //set no input focus
     xcb_icccm_set_wm_hints(QX11Info::connection(), win, &hints); //save hints back to window
  }
  //  - Remove WM_TAKE_FOCUS from the WM_PROTOCOLS for the window
  //  - - Generate the necessary atoms
  qDebug() << " - Generate WM_PROTOCOLS and WM_TAKE_FOCUS atoms";
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
    qDebug() << " -- success";
  }
  //  - - Now update the protocols for the window
  if(gotatoms){ //requires the atoms
    qDebug() << " - Get WM_PROTOCOLS";
    xcb_icccm_get_wm_protocols_reply_t proto;
    if( 1 == xcb_icccm_get_wm_protocols_reply(QX11Info::connection(), \
			xcb_icccm_get_wm_protocols_unchecked(QX11Info::connection(), win, WM_PROTOCOLS), \
			&proto, NULL) ){
	
      //Found the current protocols, see if it has the focus atom set
			//remove the take focus atom and re-save them
      bool needremove = false;
      //Note: This first loop is required so that we can initialize the modified list with a valid size
      qDebug() << " -- Check current protocols";
      for(unsigned int i=0; i<proto.atoms_len; i++){
        if(proto.atoms[i] == WM_TAKE_FOCUS){ needremove = true; break;}
      }
      if(needremove){
	qDebug() << " -- Remove WM_TAKE_FOCUS protocol";
	xcb_atom_t *protolist = new xcb_atom_t[proto.atoms_len-1];
	int num = 0;
	for(unsigned int i=0; i<proto.atoms_len; i++){
	  if(proto.atoms[i] != WM_TAKE_FOCUS){
	    protolist[num] = proto.atoms[i];
	    num++;
	  }
	}
	qDebug() << " -- Re-save modified protocols";
	xcb_icccm_set_wm_protocols(QX11Info::connection(), win, WM_PROTOCOLS, num, protolist);
      }
      qDebug() << " -- Clear protocols reply";
      xcb_icccm_get_wm_protocols_reply_wipe(&proto);
    }//end of get protocols check
  } //end of gotatoms check
  //Make sure it has the "dock" window type
  //  - get the current window types (Not necessary, only 1 type of window needed)
  
  //  - set the adjusted window type(s)
  qDebug() << " - Adjust window type";
  xcb_atom_t list[1]; 
    list[0] = EWMH._NET_WM_WINDOW_TYPE_DOCK;
  xcb_ewmh_set_wm_window_type(&EWMH, win, 1, list);
  
  //Make sure it is on all workspaces
  qDebug() << " - Set window as sticky";
  SetAsSticky(win);
	
}

void LXCB::SetAsDesktop(WId win){
  if(win==0){ return; }
  SetDisableWMActions(win); //also need to disable WM actions for this window
  xcb_atom_t list[1];
    list[0] = EWMH._NET_WM_WINDOW_TYPE_DESKTOP;
  xcb_ewmh_set_wm_window_type(&EWMH, win, 1, list);
}

// === CloseWindow() ===
void LXCB::CloseWindow(WId win){
  if(win==0){ return; }
  //This will close the specified window (might not close the entire application)
  xcb_ewmh_request_close_window(&EWMH, 0, win, QX11Info::getTimestamp(), XCB_EWMH_CLIENT_SOURCE_TYPE_OTHER);
}

void LXCB::KillClient(WId win){
  if(win==0){ return; }
  //This will forcibly close the application which created WIN 
  xcb_kill_client(QX11Info::connection(), win);
}
// === MinimizeWindow() ===
void LXCB::MinimizeWindow(WId win){ //request that the window be unmapped/minimized
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

// === MaximizeWindow() ===
void LXCB::MaximizeWindow(WId win, bool flagsonly){ //request that the window become maximized
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

// === EmbedWindow() ===
uint LXCB::EmbedWindow(WId win, WId container){
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
    event.data.data32[0] = CurrentTime; 
    event.data.data32[1] = 0; //XEMBED_EMBEDDED_NOTIFY
    event.data.data32[2] = 0;
    event.data.data32[3] = container; //WID of the container
    event.data.data32[4] = 0;

    xcb_send_event(QX11Info::connection(), 0, win,  XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT, (const char *) &event);
  
  //Now setup any redirects and return
  //qDebug() << " - select Input";
  //XSelectInput(disp, win, StructureNotifyMask); //Notify of structure changes
  uint32_t val[] = {XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY};
  xcb_change_window_attributes(QX11Info::connection(), win, XCB_CW_EVENT_MASK, val);
  //qDebug() << " - Composite Redirect";
  xcb_composite_redirect_window(QX11Info::connection(), win, XCB_COMPOSITE_REDIRECT_MANUAL);

  //Now map the window (will be a transparent child of the container)
  xcb_map_window(QX11Info::connection(), win);
  
  //Now create/register the damage handler
  xcb_damage_damage_t dmgID = xcb_generate_id(QX11Info::connection()); //This is a typedef for a 32-bit unsigned integer
  xcb_damage_create(QX11Info::connection(), dmgID, win, XCB_DAMAGE_REPORT_LEVEL_RAW_RECTANGLES);
  
  //qDebug() << " - Done";
  return ( (uint) dmgID );	
}

// === Unembed Window() ===
bool LXCB::UnembedWindow(WId win){
  if(win==0){ return false; }
  //Display *disp = QX11Info::display();
  //Remove redirects
  //XSelectInput(disp, win, NoEventMask);
  uint32_t val[] = {XCB_EVENT_MASK_NO_EVENT};	
  xcb_change_window_attributes(QX11Info::connection(), win, XCB_CW_EVENT_MASK, val);
  //Make sure it is invisible
  xcb_unmap_window(QX11Info::connection(), win);
  //Reparent the window back to the root window
  xcb_reparent_window(QX11Info::connection(), win, QX11Info::appRootWindow(), 0, 0);
  return true;	
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
