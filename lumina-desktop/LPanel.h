//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the generic class for creating a full-width panel that stays
//    on top of all other windows (top or bottom of the screen only)
//===========================================
#ifndef _LUMINA_DESKTOP_PANEL_H
#define _LUMINA_DESKTOP_PANEL_H

#include <QWidget>
#include <QBoxLayout>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QMoveEvent>
#include <QDesktopWidget>
#include <QPainter>
#include <QPaintEvent>

#include "panel-plugins/NewPP.h"
#include "panel-plugins/LPPlugin.h"

#include <LuminaX11.h>
#include <LuminaOS.h>

class LPanel : public QWidget{
	Q_OBJECT
private:
	QBoxLayout *layout;
	QSettings *settings;
	QString PPREFIX; //internal prefix for all settings
	QDesktopWidget *screen;
	QWidget *bgWindow, *panelArea;
	bool defaultpanel, horizontal;
	int screennum;
	QList<LPPlugin*> PLUGINS;

public:
	LPanel(QSettings *file, int scr = 0, int num =0, QWidget *parent=0); //settings file, screen number, panel number
	~LPanel();

	int number(){
	  return PPREFIX.section(".",-1).toInt();
	}

public slots:
	void UpdatePanel();  //Load the settings file and update the panel appropriately
	void UpdateLocale(); //Locale Changed externally
	void UpdateTheme(); //Theme Changed externally

protected:
	void paintEvent(QPaintEvent *event);
};

#endif
