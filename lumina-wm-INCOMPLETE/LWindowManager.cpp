//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LWindowManager.h"

#define DEBUG 1

LWindowManager::LWindowManager(){
	
}

LWindowManager::~LWindowManager(){
	
}

bool LWindowManager::start(){
  //Should probably do a quick loop over any existing windows with the root as parent (just in case)
  QList<WId> initial = LWM::SYSTEM->WM_RootWindows();
  for(int i=0; i<initial.length(); i++){
    NewWindow(initial[i], false); //These ones did not explicitly request to be mapped
  }
  RestackWindows();
  return true;
}

void LWindowManager::stop(){
  for(int i=0; i<WINS.length(); i++){
    if(WINS[i]->hasFrame()){
      LWM::SYSTEM->UnembedWindow(WINS[i]->clientID());
      WINS[i]->frame()->close();
    }
  }
}
//===============
//   PUBLIC SLOTS
//===============
void LWindowManager::NewWindow(WId win, bool requested){
  //Verify that this window can/should be managed first
  //if(DEBUG){ qDebug() << "New Window:" << LWM::SYSTEM->WM_ICCCM_GetClass(win); }
  QString wclass = LWM::SYSTEM->WM_ICCCM_GetClass(win);
  if( wclass.contains("lumina-wm",Qt::CaseInsensitive) ){ return; } //just in case: prevent recursion
  else{
    bool ok = (wclass.isEmpty() ? false : LWM::SYSTEM->WM_ManageWindow(win, requested) );
    if(!ok){
      //See if this window is just a transient pointing to some other window
      WId tran = LWM::SYSTEM->WM_ICCCM_GetTransientFor(win);
      if(tran!=win && tran!=0){ 
        win = tran; 
	ok = LWM::SYSTEM->WM_ManageWindow(win); 
      }
    }
    if(!ok){ return;  }
  }
  if(DEBUG){ qDebug() << "New Managed Window:" << LWM::SYSTEM->WM_ICCCM_GetClass(win); }
  LWindow *lwin = new LWindow(win);
    connect(lwin, SIGNAL(Finished(WId)), this, SLOT(FinishedWindow(WId)) );
  WINS << lwin;
  if(lwin->hasFrame()){
    lwin->frame()->windowChanged(LWM::Show); //Make sure to show it right away
  }else{
    LWM::SYSTEM->WM_ShowWindow(win); //just map the window right now
  }
}

void LWindowManager::ClosedWindow(WId win){
  for(int i=0; i<WINS.length(); i++){
    if(WINS[i]->clientID()==win){
      qDebug() << " - Closed Window";
      if(WINS[i]->hasFrame()){ WINS[i]->frame()->windowChanged(LWM::Closed); }
      else{ 
	FinishedWindow(win); 
      }
      break;
    }
  }
}

void LWindowManager::ModifyWindow(WId win, LWM::WindowAction act){
  for(int i=0; i<WINS.length(); i++){
    if(WINS[i]->clientID()==win){
      if(WINS[i]->hasFrame()){ WINS[i]->frame()->windowChanged(act); }
      return;
    }
  }
  //If it gets this far - it is an unmanaged window
  if(act==LWM::Show){
    NewWindow(win);
  }
  RestackWindows();
}

void LWindowManager::RestackWindows(){
  Stack_Desktop.clear(); Stack_Below.clear(); Stack_Normal.clear(); Stack_Above.clear(); Stack_Fullscreen.clear();
  QList<WId> currwins;
  int cwork = LWM::SYSTEM->WM_Get_Current_Desktop();
  int winwork = -1;
  QList<LXCB::WINDOWSTATE> states;
  QList<LXCB::WINDOWTYPE> types;
  for(int i=0; i<WINS.length(); i++){
    //Only show windows on the current desktop
    winwork = LWM::SYSTEM->WM_Get_Desktop(WINS[i]->clientID());
    states = LWM::SYSTEM->WM_Get_Window_States(WINS[i]->clientID());
    types = LWM::SYSTEM->WM_Get_Window_Type(WINS[i]->clientID());
    WId id = WINS[i]->clientID();
      if(WINS[i]->hasFrame()){ id = WINS[i]->frame()->winId(); }
    if(winwork<0 || winwork == cwork || states.contains(LXCB::S_STICKY) ){
      //Now check the state/type and put it in the proper stack
      currwins << WINS[i]->clientID(); //add this to the overall "age" list
      //Now add it to the proper stack
      if(types.contains(LXCB::T_DESKTOP)){ Stack_Desktop << id; }
      else if(states.contains(LXCB::S_BELOW)){ Stack_Below << id; }
      else if(types.contains(LXCB::T_DOCK) || states.contains(LXCB::S_ABOVE) ){ Stack_Above << id; }
      else if(states.contains(LXCB::S_FULLSCREEN)){ Stack_Fullscreen << id; }
      else{ Stack_Normal << id; }
    }
  }
  //Now set the root properties for these lists
  LWM::SYSTEM->WM_Set_Client_List(currwins, false); //age-ordered version
  LWM::SYSTEM->WM_Set_Client_List(QList<WId>() << Stack_Desktop << Stack_Below << Stack_Normal << Stack_Above << Stack_Fullscreen, true); //stacking order version
  //Now re-paint (in order) the windows
  RepaintWindows();
}

void LWindowManager::RepaintWindows(){
  //Go through all the current windows (in stacking order) and re-paint them
  for(int i=0; i<Stack_Desktop.length(); i++){
    LWM::SYSTEM->WM_ShowWindow(Stack_Desktop[i]);
  }	  
  for(int i=0; i<Stack_Below.length(); i++){
    LWM::SYSTEM->WM_ShowWindow(Stack_Below[i]);
  }
  for(int i=0; i<Stack_Normal.length(); i++){
    LWM::SYSTEM->WM_ShowWindow(Stack_Normal[i]);
  }
  for(int i=0; i<Stack_Above.length(); i++){
    LWM::SYSTEM->WM_ShowWindow(Stack_Above[i]);
  }
  for(int i=0; i<Stack_Fullscreen.length(); i++){
    LWM::SYSTEM->WM_ShowWindow(Stack_Fullscreen[i]);
  }
}

//=================
//   PRIVATE SLOTS
//=================
void LWindowManager::FinishedWindow(WId win){
  for(int i=0; i<WINS.length(); i++){
    if(WINS[i]->clientID() == win){ 
      qDebug() << " - Finished Window"; 
      delete WINS.takeAt(i); break; 
    }
  }
  //Now update the list of clients
  RestackWindows();
}
