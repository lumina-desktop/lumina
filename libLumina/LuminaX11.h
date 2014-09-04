//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
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
// Addition includes for compilations (cause issues with X11 libs later)
#include <QDir>
#include <QEvent>
#include <QHeaderView>



//X includes (these need to be last due to Qt compile issues)
//#include <X11/Xlib.h>
//#include <X11/Xutil.h>
//#include <X11/Xatom.h>
//#include <X11/extensions/Xrender.h>

//SYSTEM TRAY STANDARD DEFINITIONS
#define _NET_SYSTEM_TRAY_ORIENTATION_HORZ 0
#define _NET_SYSTEM_TRAY_ORIENTATION_VERT 1
#define SYSTEM_TRAY_REQUEST_DOCK 0
#define SYSTEM_TRAY_BEGIN_MESSAGE 1
#define SYSTEM_TRAY_CANCEL_MESSAGE 2

class LX11{
public:
	enum WINDOWSTATE {VISIBLE, INVISIBLE, ACTIVE, ATTENTION, IGNORE};
	
	//General Info Functions
	static QList<WId> WindowList(); //List all current windows
	static QList<WId> GetClientList(); // _NET_WM_CLIENT list
	static QList<WId> GetClientStackingList(); // _NET_WM_CLIENT_STACKING list
	static QList<WId> findChildren(WId, int); //Recursive utility for getting all children of a window
	static WId ActiveWindow();				//List the currently-active window
	
	//Window Manager Interaction
	static void SetNumberOfDesktops(int); 	// set number of virtual desktops
	static void SetCurrentDesktop(int); 		// set current virtual desktop
	static int GetNumberOfDesktops(); 		// get number of virtual desktops
	static int GetCurrentDesktop();			// get current virtual desktop
	//static bool ValidWindowEvent(Atom evAtom);	//Check if the property changed is a window event
	
	//Single Window Management
	static void CloseWindow(WId);			// Close the Window
	static void KillWindow(WId);			// Kill the window/application
	static void IconifyWindow(WId);			// Iconify (minimize) the Window
	static void RestoreWindow(WId);		// Restore (non-iconify) the Window
	static void ActivateWindow(WId);		// Make the window active;
	static void ReservePanelLocation(WId win, int xstart, int ystart, int width, int height, QString loc); //loc=[top,bottom,left,right]
	static void SetAsSticky(WId win);
	static void SetAsPanel(WId win);
	static void SetAsDesktop(WId win);

	//Advanced Window methods
	static void MoveResizeWindow(WId, QRect);
	static void ResizeWindow(WId, int width, int height);
	static WId CreateWindow(WId parent, QRect);
	static void DestroyWindow(WId);
	static bool EmbedWindow(WId win, WId container);
	static bool UnembedWindow(WId win);
	
	//Single Window Information
	static QString WindowClass(WId);		// Class name  (used for ID purposes)
	static QString WindowName(WId); 		// long name (untranslated)
	static QString WindowVisibleName(WId); 	// long name (translated)
	static QString WindowIconName(WId); 	// short name (untranslated)
	static QString WindowVisibleIconName(WId); // short name (translated)
	static QIcon WindowIcon(WId);			// Icon for the window
	static QPixmap WindowImage(WId win, bool useleader=true);		// Image for the window
	static int WindowDesktop(WId);			// Which virtual desktop the window is on
	static WINDOWSTATE GetWindowState(WId win); //State of activity
	static WId leaderWindow(WId); 			//Get the main window if this one is a redirect
	static bool isNormalWindow(WId win, bool includeDialogs=false);
	
	//System Tray Management
	static WId startSystemTray(int screen = 0); //Startup the system tray
	static void closeSystemTray(WId); //Close the system tray
	static QList<WId> findOrphanTrayWindows(); //scan for any orphaned tray apps
	
	
	//EWMH Convenience functions
	static QString getNetWMProp(WId win, QString prop); //Returns a _NET_WM_* string value
};

#endif