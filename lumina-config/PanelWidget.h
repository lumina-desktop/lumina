//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_CONFIG_PANEL_WIDGET_H
#define _LUMINA_CONFIG_PANEL_WIDGET_H

#include <QWidget>
#include <QObject>
#include <QSettings>

//#include "mainUI.h"
#include "LPlugins.h"

//namespace for using the *.ui file
namespace Ui{
	class PanelWidget;
};

class PanelWidget : public QWidget{
	Q_OBJECT
public:
	PanelWidget(QWidget *parent, QWidget *Main, LPlugins *Pinfo);
	~PanelWidget();

	void LoadSettings(QSettings *settings, int Dnum, int Pnum);
	void SaveSettings(QSettings *settings); //save the current settings
	
	int PanelNumber();
	void ChangePanelNumber(int newnum);

public slots:
	void LoadIcons();

private:
	Ui::PanelWidget *ui;
	QWidget *mainui;
	LPlugins *PINFO;

	int dnum, pnum;

	void reloadColorSample();

private slots:
	void on_tool_rm_clicked();
	void ItemChanged();
	void UseColorChanged();
	void on_tool_selectcolor_clicked();
	void on_tool_addplugin_clicked();
	void on_tool_remplugin_clicked();
	void on_tool_upplugin_clicked();
	void on_tool_downplugin_clicked();

signals:
	void PanelChanged();
	void PanelRemoved(int);

};

#endif

