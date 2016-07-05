//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details 
//===========================================
#ifndef _LUMINA_CONFIG_PAGE_FLUXBOX_KEYS_H
#define _LUMINA_CONFIG_PAGE_FLUXBOX_KEYS_H
#include "../globals.h"
#include "PageWidget.h"

namespace Ui{
	class page_fluxbox_keys;
};

class page_fluxbox_keys : public PageWidget{
	Q_OBJECT
public:
	page_fluxbox_keys(QWidget *parent);
	~page_fluxbox_keys();

public slots:
	void SaveSettings();
	void LoadSettings(int screennum);
	void updateIcons();

	//Simplification function for widget connections
	virtual void settingChanged(){
	  if(!loading){ emit HasPendingChanges(true); }
	}

private:
	Ui::page_fluxbox_keys *ui;
	bool loading;

	//Convert to/from fluxbox keyboard shortcuts
	QString dispToFluxKeys(QString);
	QString fluxToDispKeys(QString);

	//Read/overwrite a text file
	QStringList readFile(QString path);
	bool overwriteFile(QString path, QStringList contents);

private slots:
	void clearKeyBinding();
	void applyKeyBinding();
	void updateKeyConfig();
	void switchEditor();
	void togglesyntaxgroup();
};
#endif
