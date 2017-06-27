//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2015-2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  Global defines and enumerations for internal includes
//===========================================

//====WARNING=====
// ONLY #include this file within *.cpp files
// Use "global-includes.h" for the generic includes in *.h files
//=================

#ifndef _LUMINA_INTERNAL_GLOBAL_OBJECTS_H
#define _LUMINA_INTERNAL_GLOBAL_OBJECTS_H

#include "global-includes.h"

//Load the appropriate "EventFilter" class for the graphics subsystem
//#ifndef USE_WAYLAND
//#include "src-events/LXcbEventFilter.h"
//#endif
#include "src-events/LShortcutEvents.h"

#include "src-screensaver/LScreenSaver.h"
//#include "src-WM/LWindowManager.h"

//Any special defines for settings/testing
#define ANIMTIME 80 //animation time in milliseconds

//Global flags/structures
namespace Lumina{
	//Data structures and objects
	// -- Native Window System Objects
	extern NativeWindowSystem *NWS;
	extern NativeEventFilter *NEF;

	//extern EventFilter *EFILTER; //Native Event Watcher
	extern LShortcutEvents *SHORTCUTS; //Keyboard/mouse shortcut events
	extern DesktopSettings *SETTINGS; //All Settings files
	//ScreenSaver
	extern LScreenSaver *SS;
	//Root Window
	extern RootWindow *ROOTWIN;
	//Window Manager
	//LWindowManager *WM;
	//Application List
	extern XDGDesktopList *APPLIST;

	extern QThread *EVThread; //X Event thread
};

#endif
