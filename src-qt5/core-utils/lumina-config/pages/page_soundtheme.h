//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2017, JT (q5sys)
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details 
//===========================================
#ifndef _LUMINA_CONFIG_PAGE_SOUNDTHEME_H
#define _LUMINA_CONFIG_PAGE_SOUNDTHEME_H
#include "../globals.h"
#include "PageWidget.h"

namespace Ui{
    class page_soundtheme;
};

class page_soundtheme : public PageWidget{
	Q_OBJECT
public:
    page_soundtheme(QWidget *parent);
    ~page_soundtheme();

public slots:

    void SaveSettings();
    void LoadSettings(int screennum);


private:
    Ui::page_soundtheme *ui;
    QString startupsound, logoutsound, batterysound;
    QSettings *sessionsettings;

private slots:

    void on_pushButton_startup_clicked();
    void on_pushButton_logout_clicked();
    void on_pushButton_battery_clicked();
    void on_checkBox_startup_toggled(bool checked);
    void on_checkBox_logout_toggled(bool checked);
    void on_checkBox_battery_toggled(bool checked);
};
#endif
