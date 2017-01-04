//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  Global defines and enumerations for the window manager
//===========================================
#ifndef _LUMINA_WINDOW_MANAGER_GLOBAL_DEFINES_H
#define _LUMINA_WINDOW_MANAGER_GLOBAL_DEFINES_H

//Qt includes
#include <QObject>
#include <QFrame>
#include <QLabel>
#include <QToolButton>
#include <QMenu>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QAction>
#include <QPoint>
#include <QFile>
#include <QDir>
#include <QString>
#include <QTextStream>
#include <QUrl>
#include <QDebug>
#include <QStringList>
#include <QAbstractNativeEventFilter>
#include <QList>
#include <QX11Info>
#include <QCoreApplication>
#include <QPropertyAnimation>
#include <QAnimationGroup>
#include <QParallelAnimationGroup>
#include <QWindow>
#include <QWidget>
#include <QBackingStore>
#include <QPaintEvent>
#include <QPainter>
#include <QSettings>
#include <QHostInfo>
#include <QDesktopWidget>
#include <QStyleOption>
#include <QThread>

// libLumina includes
#include <LuminaX11.h>
#include <LuminaXDG.h>
#include <LuminaOS.h>
#include <LuminaThemes.h>
#include <LuminaUtils.h>
#include <LuminaSingleApplication.h>

//XCB Includes
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/damage.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_aux.h> //included in libxcb-util.so

#define ANIMTIME 80 //animation time in milliseconds
//Global flags/structures
namespace LWM{
	//Flags/enumerations
	enum WindowAction{MoveResize, Show, Hide, TryClose, Closed, WA_NONE};
	
	//Data structures
	extern LXCB *SYSTEM;
};



#endif
