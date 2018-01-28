//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore  & JT Pennington
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details 
//===========================================
#ifndef _LUMINA_CONFIG_PAGE_MOUSE_TRUEOS_H
#define _LUMINA_CONFIG_PAGE_MOUSE_TRUEOS_H
#include "../globals.h"
#include "PageWidget.h"

#include <LInputDevice.h>
#include <QTreeWidgetItem>

namespace Ui{
    class page_mouse_trueos;
};

class page_mouse_trueos : public PageWidget{
	Q_OBJECT
public:
    page_mouse_trueos(QWidget *parent);
    ~page_mouse_trueos();
    const double divisor = 100;
    double realAccelValue;
    QString realAccelValueString;
    double realDoubleClickValue;
    QString realDoubleClickValueString;
    QString resolutionValue;
    QString handString;
    QString resString;
    QString dclickString;
    QString driftString;
    QString accelString;
    QString program;
    QStringList mousedargs;
    QString deviceString;

public slots:
	void SaveSettings();
	void LoadSettings(int screennum);
//	void updateIcons();
    void swapHandedness();
    void setMouseResolution();
    void setMouseAcceleration();
    void setDoubleClickThreshold();
    void terminateDrift();
    void updateMoused();

private:
    Ui::page_mouse_trueos *ui;
};
#endif
