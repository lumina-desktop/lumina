//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "NativeWindowSystem.h"
inline QScreen* screenUnderMouse(){
  QPoint pos = QCursor::pos();
  QList<QScreen*> scrns = QGuiApplication::screens();
  for(int i=0; i<scrns.length(); i++){
    if(scrns[i]->geometry().contains(pos)){ return scrns[i]; }
  }
  return 0;
}


//Primary/private  function
void NativeWindowSystem::arrangeWindows(NativeWindowObject *primary, QString type, bool primaryonly){
  if(type.isEmpty()){ type = "center"; }
  if(primary==0){
    //Get the currently active window and treat that as the primary
    for(int i=0; i<NWindows.length(); i++){
      if(NWindows[i]->property(NativeWindowObject::Active).toBool()){ primary = NWindows[i]; }
    }
    if(primary==0 && !NWindows.isEmpty()){ primary = NWindows[0]; } //just use the first one in the list
  }
  //Now get the current screen that the mouse cursor is over (and valid area)
  QScreen *screen = screenUnderMouse();
  if(screen==0){ return; } //should never happen (theoretically)
  QRect desktopArea = screen->availableGeometry();
  //qDebug() << "Arrange Windows:" << primary->geometry() << type << primaryonly << desktopArea;
  //Now start filtering out all the windows that need to be ignored
  int wkspace = primary->property(NativeWindowObject::Workspace).toInt();
  QList<NativeWindowObject*> winlist = NWindows;
  for(int i=0; i<winlist.length(); i++){
    if(winlist[i]->property(NativeWindowObject::Workspace).toInt()!=wkspace
	|| !winlist[i]->property(NativeWindowObject::Visible).toBool()
	|| desktopArea.intersected(winlist[i]->geometry()).isNull() ){
      //window is outside of the desired area or invisible - ignore it
      winlist.removeAt(i);
      i--;
    }
  }
  if(!winlist.contains(primary)){ winlist << primary; } //could be doing this on a window right before it is shown
  else if(primaryonly){ winlist.removeAll(primary); winlist << primary; } //move primary window to last
  //QRegion used;
  for(int i=0; i<winlist.length(); i++){
    if(primaryonly && winlist[i]!=primary){ continue; } //skip this window
    //Now loop over the windows and arrange them as needed
    QRect geom = winlist[i]->geometry();
    //verify that the window is contained by the desktop area
    if(geom.width()>desktopArea.width()){ geom.setWidth(desktopArea.width()); }
    if(geom.height()>desktopArea.height()){ geom.setHeight(desktopArea.height()); }
    //Now apply the proper placement routine
    if(type=="center"){
      QPoint ct = desktopArea.center();
      winlist[i]->setGeometryNow( QRect( ct.x()-(geom.width()/2), ct.y()-(geom.height()/2), geom.width(), geom.height()) );
    }else if(type=="snap"){

    }else if(type=="single_max"){
      winlist[i]->setGeometryNow( QRect( desktopArea.x(), desktopArea.y(), desktopArea.width(), desktopArea.height()) );
    }else if(type=="under-mouse"){
      QPoint ct = QCursor::pos();
      geom = QRect(ct.x()-(geom.width()/2), ct.y()-(geom.height()/2), geom.width(), geom.height() );
      //Now verify that the top of the window is still contained within the desktop area
      if(geom.y() < desktopArea.y() ){ geom.moveTop(desktopArea.y()); }
      winlist[i]->setGeometryNow(geom);

    }
    //qDebug() << " - New Geometry:" << winlist[i]->geometry();
  } //end loop over winlist
}

// ================
// Public slots for starting the arrangement routine(s) above
// ================
void NativeWindowSystem::ArrangeWindows(WId primary, QString type){
  NativeWindowObject* win = findWindow(primary);
  if(type.isEmpty()){ type = "center"; } //grab the default arrangement format
  arrangeWindows(win, type);
}

void NativeWindowSystem::TileWindows(WId primary, QString type){
  NativeWindowObject* win =  findWindow(primary);
  if(type.isEmpty()){ type = "single_max"; } //grab the default arrangement format for tiling
  arrangeWindows(win, type);
}

void NativeWindowSystem::CheckWindowPosition(WId id, bool newwindow){
  //used after a "drop" to validate/snap/re-arrange window(s) as needed
  // if "newwindow" is true, then this is the first-placement routine for a window before it initially appears
  NativeWindowObject* win = findWindow(id);
  CheckWindowPosition(win, newwindow);
}

void NativeWindowSystem::CheckWindowPosition(NativeWindowObject *win, bool newwindow){
  if(win==0){ return; } //invalid window
  QRect geom = win->geometry();
  qDebug() << "Got Window Geometry:" << geom;
  bool changed = false;
  //Make sure it is on the screen (quick check)
  if(geom.x() < 0){ changed = true; geom.moveLeft(0); }
  if(geom.y() < 0){ changed = true; geom.moveTop(0); }
  if(geom.width() < 20){ changed = true; geom.setWidth(100); }
  if(geom.height() < 20){ changed = true; geom.setHeight(100); }
  if(changed){ win->setGeometryNow(geom); }
  //Now run it through the window arrangement routine
  qDebug() << "ArrangeWindows";
  arrangeWindows(win, newwindow ?"center" : "snap", true);
}
