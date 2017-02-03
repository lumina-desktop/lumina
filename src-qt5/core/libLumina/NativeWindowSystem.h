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
	}

	NativeWindow* findTrayWindow(WId id){
	  for(int i=0; i<TWindows.length(); i++){ 
	    if(id==TWindows[i]->id()){ return TWindows[i]; } 
	  }
	}

	//Now define a simple private_objects class so that each implementation 
	//  has a storage container for placing private objects as needed
	class p_objects;
	p_objects* obj;

	// Since some properties may be easier to update in bulk 
	//   let the native system interaction do them in whatever logical groups are best
	void UpdateWindowProperties(NativeWindow* win, QList< NativeWindow::Property > props);

public:
	NativeWindowSystem();
	~NativeWindowSystem();

	//Overarching start/stop functions
	bool start();
	void stop();

	//General-purpose listing functions
	QList<NativeWindow*> currentWindows(){ return NWindows; }

public slots:
	//These are the slots which are typically only used by the desktop system itself or the NativeWindowEventFilter

	//RootWindow interactions
	void RegisterVirtualRoot(WId);
	//void GoToWorkspace(int);
	//void RegisterWorkspaces(QStringList); //Names of workspaces, in ascending order
	//void RegisterKnownInteractions();


	//NativeWindowEventFilter interactions
	void NewWindowDetected(WId); //will automatically create the new NativeWindow object
	void WindowCloseDetected(WId); //will update the lists and make changes if needed
	void WindowPropertyChanged(WId, NativeWindow::Property); //will rescan the window and update the object as needed
	void NewKeyPress(int keycode);
	void NewKeyRelease(int keycode);
	void NewMousePress(int buttoncode);
	void NewMouseRelease(int buttoncode);
	void CheckDamageID(WId);

private slots:
	//These are the slots which are built-in and automatically connected when a new NativeWindow is created
	void RequestActivate(WId);
	void RequestClose(WId);
	void RequestSetVisible(WId, bool);
	void RequestSetGeometry(WId, QRect);
	void RequestSetFrameExtents(WId, QList<int>); //[Left,Right,Top,Bottom] in pixels

signals:
	void NewWindowAvailable(NativeWindow*);
	void NewInputEvent(); //a mouse or keypress was detected (lock-state independent);
	void NewKeyPress(int); //only emitted if lockstate = false
	void NewKeyRelease(int); //only emitted if lockstate = false
	void NewMousePress(Qt::MouseButton); //only emitted if lockstate = false

};

#endif
