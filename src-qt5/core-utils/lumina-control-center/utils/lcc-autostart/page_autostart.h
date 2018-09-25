//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_CONFIG_PAGE_AUTOSTART_H
#define _LUMINA_CONFIG_PAGE_AUTOSTART_H
#include "../globals.h"
#include "PageWidget.h"

namespace Ui{
	class page_autostart;
};

class page_autostart : public PageWidget{
	Q_OBJECT
public:
	page_autostart(QWidget *parent);
	~page_autostart();

public slots:
	void SaveSettings();
	void LoadSettings(int screennum);
	void updateIcons();

private:
	Ui::page_autostart *ui;

	QString getSysApp(bool allowreset);

private slots:
	void rmsessionstartitem();
	void addsessionstartapp();
	void addsessionstartbin();
	void addsessionstartfile();
};
#endif
