//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Susanne Jaeckel
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_BATTERY_H
#define _LUMINA_DESKTOP_BATTERY_H

#include <QTimer>
#include <QWidget>
#include <QString>
#include <QLabel>

#include <LuminaUtils.h>
#include <LuminaXDG.h>
#include <LuminaOS.h>

#include "../../Globals.h"
//#include "../LTBWidget.h"
#include "../LPPlugin.h"

class LBattery : public LPPlugin{
	Q_OBJECT
public:
	LBattery(QWidget *parent = 0, QString id = "battery", bool horizontal=true);
	~LBattery();
	
private:
	QTimer *timer;
	QLabel *label;
	int iconOld;
	
private slots:
	void updateBattery(bool force = false);
	QString getRemainingTime();

public slots:
	void OrientationChange(){
	  if(this->layout()->direction()==QBoxLayout::LeftToRight){
	    label->setFixedSize( QSize(this->height(), this->height()) );
	  }else{
	    label->setFixedSize( QSize(this->width(), this->width()) );
	  }
	  updateBattery(true); //force icon refresh
	}
};

#endif
