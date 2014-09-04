//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LuminaX11.h"

//X includes (these need to be last due to Qt compile issues)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xcomposite.h>

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
    else if( name.startsWith("Lumina-DE") ){ remove=true; }
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
QList<WId> LX11::findChildren(Window parent, int levels){
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
  Display *disp = QX11Info::display();
    XMapRaised(disp, win); //make it visible again and raise it to the top
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
  Display *disp = QX11Info::display();
  Atom stick = XInternAtom(disp, "_NET_WM_STATE_STICKY",false);
  Atom state = XInternAtom(disp, "_NET_WM_STATE", false);
  
  XChangeProperty(disp, win, state, XA_ATOM, 32, PropModeAppend, (unsigned char*) &stick, 1);
  
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
QIcon LX11::WindowIcon(WId win){
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
}


// ===== WindowImage() =====
QPixmap LX11::WindowImage(WId win, bool useleader){
  QPixmap pix;
  Display *disp = QX11Info::display();
  WId leader = LX11::leaderWindow(win); //check for an alternate window that contains the image
  if(leader!=0 && useleader){ win = leader; } //use the leader window instead
  //First get the size of the window image (embedded in the window attributes)
  XWindowAttributes att; 
  if( !XGetWindowAttributes(disp, win, &att) ){ return pix; } //invalid window attributes
  //Now extract the image
  XImage *xim = XGetImage(disp, win, 0,0, att.width, att.height, AllPlanes, ZPixmap);
  if(xim!=0){
    //Convert the X image to a Qt Image
    pix.convertFromImage( QImage( (const uchar*) xim->data, xim->width, xim->height, xim->bytes_per_line, QImage::Format_ARGB32_Premultiplied) );
    XDestroyImage(xim); //clean up
  }
  //Return the pixmap
  return pix;
}

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

  Display *disp = QX11Info::display(); /*
  Atom SA = XInternAtom(disp, "_NET_WM_STATE", false);
  Atom ATTENTION = XInternAtom(disp, "_NET_WM_STATE_DEMANDS_ATTENTION", false);
  Atom SKIPP = XInternAtom(disp, "_NET_WM_STATE_SKIP_PAGER", false);
  Atom HIDDEN = XInternAtom(disp, "_NET_WM_STATE_HIDDEN", false);
  Atom SKIPT = XInternAtom(disp, "_NET_WM_STATE_SKIP_TASKBAR", false);
  Atom MODAL = XInternAtom(disp, "_NET_WM_STATE_MODAL", false); */
  //Atom type;
  //int format;
  //unsigned long num, bytes;
  //unsigned char *data = 0;
  /*
  int status = XGetWindowProperty( disp, win, SA, 0, ~(0L), false, AnyPropertyType,
  	  			&type, &format, &num, &bytes, &data);
	
  LX11::WINDOWSTATE state = LX11::VISIBLE;
  if(status >= Success && data){
    Atom *array = (Atom*) data;
    for(unsigned int i=0; i<num; i++){
      if(forDisplay && (array[i] == SKIPP || array[i]==SKIPT || array[i]==MODAL) ){
      	state = LX11::IGNORE;
      	break;
      }else if(array[i]==HIDDEN){
	qDebug() << "Hidden Window:" << win;
	state = LX11::INVISIBLE;
      }else if(array[i]==ATTENTION){
	qDebug() << "Attention Window: " << win;
	state = LX11::ATTENTION;
      }
    }
    XFree(data);
  }
  */
  LX11::WINDOWSTATE state = LX11::VISIBLE;
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
  qDebug() << "Default Screen Atom Name:" << str;
  Atom _NET_SYSTEM_TRAY_S = XInternAtom(disp,str.toAscii(),false);
  //Make sure that there is no other system tray running
  if(XGetSelectionOwner(disp, _NET_SYSTEM_TRAY_S) != None){
    qWarning() << "An alternate system tray is currently in use";
    return 0;
  }
  //Create a simple window to register as the tray (not visible)
  Window LuminaSessionTrayID = XCreateSimpleWindow(disp, root,-1,-1,1,1,0,0,0);
  //register this widget as the system tray
  XSetSelectionOwner(disp, _NET_SYSTEM_TRAY_S, LuminaSessionTrayID, CurrentTime);
  //Make sure that it was registered properly
  if(XGetSelectionOwner(disp, _NET_SYSTEM_TRAY_S) != LuminaSessionTrayID){
    qWarning() << "Could not register the system tray";
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
