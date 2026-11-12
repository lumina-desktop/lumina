//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_CONFIG_PAGE_THEME_H
#define _LUMINA_CONFIG_PAGE_THEME_H
#include "../globals.h"
#include "PageWidget.h"

#include "../LPlugins.h"

namespace Ui{
	class page_theme;
};

class page_theme : public PageWidget{
	Q_OBJECT
public:
	page_theme(QWidget *parent);
	~page_theme();

public slots:
	void SaveSettings();
	void LoadSettings(int screennum);
	void updateIcons();

private:
	Ui::page_theme *ui;
	LPlugins *PINFO;
	bool loading;

	void findQt5Themes();

private slots:
	void settingsChanged(){
          //qDebug() << "Setting Changed:" << !loading;
	  if(!loading){ emit HasPendingChanges(true); }
	}
	void sessionEditColor();
	void sessionEditTheme();
	void checkQt5Theme();
};
#endif
