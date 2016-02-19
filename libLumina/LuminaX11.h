//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This class governs all the XLib usage and interactions
//  and provides simpler Qt-based functions for use elsewhere
//===========================================
#ifndef _LUMINA_LIBRARY_X11_H
#define _LUMINA_LIBRARY_X11_H

//Qt includes
#include <QList>
#include <QString>
#include <QPixmap>
#include <QImage>
#include <QIcon>
#include <QPixmap>
#include <QX11Info>
#include <QDebug>
#include <QPainter>
#include <QObject>
#include <QFlags>


#include <xcb/xcb_ewmh.h>

//SYSTEM TRAY STANDARD DEFINITIONS
#define _NET_SYSTEM_TRAY_ORIENTATION_HORZ 0
#define _NET_SYSTEM_TRAY_ORIENTATION_VERT 1
#define SYSTEM_TRAY_REQUEST_DOCK 0
#define SYSTEM_TRAY_BEGIN_MESSAGE 1
#define SYSTEM_TRAY_CANCEL_MESSAGE 2

#define URGENCYHINT (1L << 8) //For window urgency detection

//Simple data container for the ICCCM hints (_size, _normal, _hints)

//Simple data container for doing STRUT_PARTIAL input/output calculations
class strut_geom{
public:
  //Note: "Thick" will always be in the direction perpenticular to the start/end coordinates
  //Example: A left strut will use start/end as Y coordinates, with "thick" in X coordinates starting from the left edge
  unsigned int start, end, thick;
  strut_geom(){ start = end = thick = 0; }
  ~strut_geom(){}
};

class icccm_size_hints{
public:
  int x, y, width, height, min_width, min_height, max_width, max_height;
  //Note: The "x","y","width", and "height" values are considered depreciated in the ICCCM specs
  int width_inc, height_inc, min_aspect_num, min_aspect_den, max_aspect_num, max_aspect_den;
  int base_width, base_height;
  unsigned int win_gravity; //LXCB::GRAVITY value

  icccm_size_hints(){
    x=y=width=height=min_width=max_width=min_height=max_height = -1;
    width_inc=height_inc=min_aspect_num=min_aspect_den=max_aspect_num=max_aspect_den = -1;
    win_gravity = 0;
  }
  ~icccm_size_hints(){}
  bool isValid(){
    //See if any of the values are different from the init values
    return ( x>=0 || y>=0 || width>=0 || height>=0 || min_width>=0 || min_height>=0 || max_width>=0 || max_height>=0 \
	  || width_inc>=0 || height_inc>=0 || min_aspect_num>=0 || min_aspect_den>=0 || max_aspect_num>=0 || max_aspect_den>=0 \
	  || base_width>=0 || base_height>=0 || win_gravity>0 );
  }
};

//XCB Library replacement for LX11 (Qt5 uses XCB instead of XLib)
class LXCB{
	
public:
	enum WINDOWVISIBILITY {IGNORE, INVISIBLE, VISIBLE, ACTIVE, ATTENTION}; //note that this in order of priority
	enum ICCCM_STATE {WITHDRAWN, NORMAL, ICONIC};
	enum GRAVITY {FORGET=0, NW=1, N=2, NE=3, W=4, CENTER=5, E=6, SW=7, S=8, SE=9, STATIC=10};
	enum STACK_FLAG {ABOVE=0, BELOW=1, TOP_IF=2, BOTTOM_IF=3, OPPOSITE=4};
	enum WINDOWTYPE {T_DESKTOP, T_DOCK, T_TOOLBAR, T_MENU, T_UTILITY, T_SPLASH, T_DIALOG, T_DROPDOWN_MENU, T_POPUP_MENU, T_TOOLTIP, T_NOTIFICATION, T_COMBO, T_DND, T_NORMAL};
	enum WINDOWSTATE {S_MODAL, S_STICKY, S_MAX_VERT, S_MAX_HORZ, S_SHADED, S_SKIP_TASKBAR, S_SKIP_PAGER, S_HIDDEN, S_FULLSCREEN, S_ABOVE, S_BELOW, S_ATTENTION};
	enum WINDOWACTION {A_MOVE, A_RESIZE, A_MINIMIZE, A_SHADE, A_STICK, A_MAX_VERT, A_MAX_HORZ, A_FULLSCREEN, A_CHANGE_DESKTOP, A_CLOSE, A_ABOVE, A_BELOW};
	//Now enums which can have multiple values at once (Use the plural form for the QFlags)
	enum ICCCM_PROTOCOL {TAKE_FOCUS = 0x0, DELETE_WINDOW = 0x1}; //any combination 
	Q_DECLARE_FLAGS(ICCCM_PROTOCOLS, ICCCM_PROTOCOL);
	enum SIZE_HINT { US_POSITION=1<<0, US_SIZE=1<<1, P_POSITION=1<<2, P_SIZE=1<<3, P_MIN_SIZE=1<<4, P_MAX_SIZE=1<<5, P_RESIZE_INC=1<<6, P_ASPECT=1<<7, BASE_SIZE=1<<8, P_WIN_GRAVITY=1<<9 };
	Q_DECLARE_FLAGS(SIZE_HINTS, SIZE_HINT);
	enum MOVERESIZE_WINDOW_FLAG { X=0x0, Y=0x1, WIDTH=0x2, HEIGHT=0x3};
	Q_DECLARE_FLAGS(MOVERESIZE_WINDOW_FLAGS, MOVERESIZE_WINDOW_FLAG);
	
	xcb_ewmh_connection_t EWMH; //This is where all the screen info and atoms are located

	LXCB();
	~LXCB();
		
	//== Main Interface functions ==
	// General Information
	QList<WId> WindowList(bool rawlist = false); //list all non-Lumina windows (rawlist -> all workspaces)
	unsigned int CurrentWorkspace();
	unsigned int NumberOfWorkspaces();
	WId ActiveWindow(); //fetch the ID for the currently active window
	
	//Session Modification
	bool CheckDisableXinerama(); //returns true if Xinerama was initially set but now disabled
	void RegisterVirtualRoots(QList<WId> roots);
	void SetCurrentWorkspace(int);

	//Window Information
	QString WindowClass(WId);
	unsigned int WindowWorkspace(WId); //The workspace the window is on
	QRect WindowGeometry(WId win, bool includeFrame = true); //the geometry of the window (frame excluded)
	QList<int> WindowFrameGeometry(WId win); //Returns: [top,bottom,left,right] sizes of the frame
	LXCB::WINDOWVISIBILITY WindowState(WId win); //Visible state of window
	QString WindowVisibleIconName(WId win); //_NET_WM_VISIBLE_ICON_NAME
	QString WindowIconName(WId win); //_NET_WM_ICON_NAME
	QString WindowVisibleName(WId win); //_NET_WM_VISIBLE_NAME
	QString WindowName(WId win); //_NET_WM_NAME
	QString OldWindowName(WId win); //WM_NAME (old standard)
	QString OldWindowIconName(WId win); //WM_ICON_NAME (old standard)
	bool WindowIsMaximized(WId win);
	int WindowIsFullscreen(WId win); //Returns the screen number if the window is fullscreen (or -1)
	QIcon WindowIcon(WId win); //_NET_WM_ICON
	
	//Window Modification
	// - SubStructure simplifications (not commonly used)
	void SelectInput(WId win, bool isEmbed = false); //XSelectInput replacement (to see window events)
	uint GenerateDamageID(WId);
	
	// - General Window Modifications
	void SetAsSticky(WId); //Stick to all workspaces
	void SetDisableWMActions(WId); //Disable WM control (shortcuts/automatic functions)
	void SetAsPanel(WId); //Adjust all the window flags for a proper panel (cannot be done through Qt)
	void SetAsDesktop(WId); //Adjust window flags to set as the desktop
	void CloseWindow(WId); //request that the window be closed
	void KillClient(WId); //Force the application that created the window to close
	void MinimizeWindow(WId); //request that the window be unmapped/minimized
	void ActivateWindow(WId); //request that the window become active
	void RestoreWindow(WId); //Re-map/raise the window
	void MaximizeWindow(WId win, bool flagsonly = false); //request that the window become maximized
	void MoveResizeWindow(WId win, QRect geom);
	void ResizeWindow(WId win, int width, int height);
	void ResizeWindow(WId win, QSize sz){ ResizeWindow(win, sz.width(), sz.height()); } //overload for simplicity
	void ReserveLocation(WId win, QRect geom, QString loc);
	
	//Window Embedding/Detaching (for system tray)
	//void SetWindowBackground(QWidget *parent, QRect area, WId client);
	uint EmbedWindow(WId win, WId container); //returns the damage ID (or 0 for an error)
	bool UnembedWindow(WId win);
	QPixmap TrayImage(WId win);
	
	//System Tray Management
	WId startSystemTray(int screen = 0); //Startup the system tray (returns window ID for tray)
	void closeSystemTray(WId); //Close the system tray

	
	//============
	// WM Functions (directly changing/reading properties)
	//  - Using these directly may prevent the WM from seeing the change
	//============
	void WM_CloseWindow(WId win, bool force = false);
	void WM_ShowWindow(WId win);
	void WM_HideWindow(WId win);
	
	WId WM_CreateWindow(WId parent = 0);
	
	// WM Utility Functions
	QList<WId> WM_RootWindows(); //return all windows which have root as the parent
	bool WM_ManageWindow(WId win, bool needsmap = true); //return whether the window is/should be managed
	QRect WM_Window_Geom(WId win); //Return the current window geometry
	void setupEventsForFrame(WId frame);
	bool setupEventsForRoot(WId root = 0);
	
	// ICCCM Standards (older standards)
	// -- WM_NAME
	QString WM_ICCCM_GetName(WId win);
	void WM_ICCCM_SetName(WId win, QString name);
	// -- WM_ICON_NAME
	QString WM_ICCCM_GetIconName(WId win);
	void WM_ICCCM_SetIconName(WId win, QString name);
	// --- WM_CLIENT_MACHINE
	QString WM_ICCCM_GetClientMachine(WId win);
	void WM_ICCCM_SetClientMachine(WId win, QString name);
	// -- WM_CLASS
	QString WM_ICCCM_GetClass(WId win); //Returns: "<instance name>::::<class name>"
	void WM_ICCCM_SetClass(WId win, QString name);
	// -- WM_TRANSIENT_FOR
	WId WM_ICCCM_GetTransientFor(WId win); //Returns "win" for errors or no transient
	void WM_ICCCM_SetTransientFor(WId win, WId transient);
	// -- WM_SIZE_HINTS (older property?)
	icccm_size_hints WM_ICCCM_GetSizeHints(WId win); //most values in structure are -1 if not set
	//void WM_ICCCM_SetSizeHints(WId win, icccm_size_hints hints);
	// -- WM_NORMAL_HINTS (newer property? - check for this before falling back on WM_SIZE_HINTS)
	icccm_size_hints WM_ICCCM_GetNormalHints(WId win); //most values in structure are -1 if not set
	//void WM_ICCCM_SetNormalHints(WId win, icccm_size_hints hints);
	// -- WM_HINTS (contains WM_STATE)
	
	// -- WM_PROTOCOLS
	ICCCM_PROTOCOLS WM_ICCCM_GetProtocols(WId win);
	void WM_ICCCM_SetProtocols(WId win, ICCCM_PROTOCOLS flags);
	
	//NET_WM Standards (newer standards)
	
	// -- ROOT WINDOW PROPERTIES
	// _NET_SUPPORTED
	void WM_Set_Root_Supported(); //set the atom list of supported features on the root window
	// _NET_CLIENT_LIST
	// Note: client list ordered oldest->newest, stacking list ordered bottom->top
	QList<WId> WM_Get_Client_List(bool stacking = false);
	void WM_Set_Client_List(QList<WId> list, bool stacking=false);
	
	// _NET_NUMBER_OF_DESKTOPS
	// Note: This is the number of virtual workspaces, not monitors
	unsigned int WM_Get_Number_Desktops(); //return value equals 0 for errors
	void WM_SetNumber_Desktops(unsigned int number); //should be at least 1
	
	// _NET_DESKTOP_GEOMETRY
	// Note: This property is the combined size and/or bounding rectangle of all monitors
	//  The definition works well for single-monitors, but gets really fuzzy for multiple monitors
	QSize WM_Get_Desktop_Geometry();
	void WM_Set_Desktop_Geometry(QSize);
	
	// _NET_DESKTOP_VIEWPORT
	// Note: This is the X/Y origin  of the viewport for each monitor 
	//  Thi is normally (0,0) , unless desktop larger than monitor supports
	QList<QPoint> WM_Get_Desktop_Viewport();
	void WM_Set_Desktop_Viewport(QList<QPoint> list);
	
	// _NET_CURRENT_DESKTOP
	// Note: Current workspace number. Range = 0 to (_NET_NUMBER_OF_DESKTOPS - 1)
	int WM_Get_Current_Desktop(); //Returns -1 for errors
	void WM_Set_Current_Desktop(unsigned int num);
	
	// _NET_DESKTOP_NAMES
	QStringList WM_Get_Desktop_Names();
	void WM_Set_Desktop_Names(QStringList list);
	
	// _NET_ACTIVE_WINDOW
	WId WM_Get_Active_Window();
	void WM_Set_Active_Window(WId win);
	
	// _NET_WORKAREA
	// Note: The workarea is the recangle for each monitor where no space is reserved
	//  This accounts for any STRUT's that are set, within the current VIEWPORT
	QList<QRect> WM_Get_Workarea();
	void WM_Set_Workarea(QList<QRect> list);
	
	// _NET_SUPPORTING_WM_CHECK
	// Note: This needs to be set on two windows: root -> child, and child->child
	// So the "set" function will do both at the same time
	// The child window also needs the _NET_WM_NAME set to the window manager name
	WId WM_Get_Supporting_WM(WId win);
	void WM_Set_Supporting_WM(WId child);
	
	// _NET_VIRTUAL_ROOTS
	QList<WId> WM_Get_Virtual_Roots();
	void WM_Set_Virtual_Roots(QList<WId> list);
	
	// _NET_DESKTOP_LAYOUT
	// NOTE: Skip this implementation for now - is supposed to be set by a pager (not the WM)
	//   and the WM can choose to use/ignore it as necessary.
	// (Just use the current XRandR layout instead of this setting/property)
	
	// _NET_SHOWING_DESKTOP
	// Note: This is true/false depending on whether the WM is hiding all windows to show the desktop only
	bool WM_Get_Showing_Desktop();
	void WM_Set_Showing_Desktop(bool show);
	
	// -- ROOT WINDOW MESSAGES/REQUESTS (for non-WM usage)
	// _NET_CLOSE_WINDOW
	void WM_Request_Close_Window(WId win);
	
	// _NET_MOVERESIZE_WINDOW
	// Note: Used for finalized movement/resize operations
	void WM_Request_MoveResize_Window(WId win, QRect geom, bool fromuser = false,  LXCB::GRAVITY grav = LXCB::STATIC, LXCB::MOVERESIZE_WINDOW_FLAGS flags = LXCB::MOVERESIZE_WINDOW_FLAGS(LXCB::X | LXCB::Y | LXCB::WIDTH | LXCB::HEIGHT) );
	
	// _NET_WM_MOVERESIZE 
	// Note: Used for interactive clicks/changes to a window size/position
	// There are known race conditions/issues with this X format - so skip it for now (11/12/15)
	
	// _NET_RESTACK_WINDOW
	// Note: Send a request to re-stack a window (win) with respect to another window (sibling)
	//   based on the flag to determine how the stack order should be changed
	void WM_Request_Restack_Window(WId win, WId sibling, LXCB::STACK_FLAG flag);
	
	// _NET_REQUEST_FRAME_EXTENTS
	// Note: This is used by client windows to get the _NET_FRAME_EXTENTS property pre-set
	//  by the WM before the window is actually mapped (just an estimate of the frame at least)
	void WM_Request_Frame_Extents(WId win);
	
	// -- WINDOW PROPERTIES
	// _NET_SUPPORTED
	void WM_Set_Window_Supported(WId win); //set the atom list of supported features on the given window
	
	// _NET_WM_NAME
	QString WM_Get_Name(WId win);
	void WM_Set_Name(WId win, QString txt);
	
	// _NET_WM_VISIBLE_NAME
	QString WM_Get_Visible_Name(WId win);
	void WM_Set_Visible_Name(WId win, QString txt);
	
	// _NET_WM_ICON_NAME
	QString WM_Get_Icon_Name(WId win);
	void WM_Set_Icon_Name(WId win, QString txt);
	
	// _NET_WM_VISIBLE_ICON_NAME
	QString WM_Get_Visible_Icon_Name(WId win);
	void WM_Set_Visible_Icon_Name(WId win, QString txt);
	
	// _NET_WM_DESKTOP
	// Note: This refers to the virtual workspace, not the monitor/screen number
	int WM_Get_Desktop(WId win); //returns -1 if window on all desktops
	void WM_Set_Desktop(WId win, int num); //use -1 to set it for all desktops
	
	// _NET_WM_WINDOW_TYPE
	// Note: While this returns a list, they are ordered by priority for WM usage (use the first one known about)
	QList<LXCB::WINDOWTYPE> WM_Get_Window_Type(WId win);
	void WM_Set_Window_Type(WId win, QList<LXCB::WINDOWTYPE> list);
	
	// _NET_WM_STATE
	QList<LXCB::WINDOWSTATE> WM_Get_Window_States(WId win);
	void WM_Set_Window_States(WId win, QList<LXCB::WINDOWSTATE> list);
	
	// _NET_WM_ALLOWED_ACTIONS
	QList<LXCB::WINDOWACTION> WM_Get_Window_Actions(WId win);
	void WM_Set_Window_Actions(WId win, QList<LXCB::WINDOWACTION> list);
	
	// _NET_WM_STRUT
	QList<unsigned int> WM_Get_Window_Strut(WId win); //Returns: [left,right,top,bottom] margins in pixels (always length 4)
	void WM_Set_Window_Strut(WId win, QList<unsigned int> margins); //Input: [left, right, top, bottom] - must be length 4
	
	// _NET_WM_STRUT_PARTIAL
	QList<strut_geom> WM_Get_Window_Strut_Partial(WId win); //Returns: [left,right,top,bottom] struts
	void WM_Set_Window_Strut_Partial(WId win, QList<strut_geom> struts); //Input: [left,right,top,bottom] - must be length 4
	
	// _NET_WM_ICON_GEOMETRY
	QRect WM_Get_Icon_Geometry(WId win);
	void WM_Set_Icon_Geometry(WId win, QRect geom);
	
	// _NET_WM_ICON
	// Note: Don't write a "Set" routine for this - is handled on the client side and not the WM/DE side
	QIcon WM_Get_Icon(WId win);
	
	// _NET_WM_PID
	// Note: Don't write a "Set" routine for this - is handled on the client side and not the WM/DE side
	unsigned int WM_Get_Pid(WId win);
	
	// _NET_WM_HANDLED_ICONS
	// Note: Probably not going to need this - is used by pagers exclusively to tell the WM 
	//  not to provide task manager icons (not needed for an integrated WM/DE combination)
	bool WM_Get_Handled_Icons(WId win);
	void WM_Set_Handled_Icons(WId win, bool set);
	
	// _NET_WM_USER_TIME
	// Note: The user time property on a client window is supposed to be updated on user activity,
	//   allowing the WM to be able to distinguish user activity from automated window actions
	unsigned int WM_Get_User_Time(WId win);
	void WM_Set_User_Time(WId win, unsigned int xtime);
	
	// _NET_WM_USER_TIME_WINDOW
	// This returns the window to watch for time update events, 
	//  instead of constantly polling all the (toplevel?) windows for the app
	// IGNORED - xcb_ewmh library does not appear to have valid support for this property yet (11/13/15)
	//WId WM_Get_User_Time_WIndow(WId win);
	//void WM_Set_User_Time_Window(WId win, WId utwin);
	
	// _NET_FRAME_EXTENTS
	QList<unsigned int> WM_Get_Frame_Extents(WId win); //Returns: [left,right,top,bottom] margins in pixels (always length 4)
	void WM_Set_Frame_Extents(WId win, QList<unsigned int> margins); //Input: [left, right, top, bottom] - must be length 4
	
	// _NET_WM_OPAQUE_REGION
	// NOT SUPPORTED - missing in xcb_ewmh library (11/13/15)
	
	// _NET_WM_BYPASS_COMPOSITOR
	// NOT SUPPORTED - missing in xcb_ewmh library (11/13/15)
	
	// === SPECIAL WM PROTOCOLS (EWMH) ===
	// _NET_WM_PING
	// Note: Used to determine if a window/app is hung before killing the process (with PID)
	// The client window should respond instantly if it is still active (waiting on user input for instance)
	void WM_Send_Ping(WId win);
	
	// _NET_WM_SYNC_REQUEST
	uint64_t WM_Get_Sync_Request_Counter(WId win);
	//void WM_Set_Sync_Request_Counter(WId win, uint64_t count);
	
	// _NET_WM_FULLSCREEN_MONITORS
	QList<unsigned int> WM_Get_Fullscreen_Monitors(WId win); //Returns: [top,bottom,left,right] monitor numbers for window to use when fullscreen
	void WM_Set_Fullscreen_Montors(WId win, QList<unsigned int> list); //Input: [top,bottom,left,right] monitor numbers
	
	// _NET_WM_CM_S(n)
	// Note: This is used to check/register the compositing manager for the current X screen (#n)
	WId WM_Get_CM_Owner();
	void WM_Set_CM_Owner(WId win);
	
private:
	QList<xcb_atom_t> ATOMS;
	QStringList atoms;

	void createWMAtoms(); //fill the private lists above
};
//Now also declare the flags for Qt to be able to use normal operations on them
Q_DECLARE_OPERATORS_FOR_FLAGS(LXCB::ICCCM_PROTOCOLS);
Q_DECLARE_OPERATORS_FOR_FLAGS(LXCB::MOVERESIZE_WINDOW_FLAGS);
Q_DECLARE_OPERATORS_FOR_FLAGS(LXCB::SIZE_HINTS);

#endif
