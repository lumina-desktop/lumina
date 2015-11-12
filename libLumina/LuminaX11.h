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

//XCB Library replacement for LX11 (Qt5 uses XCB instead of XLib)
class LXCB{
	
public:
	enum WINDOWSTATE {IGNORE, INVISIBLE, VISIBLE, ACTIVE, ATTENTION}; //note that this in order of priority
	enum ICCCM_STATE {WITHDRAWN, NORMAL, ICONIC};
	enum GRAVITY {FORGET=0, NW=1, N=2, NE=3, W=4, CENTER=5, E=6, SW=7, S=8, SE=9, STATIC=10};
	enum STACK_FLAG {ABOVE=0, BELOW=1, TOP_IF=2, BOTTOM_IF=3, OPPOSITE=4};
	//Now enums which can have multiple values at once (Use the plural form for the QFlags)
	enum ICCCM_PROTOCOL {TAKE_FOCUS = 0x0, DELETE_WINDOW = 0x1}; //any combination 
	Q_DECLARE_FLAGS(ICCCM_PROTOCOLS, ICCCM_PROTOCOL);
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
	WINDOWSTATE WindowState(WId win); //Visible state of window
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
	void SelectInput(WId); //XSelectInput replacement (to see window events)
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
	// WM Functions (directly changing properties/settings)
	//  - Using these directly may prevent the WM from seeing the change
	//============
	void WM_CloseWindow(WId win);
	
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
	// -- WM_SIZE_HINTS
	
	// -- WM_NORMAL_HINTS
	
	// -- WM_HINTS
	
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
	
	// _NET_WM_STATE
	
	// _NET_WM_ALLOWED_ACTIONS
	
	// _NET_WM_STRUT
	
	// _NET_WM_STRUT_PARTIAL
	
	// _NET_WM_ICON_GEOMETRY
	
	// _NET_WM_ICON
	
	// _NET_WM_PID
	
	// _NET_WM_HANDLED_ICONS
	
	// _NET_WM_USER_TIME
	
	// _NET_WM_USER_TIME_WINDOW
	
	// _NET_FRAME_EXTENTS
	
	// _NET_WM_OPAQUE_REGION
	
	// _NET_WM_BYPASS_COMPOSITOR
	
	
private:
	QList<xcb_atom_t> ATOMS;
	QStringList atoms;

	void createWMAtoms(); //fill the private lists above
};
//Now also declare the flags for Qt to be able to use normal operations on them
Q_DECLARE_OPERATORS_FOR_FLAGS(LXCB::ICCCM_PROTOCOLS);
Q_DECLARE_OPERATORS_FOR_FLAGS(LXCB::MOVERESIZE_WINDOW_FLAGS);

#endif