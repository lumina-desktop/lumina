//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_WINDOW_MANAGER_SESSION_H
#define _LUMINA_DESKTOP_WINDOW_MANAGER_SESSION_H

#include "GlobalDefines.h"

#include "LScreenSaver.h"
#include "LXcbEventFilter.h"
#include "LWindowManager.h"

class WMSession : public QObject{
	Q_OBJECT
public:
	WMSession();
	~WMSession();

	void start(bool SSONLY = false);

private:
	//XCB Event Watcher
	EventFilter *EFILTER;
	//ScreenSaver
	LScreenSaver *SS;
	//Window Manager
	LWindowManager *WM;
	
	QThread *EVThread; //X Event thread

public slots:
	void reloadIcons();
	void newInputsAvailable(QStringList);

private slots:
	
};

#endif