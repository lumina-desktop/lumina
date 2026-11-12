//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_CONFIG_PAGE_WALLPAPER_H
#define _LUMINA_CONFIG_PAGE_WALLPAPER_H
#include "../globals.h"
#include "PageWidget.h"

namespace Ui{
	class page_wallpaper;
};

class page_wallpaper : public PageWidget{
	Q_OBJECT
public:
	page_wallpaper(QWidget *parent);
	~page_wallpaper();

	bool needsScreenSelector(){ return true; }

public slots:
	void SaveSettings();
	void LoadSettings(int screennum);
	void updateIcons();

private:
	Ui::page_wallpaper *ui;
	int cScreen, cBG; //current screen number/background
	QString DEFAULTBG;
	bool loading;

	QString getColorStyle(QString current, bool allowTransparency);

private slots:
	void updateMenus();
	void deskbgchanged();
	void desktimechanged();
	void deskbgremoved();
	void deskbgadded();
	void deskbgcoloradded();
	void deskbgdiradded();
	void deskbgdirradded();

protected:
	void resizeEvent(QResizeEvent*){
	  deskbgchanged(); //update the wallpaper preview
	}
};
#endif
