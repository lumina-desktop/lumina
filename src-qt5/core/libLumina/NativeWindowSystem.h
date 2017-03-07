//===========================================
//  Lumina-DE source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is a Qt5/Lumina wrapper around native graphics system calls
//  It is primarily designed around the creation/modification of instances of
//   the "NativeWindow" class for passing information around
//===========================================
#ifndef _LUMINA_NATIVE_WINDOW_SYSTEM_H
#define _LUMINA_NATIVE_WINDOW_SYSTEM_H

#include "NativeWindow.h"
#include <QDateTime>
#include <QTimer>

class NativeWindowSystem : public QObject{
	Q_OBJECT
private:
	QList<NativeWindow*> NWindows;
	QList<NativeWindow*> TWindows;

	//Simplifications to find an already-created window object
	NativeWindow* findWindow(WId id){
	  for(int i=0; i<NWindows.length(); i++){ 
	    if(id==NWindows[i]->id()){ return NWindows[i]; } 
	  }
	  return 0;
	}

	NativeWindow* findTrayWindow(WId id){
	  for(int i=0; i<TWindows.length(); i++){ 
	    if(id==TWindows[i]->id()){ return TWindows[i]; } 
	  }
	  return 0;
	}

	//Now define a simple private_objects class so that each implementation 
	//  has a storage container for defining/placing private objects as needed
	class p_objects;
	p_objects* obj;

	//Internal timers/variables for managing pings
	QTimer *pingTimer;
	QHash<WId, QDateTime> waitingForPong;
	void checkPings(){
	  QDateTime cur = QDateTime::currentDateTime();
	  QList<WId> waiting = waitingForPong.keys();
	  for(int i=0; i<waiting.length(); i++){
	    if(waitingForPong.value(waiting[i]) < cur){
	      waitingForPong.remove(waiting[i]); //Timeout on this window
	      if(waitingForPong.isEmpty() && pingTimer!=0){ pingTimer->stop(); }
	      NativeWindow *win = findWindow(waiting[i]);
	      if(win==0){ win = findTrayWindow(waiting[i]); }
	      if(win!=0){ win->emit WindowNotResponding(waiting[i]); }
	    }
	  }
	}

	// Since some properties may be easier to update in bulk 
	//   let the native system interaction do them in whatever logical groups are best
	void UpdateWindowProperties(NativeWindow* win, QList< NativeWindow::Property > props);

public:
	enum Property{ None, CurrentWorkspace, Workspaces, VirtualRoots, WorkAreas };
	enum MouseButton{NoButton, LeftButton, RightButton, MidButton, BackButton, ForwardButton, TaskButton, WheelUp, WheelDown, WheelLeft, WheelRight};

	NativeWindowSystem();
	~NativeWindowSystem();

	//Overarching start/stop functions
	bool start();
	void stop();

	//General-purpose listing functions
	QList<NativeWindow*> currentWindows(){ return NWindows; }
	QList<NativeWindow*> currentTrayWindows(){ return TWindows; }

	//Small simplification functions
	static Qt::Key KeycodeToQt(int keycode);
	static NativeWindowSystem::MouseButton MouseToQt(int button);

public slots:
	//These are the slots which are typically only used by the desktop system itself or the NativeWindowEventFilter

	//RootWindow interactions
	void RegisterVirtualRoot(WId);
	//void GoToWorkspace(int);
	//void RegisterWorkspaces(QStringList); //Names of workspaces, in ascending order
	//void RegisterKnownInteractions();


	//NativeWindowEventFilter interactions
	void NewWindowDetected(WId); //will automatically create the new NativeWindow object
	void NewTrayWindowDetected(WId); //will automatically create the new NativeWindow object
	void WindowCloseDetected(WId); //will update the lists and make changes if needed
	void WindowPropertyChanged(WId, NativeWindow::Property); //will rescan the window and update the object as needed
	void GotPong(WId);

/*	void NewKeyPress(int keycode, WId win = 0);
	void NewKeyRelease(int keycode, WId win = 0);
	void NewMousePress(int buttoncode, WId win = 0);
	void NewMouseRelease(int buttoncode, WId win = 0);*/
	void CheckDamageID(WId);

private slots:
	//These are the slots which are built-in and automatically connected when a new NativeWindow is created
	void RequestPropertiesChange(WId, QList<NativeWindow::Property>, QList<QVariant>);
	void RequestClose(WId);
	void RequestKill(WId);
	void RequestPing(WId);

signals:
	void NewWindowAvailable(NativeWindow*);
	void NewTrayWindowAvailable(NativeWindow*);
	void NewInputEvent(); //a mouse or keypress was detected (lock-state independent);
	void KeyPressDetected(Qt::Key, WId); //only emitted if lockstate = false
	void KeyReleaseDetected(Qt::Key, WId); //only emitted if lockstate = false
	void MousePressDetected(Qt::MouseButton, WId); //only emitted if lockstate = false
	void MouseReleaseDetected(Qt::MouseButton, WId); //only emitted if lockstate = false

};

#endif
