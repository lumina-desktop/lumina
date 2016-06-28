//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_CONFIG_PAGE_WIDGET_FETCH_H
#define _LUMINA_CONFIG_PAGE_WIDGET_FETCH_H

#include "../globals.h"
#include "PageWidget.h"


//Simplification function for creating a PAGEINFO structure
static PAGEINFO PageInfo(QString ID, QString i_name, QString i_title, QString i_icon, QString i_comment, QString i_cat, QStringList i_sys, QStringList i_tags){
  PAGEINFO page;
  page.id = ID; page.name = i_name; page.title = i_title; page.icon = i_icon;
  page.comment = i_comment; page.category = i_cat; page.req_systems = i_sys;
  page.search_tags = i_tags;
  return page;
}

//List all the known pages
// **** Add new page entries here ****
static QList<PAGEINFO> KnownPages(){
  // Valid Groups: ["appearance", "interface", "session", "apps"]
  QList<PAGEINFO> list;
  //Reminder: <ID>, <name>, <title>, <icon>, <comment>, <category>, <server subsytem list>, <search tags>
  list << PageInfo("wallpaper", QObject::tr("Change Wallpaper"), QObject::tr("Wallpaper Settings"), "preferences-desktop-wallpaper",QObject::tr("Change background image(s)"), "appearance", QStringList(), QStringList() << "background" << "wallpaper" << "color" << "image");
  list << PageInfo("theme", QObject::tr("Change Desktop Theme"), QObject::tr("Theme Settings"), "preferences-desktop-theme",QObject::tr("Change interface fonts and colors"), "appearance", QStringList(), QStringList() << "background" << "interface" << "color" << "theme" << "plugins");
  list << PageInfo("autostart", QObject::tr("Startup Services and Applications"), QObject::tr("Startup Settings"), "preferences-system-session-services",QObject::tr("Automatically start applications or services"), "session", QStringList(), QStringList() << "apps" << "autostart" << "services" << "xdg" << "startup" << "session");
  list << PageInfo("defaultapps", QObject::tr("Default Applications for File Type"), QObject::tr("Mimetype Settings"), "preferences-desktop-default-applications",QObject::tr("Change default applications"), "session", QStringList(), QStringList() << "apps" << "default" << "services" << "xdg" << "session");
  list << PageInfo("fluxbox-keys", QObject::tr("Keyboard Shortcuts"), QObject::tr("Keyboard Shortcuts"), "preferences-desktop-keyboard",QObject::tr("Change keyboard shortcuts"), "session", QStringList(), QStringList() << "apps" << "fluxbox" << "keys" << "keyboard" << "session" << "launch");
  list << PageInfo("fluxbox-settings", QObject::tr("Window Manager"), QObject::tr("Window Settings"), "preferences-system-windows",QObject::tr("Change window settings and appearances"), "appearance", QStringList(), QStringList() << "window" << "frame" << "border" << "workspace" << "theme" << "fluxbox" << "session");
  list << PageInfo("interface-desktop", QObject::tr("Desktop Icons and Plugins"), QObject::tr("Desktop Plugins"), "preferences-desktop-icons",QObject::tr("Change what icons or tools are embedded on the desktop"), "interface", QStringList(), QStringList() << "desktop" << "plugins" << "embed" << "icons" << "utilities");
  list << PageInfo("interface-panel", QObject::tr("Floating Panels and Plugins"), QObject::tr("Panels and Plugins"), "configure-toolbars",QObject::tr("Change any floating panels and what they show"), "interface", QStringList(), QStringList() << "desktop" << "toolbar" << "panel" << "floating" << "plugins");
  list << PageInfo("interface-menu", QObject::tr("Context Menu and Plugins"), QObject::tr("Menu Plugins"), "preferences-plugin",QObject::tr("Change what options are shown on the desktop context menu"), "interface", QStringList(), QStringList() << "desktop" << "menu" << "plugins" << "shortcuts");
  return list;
}

//Add any sub-pages here
#include "page_main.h"
#include "page_wallpaper.h"
#include "page_theme.h"
#include "page_autostart.h"
#include "page_defaultapps.h"
#include "page_fluxbox_keys.h"
#include "page_fluxbox_settings.h"
#include "page_interface_desktop.h"
//#include "page_interface_panels.h"
//#include "page_interface_menu.h"

static PageWidget* GetNewPage(QString id, QWidget *parent){
  //Find the page that matches this "id"
  if(id=="wallpaper"){ return new page_wallpaper(parent); }
  else if(id=="theme"){ return new page_theme(parent); }
  else if(id=="autostart"){ return new page_autostart(parent); }
  else if(id=="defaultapps"){ return new page_defaultapps(parent); }
  else if(id=="fluxbox-keys"){ return new page_fluxbox_keys(parent); }
  else if(id=="fluxbox-settings"){ return new page_fluxbox_settings(parent); }
  else if(id=="interface-desktop"){ return new page_interface_desktop(parent); }
//  else if(id=="interface-panel"){ return new page_interface_panels(parent); }
//  else if(id=="interface-menu"){ return new page_interface_menu(parent); }
  //Return the main control_panel page as the fallback/default
  return new page_main(parent);
}

#endif
