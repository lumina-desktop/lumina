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
  QList<ScreenInfo> screens;
  QSettings set("lumina-desktop","lumina-xconfig");
  if(set.allKeys().isEmpty()){ return; }
  QString profile = set.value("default_profile","").toString();
  if(profile.isEmpty() || !savedProfiles().contains(profile) ){ screens = PreviousSettings(); }
  else{ screens = PreviousSettings(profile); }
  //Now reset the display with xrandr
  RRSettings::Apply(screens);
}

void RRSettings::ApplyProfile(QString profile){
  //Make sure the profile exists first
  if( !RRSettings::savedProfiles().contains(profile) ){ return; }
  //Now load/apply the profile
  QList<ScreenInfo> screens = RRSettings::PreviousSettings(profile);
  RRSettings::Apply(screens);
}

void RRSettings::MirrorAll(){
  QList<ScreenInfo> screens = RRSettings::CurrentScreens();
  //Now find the highest resolution supported by all connected monitors
  QStringList combined;
  bool starting = true;
  for(int i=0; i<screens.length(); i++){
    //qDebug() << "Got Screen:" << screens[i].ID << screens[i].isavailable << screens[i].isactive << screens[i].resList;
    if(!screens[i].isavailable){ continue; }
    if(combined.isEmpty() && starting){ combined = screens[i].resList; starting = false; }
    else{
      for(int j=0; j<combined.length(); j++){
        QString res = combined[j].section(" ",0,0, QString::SectionSkipEmpty);
        if( screens[i].resList.filter(res).isEmpty() ){
          combined.removeAt(j); j--;
        }
      }
    }
  }
  //Get the highest res in the combined list
  if(combined.isEmpty()){
    qDebug() << "Could not find any common resolutions between the available monitors!";
    return;
  }
  //qDebug() << "Got combined res list:" << combined;
  QSize max;
  for(int i=0; i<combined.length(); i++){
    QString res = combined[i].section(" ",0,0, QString::SectionSkipEmpty);
    QSize tmp(res.section("x",0,0).toInt(), res.section("x",1,1).toInt() );
    if(tmp.width()>=max.width() && tmp.height()>=max.height()){ max = tmp; }
  }
  qDebug() << "Detected max unified resolution:" << max;
  //Setup one monitor with that resolution, and mirror the rest of them
  for(int i=0; i<screens.length(); i++){
    if(!screens[i].isavailable){ continue; } //skip it, not available at the moment
    screens[i].geom = QRect(QPoint(0,0),max);
    screens[i].rotation = 0;
    if(!screens[i].isactive){ screens[i].applyChange = 2; } //activate it
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
      //Pull the monitor rotation mode out as well (last word before the parenthesis)
      QString devrotate = info[i].section("(",0,0).split(" ",QString::SkipEmptyParts).last();
      //qDebug() << " - ID:" <<dev << "Current Geometry:" << devres;
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
	//qDebug() << "Create new Screen entry:" << dev << devres << devrotate << info[i].section("(",0,0);
	cscreen.ID = dev;
	//Note: devres format: "<width>x<height>+<xoffset>+<yoffset>"
	cscreen.geom.setRect( devres.section("+",-2,-2).toInt(), devres.section("+",-1,-1).toInt(), devres.section("x",0,0).toInt(), devres.section("+",0,0).section("x",1,1).toInt() ); 
	cscreen.isavailable = true;
        cscreen.isactive = true;
        if(devrotate=="left"){ cscreen.rotation = -90; }
        else if(devrotate=="right"){ cscreen.rotation = 90; }
        else if(devrotate=="inverted"){ cscreen.rotation = 180; }
        else{ cscreen.rotation = 0; }
      }else if(info[i].contains(" connected")){
        //Device that is connected, but not attached
	//qDebug() << "Create new Screen entry:" << dev << "none";
	cscreen.ID = dev;
	//cscreen.order = -2; //flag this right now as a non-active screen
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

QList<ScreenInfo> RRSettings::PreviousSettings(QString profile){
  QSettings set("lumina-desktop","lumina-xconfig");
  if(profile.isEmpty()){ set.beginGroup("MonitorSettings"); }
  else{ set.beginGroup("MonitorProfiles/"+profile); }
  //Setup a couple lists
  QStringList devs = set.childGroups(); //known/saved devices
  QList<ScreenInfo> screens = RRSettings::CurrentScreens();
  QStringList lastactive = set.value("lastActive",QStringList()).toStringList();
  //Now go through all the saved settings and put that info into the array
  QString primary;
  QStringList avail;
  for(int i=0; i<screens.length(); i++){
    if(devs.contains(screens[i].ID) && screens[i].isavailable){ //only load settings for monitors which are currently attached
      avail << screens[i].ID; //currently available
      set.beginGroup(screens[i].ID);
        screens[i].geom = set.value("geometry", QRect()).toRect();
        screens[i].isprimary = set.value("isprimary", false).toBool();
        if(screens[i].isprimary){ primary = screens[i].ID; }
        screens[i].applyChange = (screens[i].isactive && !lastactive.contains(screens[i].ID) ? 1 : 0); //disable/ignore
        screens[i].rotation = set.value("rotation",0).toInt();
      set.endGroup();
    }else if(screens[i].isactive){
      screens[i].applyChange = 1; //disable monitor - not enabled in the default settings
    }
  }

  //Quick checks for simple systems - just use current X config as-is
  if(devs.isEmpty() && (avail.filter("LVDS").isEmpty() || screens.length()==1) ){ return screens; }

  //Typical ID's: LVDS-[], DVI-I-[], DP-[], HDMI-[], VGA-[]
  //"LVDS" or "eDP" is the built-in laptop display normally
  if(primary.isEmpty()){
    QStringList priority; priority << "LVDS" << "eDP" << "DP" << "HDMI" << "DVI" << "VGA";
    for(int i=0; i<priority.length() && primary.isEmpty(); i++){
      QStringList filter = avail.filter(priority[i]);
      if(!filter.isEmpty()){ filter.sort(); primary = filter.first(); }
    }
    if(primary.isEmpty() && !avail.isEmpty()){ primary = avail.first(); }
  }
  //Ensure only one monitor is primary, and reset a few flags
  for(int i=0; i<screens.length(); i++){
    if(screens[i].ID!=primary){ screens[i].isprimary = false; }
  }
  return screens;
}

QStringList RRSettings::savedProfiles(){
  QSettings set("lumina-desktop","lumina-xconfig");
  set.beginGroup("MonitorProfiles");
  return set.childGroups();
}

void RRSettings::removeProfile(QString profile){
  QSettings set("lumina-desktop","lumina-xconfig");
  set.beginGroup("MonitorProfiles");
  QStringList known = set.childGroups();
  if(known.contains(profile) && !profile.isEmpty()){
    set.remove(profile);
  }
}

//Save the screen config for later
bool RRSettings::SaveScreens(QList<ScreenInfo> screens, QString profile){
  QSettings set("lumina-desktop","lumina-xconfig");
  if(profile.isEmpty()){ set.beginGroup("MonitorSettings"); }
  else{ set.beginGroup("MonitorProfiles/"+profile); }
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
      set.setValue("rotation", screens[i].rotation);
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
  //Verify that there is at least 1 active/enabled monitor first
  bool foundactive = false;
  for(int i=0; i<screens.length() && !foundactive; i++){
    if(screens[i].isactive){ foundactive = (screens[i].applyChange!=1); } //make sure we are not turning it off
    else{ foundactive = (screens[i].applyChange==2); }
  }
  if(!foundactive){ return; } //never disable all screens
  //Read all the settings and create the xrandr options to maintain these settings
  QStringList opts;
  //qDebug() << "Apply:" << screens.length();
  for(int i=0; i<screens.length(); i++){
    qDebug() << " -- Screen:" << i << screens[i].ID << screens[i].isactive;
    if( !screens[i].isactive){ continue; } //skip this screen - non-active
    else if(screens[i].applyChange==1){ opts << "--output" << screens[i].ID << "--off"; continue; } //deactivate a screen
    opts << "--output" << screens[i].ID;
    if(screens[i].applyChange==2){ opts << "--auto"; }
    else{ opts << "--mode" << QString::number(screens[i].geom.width())+"x"+QString::number(screens[i].geom.height()); }
    opts << "--pos" << QString::number(screens[i].geom.x())+"x"+QString::number(screens[i].geom.y());
      if(screens[i].rotation==-90){ opts << "--rotate" << "left"; }
      else if(screens[i].rotation==90){ opts << "--rotate" << "right"; }
      else if(screens[i].rotation==180){ opts << "--rotate" << "inverted"; }
      else{ opts << "--rotate" << "normal"; }
    if(screens[i].isprimary){ opts << "--primary"; }
  }
  qDebug() << "Run command: xrandr" << opts;
  LUtils::runCmd("xrandr", opts);
}
