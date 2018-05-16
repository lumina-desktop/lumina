//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_PLUGIN_BUTTON_UPDATES_H
#define _LUMINA_DESKTOP_PLUGIN_BUTTON_UPDATES_H

#include <global-includes.h>

class UpdatesButton : public QToolButton{
	Q_OBJECT
private slots:
	void updateButton();

public:
	UpdatesButton(QWidget *parent = 0);
	~UpdatesButton();

};
#endif
