//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_TASK_MANAGER_PLUGIN_H
#define _LUMINA_DESKTOP_TASK_MANAGER_PLUGIN_H

// Qt includes
#include <QWidget>
#include <QList>
#include <QString>
#include <QDebug>
#include <QTimer>
#include <QEvent>

// libLumina includes
#include <LuminaX11.h>

// Local includes
#include "LTaskButton.h"
#include "LWinInfo.h"
#include "../LPPlugin.h"

class LTaskManagerPlugin : public LPPlugin{
	Q_OBJECT
public:
	LTaskManagerPlugin(QWidget *parent=0, QString id="taskmanager", bool horizontal=true);
	~LTaskManagerPlugin();

private:
	QList<LTaskButton*> BUTTONS; //to keep track of the current buttons
	QTimer *timer;
	bool updating; //quick flag for if it is currently working

private slots:
	void UpdateButtons();
	void checkWindows();

public slots:
	void LocaleChange(){
	  UpdateButtons();
	}
	void ThemeChange(){
	  UpdateButtons();
	}
	void OrientationChange(){
	  if(this->layout()->direction()==QBoxLayout::LeftToRight){ //horizontal
	    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	    this->layout()->setAlignment(Qt::AlignLeft);
	    QSize sz(this->height(), this->height());
	    for(int i=0; i<BUTTONS.length(); i++){
	      BUTTONS[i]->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	      BUTTONS[i]->setIconSize(sz);
	    }
	  }else{ //vertical
	    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	    this->layout()->setAlignment(Qt::AlignTop);
	    QSize sz(this->width(), this->width());
	    for(int i=0; i<BUTTONS.length(); i++){
	      BUTTONS[i]->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	      BUTTONS[i]->setIconSize(sz);
	    }
	  }
	}
};
#endif