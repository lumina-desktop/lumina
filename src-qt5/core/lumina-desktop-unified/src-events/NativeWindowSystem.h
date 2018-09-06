//===========================================
//  Lumina-DE source code
//  Copyright (c) 2017-2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is a Qt5/Lumina wrapper around native graphics system calls
//  It is primarily designed around the creation/modification of instances of
//   the "NativeWindowObject" class for passing information around
//===========================================
#ifndef _LUMINA_DESKTOP_NATIVE_WINDOW_SYSTEM_H
#define _LUMINA_DESKTOP_NATIVE_WINDOW_SYSTEM_H

#include <NativeWindowObject.h>
#include <QObject>
#include <QHash>
#include <QTimer>


class NativeWindowSystem : public QObject{
	Q_OBJECT
private:
	QList<NativeWindowObject*> NWindows;
	QList<NativeWindowObject*> TWindows;

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
	      NativeWindowObject *win = findWindow(waiting[i]);
	      if(win==0){ win = findTrayWindow(waiting[i]); }
	      if(win!=0){ win->emit WindowNotResponding(waiting[i]); }
	    }
	  }
	}

	//Generic private variables
	bool screenLocked;

public:
	//enum Property{ None, CurrentWorkspace, Workspaces, VirtualRoots, WorkAreas };
	enum MouseButton{NoButton, LeftButton, RightButton, MidButton, BackButton, ForwardButton, TaskButton, WheelUp, WheelDown, WheelLeft, WheelRight};

	//Simplifications to find an already-created window object
	NativeWindowObject* findWindow(WId id, bool checkRelated = true);

	NativeWindowObject* findTrayWindow(WId id);
	// Since some properties may be easier to update in bulk
	//   let the native system interaction do them in whatever logical groups are best
	void UpdateWindowProperties(NativeWindowObject* win, QList< NativeWindowObject::Property > props);
	void ChangeWindowProperties(NativeWindowObject* win, QList< NativeWindowObject::Property > props, QList<QVariant> vals);

	void SetupNewWindow(NativeWindowObject *win);
	QImage GetWindowImage(NativeWindowObject *win);
	QPixmap GetTrayWindowImage(NativeWindowObject *win);

	NativeWindowSystem();
	~NativeWindowSystem();

	//Overarching start/stop functions
	bool start();
	void stop();

	//General-purpose listing functions
	QList<NativeWindowObject*> currentWindows(){ return NWindows; }
	QList<NativeWindowObject*> currentTrayWindows(){ return TWindows; }

	//Small simplification functions
	static Qt::Key KeycodeToQt(int keycode);
	static NativeWindowSystem::MouseButton MouseToQt(int button);
	void RegisterEventShortcut(Qt::Key key, bool set);
	void RegisterEventShortcut(int keycode, bool set);

	void raiseWindow(NativeWindowObject *win);
	void lowerWindow(NativeWindowObject *win);

public slots:
	//These are the slots which are typically only used by the desktop system itself or the NativeWindowEventFilter

	//This is called by the lock screen to keep the NWS aware of the current status
	// it is **NOT** the function to call for the user to actually lock the session (that is in the screensaver/lockscreen class)
	 void ScreenLockChanged(bool lock){
	  screenLocked = lock;
	}

	//Root Window property registrations
	void RegisterVirtualRoot(WId);
	void setRoot_supportedActions();
	void setRoot_numberOfWorkspaces(QStringList names);
	void setRoot_currentWorkspace(int);
	void setRoot_clientList(QList<WId>, bool stackorder = false);
	void setRoot_desktopGeometry(QRect);
	void setRoot_desktopWorkarea(QList<QRect>);
	void setRoot_activeWindow(WId);

	//  - Workspaces
	int currentWorkspace();
	//void GoToWorkspace(int);


	//NativeWindowEventFilter interactions
	void NewWindowDetected(WId); //will automatically create the new NativeWindow object
	void NewTrayWindowDetected(WId); //will automatically create the new NativeWindow object
	void WindowCloseDetected(WId); //will update the lists and make changes if needed
	void WindowPropertyChanged(WId, NativeWindowObject::Property); //will rescan the window and update the object as needed
	void WindowPropertiesChanged(WId, QList<NativeWindowObject::Property>);
	void WindowPropertyChanged(WId, NativeWindowObject::Property, QVariant); //will save that property/value to the right object
	void WindowPropertiesChanged(WId, QList<NativeWindowObject::Property>, QList<QVariant>);
	void RequestPropertyChange(WId, NativeWindowObject::Property, QVariant);
	void RequestPropertiesChange(WId, QList<NativeWindowObject::Property>, QList<QVariant>);
	void GotPong(WId);

	void NewKeyPress(int keycode, WId win = 0);
	void NewKeyRelease(int keycode, WId win = 0);
	void NewMousePress(int buttoncode, WId win = 0);
	void NewMouseRelease(int buttoncode, WId win = 0);
	void CheckDamageID(WId);

	void RequestReparent(WId, WId, QPoint); //client, parent, relative origin point in parent


private slots:
	//Internal Admin functions
	void verifyWindowExistance();

	//These are the slots which are built-in and automatically connected when a new NativeWindow is created
	void RequestClose(WId);
	void RequestKill(WId);
	void RequestPing(WId);

	//Window-mgmt functions (see Window-mgmt.cpp for details)
	void ArrangeWindows(WId primary, QString type);
	void TileWindows(WId primary, QString type);
	void CheckWindowPosition(WId id, bool newwindow = false);
	void CheckWindowPosition(NativeWindowObject *win, bool newwindow = false);
	void arrangeWindows(NativeWindowObject *primary, QString type, bool primaryonly = false);

signals:
	void NewWindowAvailable(NativeWindowObject*);
	void WindowClosed();
	void NewTrayWindowAvailable(NativeWindowObject*);
	void TrayWindowClosed();
	void NewInputEvent(); //a mouse or keypress was detected (lock-state independent);
	void KeyPressDetected(WId, Qt::Key); //only emitted if lockstate = false
	void KeyReleaseDetected(WId, Qt::Key); //only emitted if lockstate = false
	void MousePressDetected(WId, NativeWindowSystem::MouseButton); //only emitted if lockstate = false
	void MouseReleaseDetected(WId, NativeWindowSystem::MouseButton); //only emitted if lockstate = false

};

#endif
