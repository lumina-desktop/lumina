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
#ifndef USE_WAYLAND
#include "src-WM/LXcbEventFilter.h"
#endif

#include "src-WM/LScreenSaver.h"
#include "src-WM/LWindowManager.h"

//Any special defines for settings/testing
#define ANIMTIME 80 //animation time in milliseconds

//Global flags/structures
namespace Lumina{
	//Flags/enumerations
	enum WindowAction{MoveResize, Show, Hide, TryClose, Closed, WA_NONE};
	
	//Data structures and objects
	extern LXCB *SYSTEM; //Native graphic system interface
	EventFilter *EFILTER; //Native Event Watcher
	//ScreenSaver
	LScreenSaver *SS;
	//Window Manager
	LWindowManager *WM;
	
	QThread *EVThread; //X Event thread

};


#endif
