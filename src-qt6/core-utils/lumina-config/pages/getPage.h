//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_CONFIG_PAGE_WIDGET_FETCH_H
#define _LUMINA_CONFIG_PAGE_WIDGET_FETCH_H

#include "../globals.h"
#include "PageWidget.h"

class Pages{
public:
  static PAGEINFO PageInfo(QString, QString, QString, QString, QString, QString, QStringList, QStringList);
  static QList<PAGEINFO> KnownPages();
  static PageWidget* GetNewPage(QString id, QWidget *parent);
};

#endif
