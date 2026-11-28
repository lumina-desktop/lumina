//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_CONFIG_PAGE_INTERFACE_PANELS_H
#define _LUMINA_CONFIG_PAGE_INTERFACE_PANELS_H
#include "../globals.h"
#include "PageWidget.h"
#include "../LPlugins.h"
#include "../PanelWidget.h"

namespace Ui{
	class page_interface_panels;
};

class page_interface_panels : public PageWidget{
	Q_OBJECT
public:
	page_interface_panels(QWidget *parent);
	~page_interface_panels();

	bool needsScreenSelector(){ return true; }

public slots:
	void SaveSettings();
	void LoadSettings(int screennum = -1);
	void updateIcons();

private:
	Ui::page_interface_panels *ui;
	bool loading;
	int cscreen; //current monitor/screen number
	QSettings *settings;
	LPlugins *PINFO;
	QList<PanelWidget*> PANELS;

	void setupProfiles();

private slots:
	void panelValChanged();
	void newPanel();
	void removePanel(int); //connected to a signal from the panel widget
	void applyProfile(QAction*);
	void applyImport(QAction*);
	void applyImport(QString fromID);
};
#endif
