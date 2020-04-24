//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015-2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "BaseAnimGroup.h"

//Include all the known subclasses here, then add a unique ID for it to the functions at the bottom
//#include "SampleAnimation.h"
#include "Fireflies.h"
#include "Grav.h"
#include "SampleAnimation.h"
#include "Text.h"
#include "ImageSlideshow.h"
#include "VideoSlideshow.h"


QVariant BaseAnimGroup::readSetting(QString variable, QVariant defaultvalue){
  return DesktopSettings::instance()->value(DesktopSettings::ScreenSaver,
		 	"Animations/"+animPlugin+"/"+variable, defaultvalue);
}

//==============================
//     PLUGIN LOADING/LISTING
//==============================
BaseAnimGroup* BaseAnimGroup::NewAnimation(QString type, QWidget *parent){
  //This is where we place all the known plugin ID's, and load the associated subclass
  BaseAnimGroup *anim = 0;
  if(type=="fireflies"){
    anim = new FirefliesAnimation(parent);
  }else if(type == "grav") {
    anim = new GravAnimation(parent);
  }else if(type == "text") {
    anim = new TextAnimation(parent);
  }else if(type == "imageSlideshow") {
    anim = new ImageAnimation(parent);
  }else if(type == "videoSlideshow") {
    anim = new VideoAnimation(parent);
  }else {
    //Unknown screensaver, return a blank animation group
    anim = new BaseAnimGroup(parent);
  }
  //tag the animation with the type it is and return it
  if(anim!=0){ anim->animPlugin = type; }
  return anim;
}

QStringList BaseAnimGroup::KnownAnimations(){
  return (QStringList() << "none" << "grav" << "text" << "imageSlideshow" << "videoSlideshow" << "fireflies");
}
