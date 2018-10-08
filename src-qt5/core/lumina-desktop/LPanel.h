//===========================================
//  Lumina-DE source code
//  Copyright (c) 2012-2015, Ken Moore
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
        //QRect hidegeom, showgeom; //for hidden panels
	QPoint hidepoint, showpoint; //for hidden panels: locations when hidden/visible
	bool defaultpanel, horizontal, hidden, reserveloc;
	QString screenID;
	int panelnum;
	int viswidth, fullwidth;
	QList<LPPlugin*> PLUGINS;
	QRect desiredGeom;

	int Screen(); //Turn the screenID into the appropriate number

public:
	LPanel(QSettings *file, QString scr = 0, int num = 0, QWidget *parent = 0, bool reservespace = true); //settings file, screen number, panel number
	~LPanel();

	int number(){
	  return panelnum;
	}

	QString prefix(){
	  return PPREFIX;
	}

	int visibleWidth(){
	  return viswidth;
	}
	void prepareToClose();
	void scalePanel(double xscale, double yscale);

public slots:
	void UpdatePanel(bool geomonly = false);  //Load the settings file and update the panel appropriately
	void UpdateLocale(); //Locale Changed externally
	void UpdateTheme(); //Theme Changed externally

private slots:
	void checkPanelFocus();

protected:
	void resizeEvent(QResizeEvent *event);
	void paintEvent(QPaintEvent *event);
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);
};

#endif
