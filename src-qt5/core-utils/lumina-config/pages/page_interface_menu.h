//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details 
//===========================================
#ifndef _LUMINA_CONFIG_PAGE_INTERFACE_MENU_H
#define _LUMINA_CONFIG_PAGE_INTERFACE_MENU_H
#include "../globals.h"
#include "PageWidget.h"
#include "../LPlugins.h"

namespace Ui{
	class page_interface_menu;
};

class page_interface_menu : public PageWidget{
	Q_OBJECT
public:
	page_interface_menu(QWidget *parent);
	~page_interface_menu();

public slots:
	void SaveSettings();
	void LoadSettings(int screennum);
	void updateIcons();

private:
	Ui::page_interface_menu *ui;
	LPlugins *PINFO;
	//Get an application on the system
	QString getSysApp(bool allowreset = false);

private slots:
	void addmenuplugin();
	void rmmenuplugin();
	void upmenuplugin();
	void downmenuplugin();
	void checkmenuicons();
};
#endif
