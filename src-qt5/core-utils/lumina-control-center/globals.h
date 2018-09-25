//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_CONFIG_GLOBALS_H
#define _LUMINA_CONFIG_GLOBALS_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QWidget>
#include <QDesktopWidget>
#include <QScreen>
#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QToolButton>
#include <QSettings>
#include <QFileDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QShortcut>
#include <QImageReader>
#include <QDialog>
#include <QPoint>
#include <QCursor>
#include <QMenu>

//Now the Lumina Library classes
#include <LuminaXDG.h>
#include <LUtils.h>
#include <LDesktopUtils.h>
#include <LuminaX11.h>
#include <LuminaOS.h>
#include <LuminaThemes.h>

#include "utils/PageWidget.h"

#endif

//Now the global class for available system applications
extern XDGDesktopList *APPSLIST;
