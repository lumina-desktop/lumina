//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LPlugins.h"

LPlugins::LPlugins(){
  LoadPanelPlugins();
  LoadDesktopPlugins();
  LoadMenuPlugins();
  LoadColorItems();
}

LPlugins::~LPlugins(){
}

//Plugin lists
QStringList LPlugins::panelPlugins(){
  return PANEL.keys();
}
QStringList LPlugins::desktopPlugins(){
  return DESKTOP.keys();
}
QStringList LPlugins::menuPlugins(){
  return MENU.keys();
}
QStringList LPlugins::colorItems(){
  return COLORS.keys();
}
//Information on individual plugins
LPI LPlugins::panelPluginInfo(QString plug){
  if(PANEL.contains(plug)){ return PANEL[plug]; }
  else{ return LPI(); }
}
LPI LPlugins::desktopPluginInfo(QString plug){
  if(DESKTOP.contains(plug)){ return DESKTOP[plug]; }
  else{ return LPI(); }
}
LPI LPlugins::menuPluginInfo(QString plug){
  if(MENU.contains(plug)){ return MENU[plug]; }
  else{ return LPI(); }
}
LPI LPlugins::colorInfo(QString item){
  if(COLORS.contains(item)){ return COLORS[item]; }
  else{ return LPI(); }  
}

//===================
//             PLUGINS
//===================
void LPlugins::LoadPanelPlugins(){
  PANEL.clear();
  //User Button
  LPI info;
    info.name = QObject::tr("User Button");
    info.description = QObject::tr("This is the main system access button for the user (applications, directories, settings, log out).");
    info.ID = "userbutton";
    info.icon = "user-identity";
  PANEL.insert(info.ID, info);
  //Desktop Bar
  info = LPI(); //clear it
    info.name = QObject::tr("Desktop Bar");
    info.description = QObject::tr("This provides shortcuts to everything in the desktop folder - allowing easy access to all your favorite files/applications.");
    info.ID = "desktopbar";
    info.icon = "user-desktop";
  PANEL.insert(info.ID, info);  
  //Spacer
  info = LPI(); //clear it
    info.name = QObject::tr("Spacer");
    info.description = QObject::tr("Invisible spacer to separate plugins.");
    info.ID = "spacer";
    info.icon = "transform-move";
  PANEL.insert(info.ID, info);  	
  //Desktop Switcher
  info = LPI(); //clear it
    info.name = QObject::tr("Desktop Switcher");
    info.description = QObject::tr("Controls for switching between the various virtual desktops.");
    info.ID = "desktopswitcher";
    info.icon = "preferences-desktop-display-color";
  PANEL.insert(info.ID, info);  	
  //Battery
  info = LPI(); //clear it
    info.name = QObject::tr("Battery Monitor");
    info.description = QObject::tr("Keep track of your battery status.");
    info.ID = "battery";
    info.icon = "battery-charging";
  PANEL.insert(info.ID, info);  	
  //Clock
  info = LPI(); //clear it
    info.name = QObject::tr("Time/Date");
    info.description = QObject::tr("View the current time and date.");
    info.ID = "clock";
    info.icon = "preferences-system-time";
  PANEL.insert(info.ID, info);  
  //System Dachboard plugin
  info = LPI(); //clear it
    info.name = QObject::tr("System Dashboard");
    info.description = QObject::tr("View or change system settings (audio volume, screen brightness, battery life, virtual desktops).");
    info.ID = "systemdashboard";
    info.icon = "dashboard-show";
  PANEL.insert(info.ID, info);  
  //Task Manager
  info = LPI(); //clear it
    info.name = QObject::tr("Task Manager");
    info.description = QObject::tr("View and control any running application windows (every application has a button)");
    info.ID = "taskmanager";
    info.icon = "preferences-system-windows";
  PANEL.insert(info.ID, info); 
  //Task Manager
  info = LPI(); //clear it
    info.name = QObject::tr("Task Manager (No Groups)");
    info.description = QObject::tr("View and control any running application windows (every window has a button)");
    info.ID = "taskmanager-nogroups";
    info.icon = "preferences-system-windows";
  PANEL.insert(info.ID, info); 
  //System Tray
  info = LPI(); //clear it
    info.name = QObject::tr("System Tray");
    info.description = QObject::tr("Display area for dockable system applications");
    info.ID = "systemtray";
    info.icon = "preferences-system-windows-actions";
  PANEL.insert(info.ID, info);  
}

void LPlugins::LoadDesktopPlugins(){
  DESKTOP.clear();
  //Calendar Plugin
  LPI info;
    info.name = QObject::tr("Calendar");
    info.description = QObject::tr("Display a calendar on the desktop");
    info.ID = "calendar";
    info.icon = "view-calendar";
  DESKTOP.insert(info.ID, info);
  //Application Launcher Plugin
  info = LPI(); //clear it
    info.name = QObject::tr("Application Launcher");
    info.description = QObject::tr("Desktop button for launching an application");
    info.ID = "applauncher";
    info.icon = "quickopen";
  DESKTOP.insert(info.ID, info);
  //Desktop View Plugin
  info = LPI(); //clear it
    info.name = QObject::tr("Desktop Icons View");
    info.description = QObject::tr("Area for automatically showing desktop icons");
    info.ID = "desktopview";
    info.icon = "preferences-desktop-icons";
  DESKTOP.insert(info.ID, info);
  //Notepad Plugin
  info = LPI(); //clear it
    info.name = QObject::tr("Note Pad");
    info.description = QObject::tr("Keep simple text notes on your desktop");
    info.ID = "notepad";
    info.icon = "text-enriched";
  DESKTOP.insert(info.ID, info);
}

void LPlugins::LoadMenuPlugins(){
  MENU.clear();
  //Terminal
  LPI info;
    info.name = QObject::tr("Terminal");
    info.description = QObject::tr("Start the default system terminal.");
    info.ID = "terminal";
    info.icon = "utilities-terminal";
  MENU.insert(info.ID, info);
  //File Manager
  info = LPI(); //clear it
    info.name = QObject::tr("File Manager");
    info.description = QObject::tr("Browse the system with the default file manager.");
    info.ID = "filemanager";
    info.icon = "Insight-FileManager";
  MENU.insert(info.ID, info);
  //Applications
  info = LPI(); //clear it
    info.name = QObject::tr("Applications");
    info.description = QObject::tr("Show the system applications menu.");
    info.ID = "applications";
    info.icon = "system-run";
  MENU.insert(info.ID, info);
  //Line seperator
  info = LPI(); //clear it
    info.name = QObject::tr("Separator");
    info.description = QObject::tr("Static horizontal line.");
    info.ID = "line";
    info.icon = "insert-horizontal-rule";
  MENU.insert(info.ID, info);
  //Settings
  info = LPI(); //clear it
    info.name = QObject::tr("Settings");
    info.description = QObject::tr("Show the desktop settings menu.");
    info.ID = "settings";
    info.icon = "configure";
  MENU.insert(info.ID, info);
  //Window List
  info = LPI(); //clear it
    info.name = QObject::tr("Window List");
    info.description = QObject::tr("List the open application windows");
    info.ID = "windowlist";
    info.icon = "preferences-system-windows";
  MENU.insert(info.ID, info);  
  //Custom Apps
  info = LPI(); //clear it
    info.name = QObject::tr("Custom App");
    info.description = QObject::tr("Start a custom application");
    info.ID = "app";
    info.icon = "application-x-desktop";
  MENU.insert(info.ID, info);
}

void LPlugins::LoadColorItems(){
  COLORS.clear();
  //Text Color
  LPI info;
    info.name = QObject::tr("Text");
    info.description = QObject::tr("Color to use for all visible text.");
    info.ID = "TEXTCOLOR";
  COLORS.insert(info.ID, info);
  //Text Color (Disabled)
  info = LPI(); //clear it
    info.name = QObject::tr("Text (Disabled)");
    info.description = QObject::tr("Text color for disabled or inactive items.");
    info.ID = "TEXTDISABLECOLOR";
  COLORS.insert(info.ID, info);
  //Text Color (Highlighted)
  info = LPI(); //clear it
    info.name = QObject::tr("Text (Highlighted)");
    info.description = QObject::tr("Text color when selection is highlighted.");
    info.ID = "TEXTHIGHLIGHTCOLOR";
  COLORS.insert(info.ID, info);
  //Base Color (Normal)
  info = LPI(); //clear it
    info.name = QObject::tr("Base Window Color");
    info.description = QObject::tr("Main background color for the window/dialog.");
    info.ID = "BASECOLOR";
  COLORS.insert(info.ID, info);
  //Base Color (Alternate)
  info = LPI(); //clear it
    info.name = QObject::tr("Base Window Color (Alternate)");
    info.description = QObject::tr("Main background color for widgets that list or display collections of items.");
    info.ID = "ALTBASECOLOR";
  COLORS.insert(info.ID, info);
  //Primary Color (Normal)
  info = LPI(); //clear it
    info.name = QObject::tr("Primary Color");
    info.description = QObject::tr("Dominant color for the theme.");
    info.ID = "PRIMARYCOLOR";
  COLORS.insert(info.ID, info);
  //Primary Color (Disabled)
  info = LPI(); //clear it
    info.name = QObject::tr("Primary Color (Disabled)");
    info.description = QObject::tr("Dominant color for the theme (more subdued).");
    info.ID = "PRIMARYDISABLECOLOR";
  COLORS.insert(info.ID, info);
  //Secondary Color (Normal)
  info = LPI(); //clear it
    info.name = QObject::tr("Secondary Color");
    info.description = QObject::tr("Alternate color for the theme.");
    info.ID = "SECONDARYCOLOR";
  COLORS.insert(info.ID, info);
  //Secondary Color (Disabled)
  info = LPI(); //clear it
    info.name = QObject::tr("Secondary Color (Disabled)");
    info.description = QObject::tr("Alternate color for the theme (more subdued).");
    info.ID = "SECONDARYDISABLECOLOR";
  COLORS.insert(info.ID, info);
  //Accent Color (Normal)
  info = LPI(); //clear it
    info.name = QObject::tr("Accent Color");
    info.description = QObject::tr("Color used for borders or other accents.");
    info.ID = "ACCENTCOLOR";
  COLORS.insert(info.ID, info);
  //Accent Color (Disabled)
  info = LPI(); //clear it
    info.name = QObject::tr("Accent Color (Disabled)");
    info.description = QObject::tr("Color used for borders or other accents (more subdued).");
    info.ID = "ACCENTDISABLECOLOR";
  COLORS.insert(info.ID, info);
  //Highlight Color (Normal)
  info = LPI(); //clear it
    info.name = QObject::tr("Highlight Color");
    info.description = QObject::tr("Color used for highlighting an item.");
    info.ID = "HIGHLIGHTCOLOR";
  COLORS.insert(info.ID, info);
  //Highlight Color (Disabled)
  info = LPI(); //clear it
    info.name = QObject::tr("Highlight Color (Disabled)");
    info.description = QObject::tr("Color used for highlighting an item (more subdued).");
    info.ID = "HIGHLIGHTDISABLECOLOR";
  COLORS.insert(info.ID, info);
}