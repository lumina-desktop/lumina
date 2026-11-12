//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details 
//===========================================
#ifndef _LUMINA_CONFIG_PAGE_SESSION_LOCALE_H
#define _LUMINA_CONFIG_PAGE_SESSION_LOCALE_H
#include "../globals.h"
#include "PageWidget.h"

namespace Ui{
	class page_session_locale;
};

class page_session_locale : public PageWidget{
	Q_OBJECT
public:
	page_session_locale(QWidget *parent);
	~page_session_locale();

public slots:
	void SaveSettings();
	void LoadSettings(int screennum);
	void updateIcons();

private:
	Ui::page_session_locale *ui;

	void setupLocales();

private slots:

};
#endif
