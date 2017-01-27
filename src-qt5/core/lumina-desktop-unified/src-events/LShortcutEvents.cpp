//===========================================
//  Lumina desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LShortcutEvents.h"
#include "global-objects.h"

// === PUBLIC ===
LShortcutEvents::LShortcutEvents(){
  WIN = 0;
  clearTimer = 0;
  evaluated = false;
}

LShortcutEvents::~LShortcutEvents(){

}

void LShortcutEvents::start(){
  if(clearTimer==0){
    clearTimer = new QTimer(this);
    clearTimer->setInterval(2000); //2 seconds
    clearTimer->setSingleShot(true);
    connect(clearTimer, SIGNAL(timeout()), this, SLOT(clearKeys()) );
  }
  //Now connect this object to the global EFILTER object signals
  connect(Lumina::EFILTER, SIGNAL(KeyPressed(WId, int)), this, SLOT(KeyPress(WId, int)) );
  connect(Lumina::EFILTER, SIGNAL(KeyReleased(WId, int)), this, SLOT(KeyRelease(WId, int)) );
  connect(Lumina::EFILTER, SIGNAL(MousePressed(WId, Lumina::MouseButton)), this, SLOT(MousePress(WId, Lumina::MouseButton)) );
  connect(Lumina::EFILTER, SIGNAL(MouseReleased(WId, Lumina::MouseButton)), this, SLOT(MouseRelease(WId, Lumina::MouseButton)) );
}

void LShortcutEvents::stop(){
  disconnect(Lumina::EFILTER, SIGNAL(KeyPressed(WId, int)), this, SLOT(KeyPress(WId, int)) );
  disconnect(Lumina::EFILTER, SIGNAL(KeyReleased(WId, int)), this, SLOT(KeyRelease(WId, int)) );
  disconnect(Lumina::EFILTER, SIGNAL(MousePressed(WId, Lumina::MouseButton)), this, SLOT(MousePress(WId, Lumina::MouseButton)) );
  disconnect(Lumina::EFILTER, SIGNAL(MouseReleased(WId, Lumina::MouseButton)), this, SLOT(MouseRelease(WId, Lumina::MouseButton)) );
  clearKeys();
}

// === PRIVATE ===
void LShortcutEvents::CheckKeySequence(WId win){
  //Get the keyboard modifiers
  QString shortcut = keylistToString();
 //Now see if there is a match for this shortcut
  //  "strict" actions (operate even if a non-desktop window is active/focused)
  QString action = Lumina::SETTINGS->value(DesktopSettings::Keys, "strict/"+shortcut, "").toString();
  qDebug() << "Strict Action:" << "strict/"+shortcut << action;
  if(action.isEmpty() && win==0){
    //"loose" actions (operating on the desktop or root window itself)
    action = Lumina::SETTINGS->value(DesktopSettings::Keys, "desktop/"+shortcut, "").toString();
    qDebug() << "Desktop Action:" << "desktop/"+shortcut << action;
  }
  if(!action.isEmpty()){
    //Found a valid action - go ahead and evaluate it
    evaluateShortcutAction(action);
  }
}

void LShortcutEvents::CheckMouseSequence(WId win, Lumina::MouseButton button, bool release){
  if(release && (button == Lumina::WheelUp || button == Lumina::WheelDown || button == Lumina::WheelLeft || button == Lumina::WheelRight)){ 
    return; //skip mouse release events for wheel actions (always come in pairs of press/release)
  }else if(keylist.isEmpty() || button == Lumina::NoButton){ return; } //Never overwrite mouse clicks themselves - just combinations with key presses
  //Get the keyboard modifiers
  QString shortcut = keylistToString();
  //Add the mouse button to the shortcut
  switch(button){
    case Lumina::LeftButton:
      shortcut.append("+LeftMouse");
      break;
    case Lumina::RightButton:
      shortcut.append("+RightMouse");
      break;
    case Lumina::MidButton:
      shortcut.append("+MiddleMouse");
      break;
    case Lumina::BackButton:
      shortcut.append("+BackMouse");
      break;
    case Lumina::ForwardButton:
      shortcut.append("+ForwardMouse");
      break;
    case Lumina::TaskButton:
      shortcut.append("+TaskMouse");
      break;
    case Lumina::WheelUp:
      shortcut.append("+WheelUp");
      break;
    case Lumina::WheelDown:
      shortcut.append("+WheelDown");
      break;
    case Lumina::WheelLeft:
      shortcut.append("+WheelLeft");
      break;
    case Lumina::WheelRight:
      shortcut.append("+WheelRight");
      break;
    default:
      shortcut.clear();
  }
  if(shortcut.isEmpty()){ return; }
  //Now see if there is a match for this shortcut
  //  "strict" actions (operate even if a non-desktop window is active/focused)
  QString action = Lumina::SETTINGS->value(DesktopSettings::Keys, "strict/"+shortcut, "").toString();
  if(action.isEmpty() && win==0){
    //"loose" actions (operating on the desktop or root window itself)
    action = Lumina::SETTINGS->value(DesktopSettings::Keys, "desktop/"+shortcut, "").toString();
  }
  if(!action.isEmpty()){
    //Found a valid action - go ahead and evaluate it
    evaluateShortcutAction(action);
  }
}

QString LShortcutEvents::keylistToString(){
  QString shortcut;
  for(int i=0; i<keylist.length(); i++){
    if(i>0){ shortcut.append("+"); }
    shortcut.append( QString::number(keylist[i]) );
  }
  /*qDebug() << "KeyList to String:";
  qDebug() << "  keys:" << keylist;
  qDebug() << "  string:" << shortcut;*/
  return shortcut;
}

void LShortcutEvents::evaluateShortcutAction(QString action){
  qDebug() << "Found Shortcut Action:" << action;
  evaluated = true;
  if(action.startsWith("Exec=")){
    emit LaunchApplication(action.section("=",1,-1));
    return;
  }
  //Specific Internal actions
  action = action.toLower();
  //Power Options
  if(action=="logout"){ emit StartLogout(); }
  else if(action=="reboot"){ emit StartReboot(); }
  else if(action=="shutdown"){ emit StartShutdown(); }
  else if(action=="show_leave_options"){ emit OpenLeaveDialog(); }

}

// === PUBLIC SLOTS ===
void LShortcutEvents::KeyPress(WId window, int key){
  if(window!=WIN){ keylist.clear(); WIN = window; }
  if(!keylist.contains(key)){ 
    //Put it in the list in ascending order
    bool found = false;
    for(int i=0; i<keylist.length() && !found; i++){
      if(keylist[i]>key){ keylist.insert(i,key); found = true; }
    }
    if(!found){ keylist << key;  }
    evaluated = false;
  }
  //Evaluate the key sequence only when the first one is released
  clearTimer->start(); //will "restart" if already running 
}

void LShortcutEvents::KeyRelease(WId window, int key){
  if(window!=WIN){ keylist.clear(); return; }
  if(!evaluated){ CheckKeySequence(WIN); } //run this "before" removing the key from the list
  keylist.removeAll(key);
  clearTimer->start(); //will "restart" if already running 
}

void LShortcutEvents::MousePress(WId window, Lumina::MouseButton button){
  //We do not provide shortcuts for combinations of mouse buttons - just mouse+keyboard combinations
  CheckMouseSequence(window, button, false);
  clearTimer->start(); //will "restart" if already running 
}

void LShortcutEvents::MouseRelease(WId window, Lumina::MouseButton button){
  //We do not provide shortcuts for combinations of mouse buttons - just mouse+keyboard combinations
  CheckMouseSequence(window, button, true);
  clearTimer->start(); //will "restart" if already running 
}

void LShortcutEvents::clearKeys(){
  keylist.clear();
  WIN = 0;
  if(clearTimer!=0){ clearTimer->stop(); }
}
