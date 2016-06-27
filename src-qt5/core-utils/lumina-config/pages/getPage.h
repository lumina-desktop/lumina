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
  return list;
}

//Add any sub-pages here
#include "page_main.h"
#include "page_wallpaper.h"
#include "page_theme.h"
#include "page_autostart.h"

static PageWidget* GetNewPage(QString id, QWidget *parent){
  //Find the page that matches this "id"
  if(id=="wallpaper"){ return new page_wallpaper(parent); }
  else if(id=="theme"){ return new page_theme(parent); }
  else if(id=="autostart"){ return new page_autostart(parent); }
  //Return the main control_panel page as the fallback/default
  return new page_main(parent);
}

#endif
