//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_WINDOW_MANAGER_MAIN_CLASS_H
#define _LUMINA_DESKTOP_WINDOW_MANAGER_MAIN_CLASS_H

#include "GlobalDefines.h"
#include "LWindow.h"

class LWindowManager : public QObject{
	Q_OBJECT
public:
	LWindowManager();
	~LWindowManager();

	bool start();
	void stop();

private:
	QList<LWindow*> WINS;
	QList<WId> Stack_Desktop, Stack_Below, Stack_Normal, Stack_Above, Stack_Fullscreen;
public slots:
	void NewWindow(WId win, bool requested = true);
	void ClosedWindow(WId win);
	void ModifyWindow(WId win, LWM::WindowAction act);

	void RestackWindows();
	void RepaintWindows();

private slots:
	void FinishedWindow(WId win); //This is used for LWindow connections/animations

signals:
	void NewFullScreenWindows(QList<WId>);
};

#endif
