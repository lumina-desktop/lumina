//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details 
//===========================================
#ifndef _LUMINA_CONFIG_PAGE_INTERFACE_DESKTOP_H
#define _LUMINA_CONFIG_PAGE_INTERFACE_DESKTOP_H
#include "../globals.h"
#include "PageWidget.h"
#include "../LPlugins.h"

namespace Ui{
	class page_interface_desktop;
};

class page_interface_desktop : public PageWidget{
	Q_OBJECT
public:
	page_interface_desktop(QWidget *parent);
	~page_interface_desktop();

	bool needsScreenSelector(){ return true; }

public slots:
	void SaveSettings();
	void LoadSettings(int screennum = -1);
	void updateIcons();

private:
	Ui::page_interface_desktop *ui;
	int cscreen; //current monitor/screen number
	LPlugins *PINFO;

	//Get an application on the system
	QString getSysApp(bool allowreset = false);

private slots:
	void deskplugadded();
	void deskplugremoved();
};
#endif
