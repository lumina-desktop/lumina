//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_SCREENSAVER_PLUGINS_CLASS_H
#define _LUMINA_DESKTOP_SCREENSAVER_PLUGINS_CLASS_H

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

class SSPlugin : public BasePlugin {
public:
	SSPlugin();
	~SSPlugin();

	virtual bool isValid() Q_DECL_OVERRIDE;
};

#endif
