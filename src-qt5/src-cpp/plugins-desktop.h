//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_DESKTOP_PLUGINS_CLASS_H
#define _LUMINA_DESKTOP_DESKTOP_PLUGINS_CLASS_H

#include "plugins-base.h"
#include <QJsonObject>
#include <QString>
#include <QUrl>
#include <QObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QIcon>

class DTPlugin : public BasePlugin{
private:
  QSize gridSize;
  bool panelPossible;
  QIcon pluginIcon;
public:
	DTPlugin();
	~DTPlugin();

	virtual bool isValid() Q_DECL_OVERRIDE;

  QSize getSize { return gridSize; }
  bool getPanelable { return panelPossible; }
  QIcon getIcon { return pluginIcon; }
};

#endif
