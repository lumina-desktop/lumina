//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details 
//===========================================
#ifndef _LUMINA_CONFIG_PAGE_MOUSE_H
#define _LUMINA_CONFIG_PAGE_MOUSE_H
#include "../globals.h"
#include "PageWidget.h"

#include <LInputDevice.h>

namespace Ui{
	class page_mouse;
};

class page_mouse : public PageWidget{
	Q_OBJECT
public:
	page_mouse(QWidget *parent);
	~page_mouse();

public slots:
	void SaveSettings();
	void LoadSettings(int screennum);
	void updateIcons();

private:
	Ui::page_mouse *ui;
	QList<LInputDevice*> devices;

	void generateUI();
	void populateDeviceTree(QTreeWidget *tree, LInputDevice *device);
	void populateDeviceItemValue(QTreeWidget *tree, QTreeWidgetItem *it, QVariant value, QString id);

private slots:
	void valueChanged();

};
#endif
