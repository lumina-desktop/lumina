//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_ROOT_WINDOW_H
#define _LUMINA_DESKTOP_ROOT_WINDOW_H
#include <global-includes.h>
#include "src-cpp/RootDesktopObject.h"

class RootWindow : public QObject{
	Q_OBJECT
private:
	QWindow *root_win;
	QQuickView *root_view;
	RootDesktopObject *root_obj;

public:
	RootWindow();
	~RootWindow();

	void start();

	WId viewID(){ return root_view->winId(); }

public slots:
	void syncRootSize();

signals:
	void startLogout();
	void RegisterVirtualRoot(WId);
	void RootResized(QRect);
	void MouseMoved();
};

#endif
