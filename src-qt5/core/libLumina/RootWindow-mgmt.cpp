//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "RootWindow.h"

//Primary/private  function
void RootWindow::arrangeWindows(RootSubWindow *primary, QString type){
  if(primary==0){
    //Get the currently active window and treat that as the primary

  }
  //Now loop over the windows and arrange them as needed

}

// ================
// Public slots for starting the arrangement routine(s) above
// ================
void RootWindow::ArrangeWindows(WId primary, QString type){
  RootSubWindow* win = windowForId(primary);
  if(type.isEmpty()){ type = ""; } //grab the default arrangement format
  arrangeWindows(win, type);
}

void RootWindow::TileWindows(WId primary, QString type){
  RootSubWindow* win = windowForId(primary);
  if(type.isEmpty()){ type = ""; } //grab the default arrangement format for tiling
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
  ArrangeWindows(id);
}
