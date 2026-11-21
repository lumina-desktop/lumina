//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2015-2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  Global defines and enumerations for external includes
//===========================================
#ifndef _LUMINA_EXTERNAL_GLOBAL_DEFINES_H
#define _LUMINA_EXTERNAL_GLOBAL_DEFINES_H

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
#include <QTemporaryFile>
#include <QFile>
#include <QDir>
#include <QString>
#include <QTextStream>
#include <QUrl>
#include <QDebug>
#include <QStringList>
#include <QAbstractNativeEventFilter>
#include <QList>
#include "LuminaX11Bridge.h"
#include <QCoreApplication>
#include <QPropertyAnimation>
#include <QAnimationGroup>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QWindow>
#include <QWidget>
#include <QWidgetAction>
#include <QBackingStore>
#include <QPaintEvent>
#include <QPainter>
#include <QSettings>
#include <QHostInfo>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QDesktopWidget>
#endif
#include <QStyleOption>
#include <QThread>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QMediaObject>
#include <QMediaPlaylist>
#endif
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QQuickView>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickImageProvider>
#include <QtConcurrent>

// libLumina includes
#include <LuminaX11.h>
#include <LuminaXDG.h>
#include <LuminaOS.h>
#include <LuminaThemes.h>
#include <LUtils.h>
#include <LDesktopUtils.h>
#include <LuminaSingleApplication.h>
#include <QThreadPool>

//Delete a desktop item (file, symlink or directory tree) without going through a shell command.
// - A symlink (even one pointing at a directory) is unlinked, never followed: QDir::removeRecursively()
//   on a link would empty the link's *target*.
// - Directory trees are removed on a pool thread so a large folder does not freeze the desktop.
// (Replaces LUtils::startDetached("rm -r \"<path>\""), which broke on names containing a quote.)
inline void LuminaRemoveDesktopItem(const QString &path){
  QFileInfo info(path);
  if(info.isSymLink() || !info.isDir()){ QFile::remove(path); return; }
  QThreadPool::globalInstance()->start([path](){ QDir(path).removeRecursively(); });
}
#include <DesktopSettings.h>
#include <ExternalProcess.h>
#include <XDGMime.h>
#include <LIconCache.h>
#include <LFileInfo.h>
#include <framework-OSInterface.h>

// C++ Backend classes for QML interface
//#include <NativeWindowObject.h>
//#include <ScreenObject.h>

//Setup any global defines (no classes or global objects: use "global-objects.h" for that)


#endif
