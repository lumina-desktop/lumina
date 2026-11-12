//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details 
//===========================================
#ifndef _LUMINA_CONFIG_PAGE_SAMPLE_H
#define _LUMINA_CONFIG_PAGE_SAMPLE_H
#include "../globals.h"
#include "PageWidget.h"

namespace Ui{
	class page_sample;
};

class page_sample : public PageWidget{
	Q_OBJECT
public:
	page_sample(QWidget *parent);
	~page_sample();

public slots:
	void SaveSettings();
	void LoadSettings(int screennum);
	void updateIcons();

private:
	Ui::page_sample *ui;

private slots:

};
#endif
