//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "RootWindow.h"

//Primary/private  function
void RootWindow::arrangeWindows(RootSubWindow *primary, QString type, bool primaryonly){
  if(type.isEmpty()){ type = "center"; }
  if(primary==0){
    //Get the currently active window and treat that as the primary
    for(int i=0; i<WINDOWS.length(); i++){
      if(WINDOWS[i]->nativeWindow()->property(NativeWindow::Active).toBool()){ primary = WINDOWS[i]; }
    }
    if(primary==0 && !WINDOWS.isEmpty()){ primary = WINDOWS[0]; } //just use the first one in the list
  }
  //Now get the current screen that the mouse cursor is over (and valid area)
  QScreen *screen = screenUnderMouse();
  QRect desktopArea = screen->availableGeometry();
  //qDebug() << "Arrange Windows:" << primary->geometry() << type << primaryonly << desktopArea;
  //Now start filtering out all the windows that need to be ignored
  int wkspace = primary->nativeWindow()->property(NativeWindow::Workspace).toInt();
  QList<RootSubWindow*> winlist = WINDOWS;
  for(int i=0; i<winlist.length(); i++){
    if(winlist[i]->nativeWindow()->property(NativeWindow::Workspace).toInt()!=wkspace
	|| !winlist[i]->nativeWindow()->property(NativeWindow::Visible).toBool()
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
      winlist[i]->setGeometry( ct.x()-(geom.width()/2), ct.y()-(geom.height()/2), geom.width(), geom.height());
    }else if(type=="snap"){

    }else if(type=="single_max"){
      winlist[i]->setGeometry( desktopArea.x(), desktopArea.y(), desktopArea.width(), desktopArea.height());
    }else if(type=="under-mouse"){
      QPoint ct = QCursor::pos();
      geom = QRect(ct.x()-(geom.width()/2), ct.y()-(geom.height()/2), geom.width(), geom.height() );
      //Now verify that the top of the window is still contained within the desktop area
      if(geom.y() < desktopArea.y() ){ geom.moveTop(desktopArea.y()); }
      winlist[i]->setGeometry(geom);

    }
    //qDebug() << " - New Geometry:" << winlist[i]->geometry();
  } //end loop over winlist
}

// ================
// Public slots for starting the arrangement routine(s) above
// ================
void RootWindow::ArrangeWindows(WId primary, QString type){
  RootSubWindow* win = windowForId(primary);
  if(type.isEmpty()){ type = "center"; } //grab the default arrangement format
  arrangeWindows(win, type);
}

void RootWindow::TileWindows(WId primary, QString type){
  RootSubWindow* win = windowForId(primary);
  if(type.isEmpty()){ type = "single_max"; } //grab the default arrangement format for tiling
  arrangeWindows(win, type);
}

void RootWindow::CheckWindowPosition(WId id, bool newwindow){
  //used after a "drop" to validate/snap/re-arrange window(s) as needed
  // if "newwindow" is true, then this is the first-placement routine for a window before it initially appears
  RootSubWindow* win = windowForId(id);
  if(win==0){ return; } //invalid window
  QRect geom = win->nativeWindow()->geometry();
  bool changed = false;
  //Make sure it is on the screen (quick check)
  if(geom.x() < 0){ changed = true; geom.moveLeft(0); }
  if(geom.y() < 0){ changed = true; geom.moveTop(0); }
  if(geom.width() < 20){ changed = true; geom.setWidth(100); }
  if(geom.height() < 20){ changed = true; geom.setHeight(100); }
  if(changed){ win->setGeometry(geom); }
  //Now run it through the window arrangement routine
  arrangeWindows(win, newwindow ?"center" : "snap", true);
}
