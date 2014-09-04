//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This class is the interface to load all the different panel plugins
//===========================================
#ifndef _LUMINA_DESKTOP_NEW_PANEL_PLUGIN_H
#define _LUMINA_DESKTOP_NEW_PANEL_PLUGIN_H

#include <QDebug>

//List all the individual plugin includes here
#include "LPPlugin.h"
#include "userbutton/LUserButton.h"
#include "desktopbar/LDeskBar.h"
#include "spacer/LSpacer.h"
#include "clock/LClock.h"
#include "battery/LBattery.h"
#include "desktopswitcher/LDesktopSwitcher.h"
#include "taskmanager/LTaskManagerPlugin.h"
#include "systemdashboard/LSysDashboard.h"
#include "systemtray/LSysTray.h" //must be last due to X11 compile issues

class NewPP{
public:
	static LPPlugin* createPlugin(QString plugin, QWidget* parent = 0, bool horizontal = true){
	  LPPlugin *plug = 0;
	  if(plugin.startsWith("userbutton---")){
	    plug = new LUserButtonPlugin(parent, plugin, horizontal);
	  }else if(plugin.startsWith("desktopbar---")){
	    plug = new LDeskBarPlugin(parent, plugin, horizontal);
	  }else if(plugin.startsWith("spacer---")){
	    plug = new LSpacerPlugin(parent, plugin, horizontal);
	  }else if(plugin.startsWith("taskmanager---")){
	    plug = new LTaskManagerPlugin(parent, plugin, horizontal);
	  }else if(plugin.startsWith("systemtray---")){
	    plug = new LSysTray(parent, plugin, horizontal);
	  }else if(plugin.startsWith("desktopswitcher---")){
	    plug = new LDesktopSwitcher(parent, plugin, horizontal);
	  }else if(plugin.startsWith("battery---")){
	    plug = new LBattery(parent, plugin, horizontal);
	  }else if(plugin.startsWith("clock---")){
	    plug = new LClock(parent, plugin, horizontal);
	  }else if(plugin.startsWith("systemdashboard---")){
	    plug = new LSysDashboard(parent, plugin, horizontal);
	  }else{
	    qWarning() << "Invalid Panel Plugin:"<<plugin << " -- Ignored";
	  }
	  return plug;
	}

};

#endif
