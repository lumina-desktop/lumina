//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details 
//===========================================
#ifndef _LUMINA_CONFIG_PAGE_COMPTON_H
#define _LUMINA_CONFIG_PAGE_COMPTON_H
#include "../globals.h"
#include "PageWidget.h"

namespace Ui{
	class page_compton;
};

class page_compton : public PageWidget{
	Q_OBJECT
public:
	page_compton(QWidget *parent);
	~page_compton();

public slots:
	void SaveSettings();
	void LoadSettings(int screennum);
	void updateIcons();

private:
	Ui::page_compton *ui;

private slots:

};
#endif
