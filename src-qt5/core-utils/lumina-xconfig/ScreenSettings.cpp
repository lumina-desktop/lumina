//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "ScreenSettings.h"
#include <LUtils.h>
#include <QDebug>
#include <QSettings>

//Reset current screen config to match previously-saved settings
void RRSettings::ApplyPrevious(){
  QSettings set("lumina-desktop","lumina-xconfig");
  set.beginGroup("MonitorSettings");
  //Setup a couple lists
  QStringList devs = set.childGroups(); //known/saved devices
  QList<ScreenInfo> screens = RRSettings::CurrentScreens();
  QStringList lastactive = set.value("lastActive",QStringList()).toStringList();
  //Now go through all the saved settings and put that info into the array
  QString primary;
  QStringList avail;
  for(int i=0; i<screens.length(); i++){
    //if(screens[i].order>=0){screens[i].order = -1; } //reset all screen orders (need to re-check all)
    if(devs.contains(screens[i].ID) && screens[i].isavailable){ //only load settings for monitors which are currently attached
      set.beginGroup(screens[i].ID);
        screens[i].geom = set.value("geometry", QRect()).toRect();
        screens[i].isprimary = set.value("isprimary", false).toBool();
        if(screens[i].isprimary){ primary = screens[i].ID; }
        screens[i].isactive = lastactive.contains(screens[i].ID);
        screens[i].order = (screens[i].isactive ? -1 : -3); //check/ignore
      set.endGroup();
    }else if(screens[i].isavailable){
      screens[i].order = -2; //needs activation/placement
    }else{
      screens[i].order = -3; //ignored
    }
    //Now clean up the list as needed
    if(screens[i].order < -2){ screens.removeAt(i); i--; } //just remove it (less to loop through later)
    else{ avail << screens[i].ID; } //needed for some checks later - make it simple
  }
  //NOTE ABOUT orders: -1: check geom, -2: auto-add to end, -3: ignored
  
  //Quick checks for simple systems - just use current X config as-is
  if(devs.isEmpty() && (avail.filter("LVDS").isEmpty() || screens.length()==1) ){ return; } 

  //Typical ID's: LVDS-[], DVI-I-[], DP-[], HDMI-[], VGA-[]
  //"LVDS" is the built-in laptop display normally
  if(primary.isEmpty()){
    QStringList priority; priority << "LVDS" << "DP" << "HDMI" << "DVI" << "VGA";
    for(int i=0; i<priority.length() && primary.isEmpty(); i++){
      QStringList filter = avail.filter(priority[i]);
      if(!filter.isEmpty()){ filter.sort(); primary = filter.first(); }
    }
    if(primary.isEmpty()){ primary = avail.first(); }
  }
  //Ensure only one monitor is primary, and reset a few flags
  for(int i=0; i<screens.length(); i++){  
    if(screens[i].ID!=primary){ screens[i].isprimary = false; }  
    screens[i].isactive = true; //we want all these monitors to be active eventually
  }
  // Handle all the available monitors
  int handled = 0;
  int cx = 0; //current x point
  while(handled<screens.length()){
    //Go through horizontally and place monitors (TO-DO: Vertical placement not handled yet)
    int next = -1;
    int diff = -1;
    for(int i=0; i<screens.length(); i++){
      if(screens[i].order==-1){
        if(diff<0 || ((screens[i].geom.x()-cx) < diff)){
          diff = screens[i].geom.x()-cx;
          next = i;
        }
      }
    }//end loop over screens
    if(next<0){
      //Go through and start adding the non-assigned screens to the end
      for(int i=0; i<screens.length(); i++){
        if(screens[i].order==-2){
          if(diff<0 || ((screens[i].geom.x()-cx) < diff)){
            diff = screens[i].geom.x()-cx;
            next = i;
          }
        }
      } //end loop over screens
    }
    if(next>=0){ 
      cx+=screens[next].geom.width();
      screens[next].order = handled; handled++; 
    }else{
      //Still missing monitors (vertical alignment?)
      qDebug() << "Unhandled Monitors:" << screens.length()-handled;
      break;
    }
  }
  //Now reset the display with xrandr
  RRSettings::Apply(screens);
}

//Read the current screen config from xrandr
QList<ScreenInfo> RRSettings::CurrentScreens(){
  QList<ScreenInfo> SCREENS;
  QStringList info = LUtils::getCmdOutput("xrandr -q");
  ScreenInfo cscreen;
  for(int i=0; i<info.length(); i++){
    if(info[i].contains("connected") ){
      //qDebug() << "xrandr info:" << info[i];
      if(!cscreen.ID.isEmpty()){ 
	SCREENS << cscreen; //current screen finished - save it into the array
	cscreen = ScreenInfo(); //Now create a new structure      
      } 
      //qDebug() << "Line:" << info[i];
      QString dev = info[i].section(" ",0,0); //device ID
      //The device resolution can be either the 3rd or 4th output - check both
      QString devres = info[i].section(" ",2,2, QString::SectionSkipEmpty);
      if(!devres.contains("x")){ devres = info[i].section(" ",3,3,QString::SectionSkipEmpty); }
      if(!devres.contains("x")){ devres.clear(); }
      qDebug() << " - ID:" <<dev << "Current Geometry:" << devres;
      //qDebug() << " - Res:" << devres;
      if( !devres.contains("x") || !devres.contains("+") ){ devres.clear(); }
      //qDebug() << " - Res (modified):" << devres;
      if(info[i].contains(" disconnected ") && !devres.isEmpty() ){
        //Device disconnected, but still active on X
	cscreen.isavailable = false;
        cscreen.isactive = true;
     }else if( !devres.isEmpty() ){
        cscreen.isprimary = info[i].contains(" primary ");
	//Device that is connected and attached (has a resolution)
	qDebug() << "Create new Screen entry:" << dev << devres;
	cscreen.ID = dev;
	//Note: devres format: "<width>x<height>+<xoffset>+<yoffset>"
	cscreen.geom.setRect( devres.section("+",-2,-2).toInt(), devres.section("+",-1,-1).toInt(), devres.section("x",0,0).toInt(), devres.section("+",0,0).section("x",1,1).toInt() ); 
	cscreen.isavailable = true;
        cscreen.isactive = true;
      }else if(info[i].contains(" connected")){
        //Device that is connected, but not attached
	qDebug() << "Create new Screen entry:" << dev << "none";
	cscreen.ID = dev;
	cscreen.order = -2; //flag this right now as a non-active screen
        cscreen.isavailable = true;
        cscreen.isactive = false;
      }
    }else if( !cscreen.ID.isEmpty() && info[i].section("\t",0,0,QString::SectionSkipEmpty).contains("x")){
      //available resolution for a device
      cscreen.resList << info[i].section("\t",0,0,QString::SectionSkipEmpty);
    }
  } //end loop over primary info lines
  if(!cscreen.ID.isEmpty()){ SCREENS << cscreen; } //make sure to add the last screen to the array
  return SCREENS;
}

//Save the screen config for later
bool RRSettings::SaveScreens(QList<ScreenInfo> screens){
  QSettings set("lumina-desktop","lumina-xconfig");
  set.beginGroup("MonitorSettings");
  //Setup a couple lists
  QStringList olddevs = set.childGroups();
  QStringList active;
  //Now go through all the current screens and save that info
  for(int i=0; i<screens.length(); i++){
    olddevs.removeAll(screens[i].ID); //this is still a valid device
    if(screens[i].isactive){ active << screens[i].ID; }
    set.beginGroup(screens[i].ID);
      set.setValue("geometry", screens[i].geom);
      set.setValue("isprimary", screens[i].isprimary);
    set.endGroup();
  }
  set.setValue("lastActive",active);
  //Clean up any old device settings (no longer available for this hardware)
  for(int i=0; i<olddevs.length(); i++){
    set.remove(olddevs[i]);
  }
  return true;
}
	
//Apply screen configuration
void RRSettings::Apply(QList<ScreenInfo> screens){
  //Read all the settings and create the xrandr options to maintain these settings
  QStringList opts;
  qDebug() << "Apply:" << screens.length();
  for(int i=0; i<screens.length(); i++){
    qDebug() << " -- Screen:" << i << screens[i].ID << screens[i].isactive << screens[i].order;
    if(screens[i].order <0 || !screens[i].isactive){ continue; } //skip this screen - non-active
    opts << "--output" << screens[i].ID << "--mode" << QString::number(screens[i].geom.width())+"x"+QString::number(screens[i].geom.height());
    opts << "--pos" << QString::number(screens[i].geom.x())+"x"+QString::number(screens[i].geom.y());
    if(screens[i].isprimary){ opts << "--primary"; }
  }
  qDebug() << "Run command: xrandr" << opts;
  LUtils::runCmd("xrandr", opts);
}
