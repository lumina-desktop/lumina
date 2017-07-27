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
void RootWindow::ArrangeWindows(WId *primary, QString type){

}

void RootWindow::TileWindows(WId *primary, QString type){

}

void RootWindow::CheckWindowPosition(WId, bool newwindow){
  //used after a "drop" to validate/snap/re-arrange window(s) as needed
  // if "newwindow" is true, then this is the first-placement routine for a window before it initially appears
  
}
