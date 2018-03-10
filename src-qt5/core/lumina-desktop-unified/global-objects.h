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

#include "src-events/NativeWindowSystem.h"
#include "src-events/NativeEventFilter.h"
#include "src-desktop/src-cpp/RootDesktopObject.h"

#include "src-events/LShortcutEvents.h"
#include "src-desktop/DesktopManager.h"
#include "src-screensaver/LScreenSaver.h"

#include <RootWindow.h>
#include "LSession.h"

// Standard C includes
#include <unistd.h>

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

	//ScreenSaver
	extern LScreenSaver *SS;
	//Root Window
	extern RootWindow *ROOTWIN;
	//Desktop Manager
	extern DesktopManager *DESKMAN;
	//Application List
	extern XDGDesktopList *APPLIST;

	extern QThread *EVThread; //X Event thread
};

#endif
