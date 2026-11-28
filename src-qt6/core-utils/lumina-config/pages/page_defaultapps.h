//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details 
//===========================================
#ifndef _LUMINA_CONFIG_PAGE_DEFAULTAPPS_H
#define _LUMINA_CONFIG_PAGE_DEFAULTAPPS_H
#include "../globals.h"
#include "PageWidget.h"

namespace Ui{
	class page_defaultapps;
};

class page_defaultapps : public PageWidget{
	Q_OBJECT
public:
	page_defaultapps(QWidget *parent);
	~page_defaultapps();

public slots:
	void SaveSettings();
	void LoadSettings(int screennum);
	void updateIcons();

private:
	Ui::page_defaultapps *ui;
  QString defaultBrowser;
  QString defaultEmail;
  QString defaultFileManager;
  QString defaultTerminal;

	QString getSysApp(bool allowreset, QString defaultPath = "");

	void updateDefaultButton(QToolButton *button, QString app);

private slots:
	//Simple defaults tab
	void changeDefaultBrowser();
	void changeDefaultEmail();
	void changeDefaultFileManager();
	void changeDefaultTerminal();
	//Advanced defaults tab
	void cleardefaultitem();
	void setdefaultitem();
	void setdefaultbinary();
	void checkdefaulticons();

};
#endif
