//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details 
//===========================================
#ifndef _LUMINA_CONFIG_PAGE_FLUXBOX_SETTINGS_H
#define _LUMINA_CONFIG_PAGE_FLUXBOX_SETTINGS_H
#include "../globals.h"
#include "PageWidget.h"

namespace Ui{
	class page_fluxbox_settings;
};

class page_fluxbox_settings : public PageWidget{
	Q_OBJECT
public:
	page_fluxbox_settings(QWidget *parent);
	~page_fluxbox_settings();

public slots:
	void SaveSettings();
	void LoadSettings(int screennum);
	void updateIcons();

private:
	Ui::page_fluxbox_settings *ui;
	bool loading;

	//Read/overwrite a text file
	QStringList readFile(QString path);
	bool overwriteFile(QString path, QStringList contents);

private slots:
	//Simplification function for widget connections
	void settingChanged(){
	  if(!loading){ emit HasPendingChanges(true); }
	}

	void sessionthemechanged();
	void switchEditor();
};
#endif
