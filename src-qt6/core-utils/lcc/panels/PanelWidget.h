//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2024, jt(q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_LCC_PANELS_PANELWIDGET_H
#define _LUMINA_LCC_PANELS_PANELWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QApplication>
#include <QScreen>
#include <QColorDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QToolButton>
#include <QTabWidget>

#include <LUtils.h>
#include <LuminaXDG.h>

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
	void SaveSettings(QSettings *settings, QString screenID = ""); //save the current settings
	
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
	QString getSysApp(bool allowreset = false);
	QString getColorStyle(QString current, bool allowTransparency = true);
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
