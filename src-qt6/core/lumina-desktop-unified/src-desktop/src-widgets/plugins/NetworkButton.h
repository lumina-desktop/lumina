//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_PLUGIN_BUTTON_NETWORK_H
#define _LUMINA_DESKTOP_PLUGIN_BUTTON_NETWORK_H

#include <global-includes.h>

class NetworkButton : public QToolButton{
	Q_OBJECT
private slots:
	void updateButton();
	void buttonClicked();

public:
	NetworkButton(QWidget *parent = 0);
	~NetworkButton();

};
#endif
