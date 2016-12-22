//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore & JT Pennington
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details 
//===========================================
#ifndef _LUMINA_CONFIG_PAGE_BLUETOOTH_TRUEOS_H
#define _LUMINA_CONFIG_PAGE_BLUETOOTH_TRUEOS_H
#include "../globals.h"
#include "PageWidget.h"

namespace Ui{
    class page_bluetooth;
};

class page_bluetooth : public PageWidget{
	Q_OBJECT
public:
    page_bluetooth(QWidget *parent);
    ~page_bluetooth();

public slots:
	void SaveSettings();
	void LoadSettings(int screennum);
	void updateIcons();

private:
    Ui::page_bluetooth *ui;

private slots:

};
#endif
