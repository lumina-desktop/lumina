//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "getPage.h"

//Add any sub-pages here
#include "page_main.h"
#include "page_wallpaper.h"
//#include "page_theme.h"
#include "page_autostart.h"
#include "page_defaultapps.h"
#include "page_fluxbox_keys.h"
#include "page_fluxbox_settings.h"
#include "page_interface_desktop.h"
#include "page_interface_panels.h"
#include "page_interface_menu.h"
#include "page_session_locale.h"
#include "page_session_options.h"
#include "page_compton.h"
#include "page_soundtheme.h"

// #include "page_mouse_trueos.h"
// #include "page_bluetooth_trueos.h"

//Simplification function for creating a PAGEINFO structure
PAGEINFO Pages::PageInfo(QString ID, QString i_name, QString i_title, QString i_icon, QString i_comment, QString i_cat, QStringList i_sys, QStringList i_tags){
  PAGEINFO page;
  page.id = ID; page.name = i_name; page.title = i_title; page.icon = i_icon;
  page.comment = i_comment; page.category = i_cat; page.req_systems = i_sys;
  page.search_tags = i_tags;
  return page;
}

//List all the known pages
// **** Add new page entries here ****
QList<PAGEINFO> Pages::KnownPages(){
  // Valid Groups: ["appearance", "interface", "session", "user"]
  QList<PAGEINFO> list;
  //Reminder: <ID>, <name>, <title>, <icon>, <comment>, <category>, <server subsytem list>, <search tags>
  list << Pages::PageInfo("wallpaper", QObject::tr("Wallpaper"), QObject::tr("Wallpaper Settings"), "preferences-desktop-wallpaper",QObject::tr("Change background image(s)"), "appearance", QStringList(), QStringList() << "background" << "wallpaper" << "color" << "image");
  list << Pages::PageInfo(LUtils::AppToAbsolute("lthemeengine.desktop"), QObject::tr("Theme"), QObject::tr("Theme Settings"), "preferences-desktop-theme",QObject::tr("Change interface fonts and colors"), "appearance", QStringList(), QStringList() << "background" << "interface" << "color" << "theme" << "plugins");
  list << Pages::PageInfo("compton", QObject::tr("Window Effects"), QObject::tr("Window Effects"), "window-duplicate",QObject::tr("Adjust transparency levels and window effects"), "appearance", QStringList(), QStringList() << "background" << "interface" << "color" << "transparency" << "windows" << "compositing");
  list << Pages::PageInfo("autostart", QObject::tr("Autostart"), QObject::tr("Startup Settings"), "preferences-system-session-services",QObject::tr("Automatically start applications or services"), "session", QStringList(), QStringList() << "apps" << "autostart" << "services" << "xdg" << "startup" << "session");
  list << Pages::PageInfo("defaultapps", QObject::tr("Applications"), QObject::tr("Mimetype Settings"), "preferences-desktop-default-applications",QObject::tr("Change default applications"), "session", QStringList(), QStringList() << "apps" << "default" << "services" << "xdg" << "session");
  list << Pages::PageInfo("fluxbox-keys", QObject::tr("Keyboard Shortcuts"), QObject::tr("Keyboard Shortcuts"), "preferences-desktop-keyboard",QObject::tr("Change keyboard shortcuts"), "session", QStringList(), QStringList() << "apps" << "fluxbox" << "keys" << "keyboard" << "session" << "launch");
  list << Pages::PageInfo("fluxbox-settings", QObject::tr("Window Manager"), QObject::tr("Window Settings"), "preferences-system-windows-actions",QObject::tr("Change window settings and appearances"), "appearance", QStringList(), QStringList() << "window" << "frame" << "border" << "workspace" << "theme" << "fluxbox" << "session");
  list << Pages::PageInfo("interface-desktop", QObject::tr("Desktop"), QObject::tr("Desktop Plugins"), "preferences-desktop-icons",QObject::tr("Change what icons or tools are embedded on the desktop"), "interface", QStringList(), QStringList() << "desktop" << "plugins" << "embed" << "icons" << "utilities");
  list << Pages::PageInfo("interface-panel", QObject::tr("Panels"), QObject::tr("Panels and Plugins"), "configure-toolbars",QObject::tr("Change any floating panels and what they show"), "interface", QStringList(), QStringList() << "desktop" << "toolbar" << "panel" << "floating" << "plugins");
  list << Pages::PageInfo("interface-menu", QObject::tr("Menu"), QObject::tr("Menu Plugins"), "format-list-unordered",QObject::tr("Change what options are shown on the desktop context menu"), "interface", QStringList(), QStringList() << "desktop" << "menu" << "plugins" << "shortcuts");
  list << Pages::PageInfo("session-locale", QObject::tr("Localization"), QObject::tr("Locale Settings"), "preferences-desktop-locale",QObject::tr("Change the default locale settings for this user"), "user", QStringList(), QStringList() << "user"<<"locale"<<"language"<<"translations");
  list << Pages::PageInfo("session-options", QObject::tr("General Options"), QObject::tr("User Settings"), "configure",QObject::tr("Change basic user settings such as time/date formats"), "user", QStringList(), QStringList() << "user"<<"settings"<<"time"<<"date"<<"icon"<<"reset"<<"numlock"<<"clock");
  list << Pages::PageInfo("soundtheme", QObject::tr("Sound Themeing"), QObject::tr("Theme"), "media-playlist-audio",QObject::tr("Change basic sound settings"), "session", QStringList(), QStringList() << "session"<<"settings"<<"sound"<<"theme");
 // list << Pages::PageInfo("mouse-settings", QObject::tr("TrueOS Mouse Settings"), QObject::tr("TrueOS Mouse Settings"), "preferences-desktop-mouse",QObject::tr("Adjust mouse devices"), "user", QStringList(), QStringList() << "user"<<"speed"<<"accel"<<"mouse");
 // list << Pages::PageInfo("bluetooth-settings", QObject::tr("TrueOS Bluetooth Settings"), QObject::tr("TrueOS Bluetooth Settings"), "preferences-desktop-bluetooth",QObject::tr("Setup Bluetooth devices"), "user", QStringList(), QStringList() << "user"<<"bluetooth"<<"audio");

  return list;
}

PageWidget* Pages::GetNewPage(QString id, QWidget *parent){
  //Find the page that matches this "id"
  PageWidget* page = 0;
  if(id=="wallpaper"){ page = new page_wallpaper(parent); }
  //else if(id=="theme"){ page = new page_theme(parent); }
  else if(id=="autostart"){ page = new page_autostart(parent); }
  else if(id=="defaultapps"){ page = new page_defaultapps(parent); }
  else if(id=="fluxbox-keys"){ page = new page_fluxbox_keys(parent); }
  else if(id=="fluxbox-settings"){ page = new page_fluxbox_settings(parent); }
  else if(id=="interface-desktop"){ page = new page_interface_desktop(parent); }
  else if(id=="interface-panel"){ page = new page_interface_panels(parent); }
  else if(id=="interface-menu"){ page = new page_interface_menu(parent); }
  else if(id=="session-locale"){ page = new page_session_locale(parent); }
  else if(id=="session-options"){ page = new page_session_options(parent); }
  else if(id=="compton"){ page = new page_compton(parent); }
  else if(id=="soundtheme"){ page = new page_soundtheme(parent); }
 // else if(id=="mouse-settings"){ page = new page_mouse_trueos(parent); }
 // else if(id=="bluetooth-settings"){ page = new page_bluetooth_trueos(parent); }
  //Return the main control_panel page as the fallback/default
  if(page==0){ id.clear(); page = new page_main(parent); }
  page->setWhatsThis(id);
  return page;
}
