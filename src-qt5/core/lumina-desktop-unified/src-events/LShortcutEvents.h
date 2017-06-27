//===========================================
//  Lumina desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  Class for managing key presses and sending out signals
//    when a shortcut combination is pressed
//===========================================
#ifndef _LUMINA_KEY_SEQUENCE_DETECTION_H
#define _LUMINA_KEY_SEQUENCE_DETECTION_H

#include "../global-includes.h"

class LShortcutEvents : public QObject{
	Q_OBJECT
public:
	LShortcutEvents();
	~LShortcutEvents();

	void start();
	void stop();

private:
	QList<int> keylist; //keys currently held down (NOTE: QKeySequence has a max of 4 keys for combinations)
	WId WIN; //current window being acted on by the keys
	QTimer *clearTimer; //used to clear the internal keylist every once in a while if no events come in.
	bool evaluated;

	//Actual check functions
	void CheckKeySequence(WId win);
	void CheckMouseSequence(WId win, NativeWindowSystem::MouseButton, bool release);
	QString keylistToString();
	void evaluateShortcutAction(QString action);

public slots:
	void KeyPress(WId window, int key);
	void KeyRelease(WId window, int key);
	void MousePress(WId window, NativeWindowSystem::MouseButton);
	void MouseRelease(WId window, NativeWindowSystem::MouseButton);
	void clearKeys();

signals:
	// Power Options
	void OpenLeaveDialog();
	void StartLogout();
	void StartReboot();  //assumes startUpdates==true
	void StartShutdown(); //assumes startUpdates==true

	// Session Options
	void ChangeWorkspace(int); // +/- 1 from current
	void LockSession();

	//Active Window Options
	void ActiveWindowMoveToWorkspace(int); //number of workspace
	void ActiveWindowTakeToWorkspace(int); //number of workspace
	void ActiveWindowKill();
	void ActiveWindowClose();
	void ActiveWindowMinMaxToggle();
	void ActiveWindowFullscreenToggle();
	void ActiveWindowStartMove();
	void ActiveWindowStopMove();
	void ActiveWindowStartResize();
	void ActiveWindowStopResize();


	//General Utility Launch
	void LaunchApplication(QString exec);

};

#endif
