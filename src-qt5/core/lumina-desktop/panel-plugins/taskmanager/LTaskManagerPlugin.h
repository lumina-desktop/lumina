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
#include <QDateTime>

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
//	int vertsizeicon;
//	int *dpi;  //this comes from the PCDM dpi

private:
	QList<LTaskButton*> BUTTONS; //to keep track of the current buttons
	QTimer *timer;
	QDateTime updating; //quick flag for if it is currently working
	bool usegroups;

private slots:
	void UpdateButtons();
	void UpdateButton(WId win);
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
//            QSize sz(this->width(), this->height());  //we want to increase the width but not the height of the icons
	    for(int i=0; i<BUTTONS.length(); i++){
	      BUTTONS[i]->setToolButtonStyle(Qt::ToolButtonIconOnly);
//	      BUTTONS[i]->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
//		if( dpi = 196 ){ int vertsizeicon = ; BUTTONS[i]->setIconSize(vertsizeicon);}
//		elseif( dpi = 144 ) { int vertsizeicon = ; BUTTONS[i]->setIconSize(vertsizeicon);}
//		elseif( dpi = 96 ) { int vertsizeicon = ; BUTTONS[i]->setIconSize(vertsizeicon);}
//		elseif( dpi = 48 ) { int vertsizeicon = ; BUTTONS[i]->setIconSize)vertsizeicon);}
	      BUTTONS[i]->setIconSize(sz);
	    }
	  }
	}
};
#endif
