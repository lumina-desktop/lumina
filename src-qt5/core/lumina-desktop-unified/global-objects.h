//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2015-2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  Global defines and enumerations for external includes
//===========================================
#ifndef _LUMINA_INTERNAL_GLOBAL_OBJECTS_H
#define _LUMINA_INTERNAL_GLOBAL_OBJECTS_H

#include "Global-includes.h"

//Any special defines for settings/testing
#define ANIMTIME 80 //animation time in milliseconds

//Global flags/structures
namespace Lumina{
	//Flags/enumerations
	enum WindowAction{MoveResize, Show, Hide, TryClose, Closed, WA_NONE};
	
	//Data structures and objects
	extern LXCB *SYSTEM;

};


#endif
