//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015-2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This class is the container which provides the screensaver animations
//  and should be subclassed for each of the various animation types
//===========================================
#ifndef _LUMINA_DESKTOP_SCREEN_SAVER_BASE_ANIMATION_GROUP_H
#define _LUMINA_DESKTOP_SCREEN_SAVER_BASE_ANIMATION_GROUP_H

#include "global-includes.h"

class BaseAnimGroup : public QParallelAnimationGroup{
	Q_OBJECT
public:
	QWidget *canvas;
	QSettings *settings;

	virtual void LoadAnimations(){} //This is the main function which needs to be subclassed

	BaseAnimGroup(QWidget *parent, QSettings *set){
	  canvas = parent;
	  settings = set;
	  canvas->setCursor( QCursor(Qt::BlankCursor) );
	}
	~BaseAnimGroup(){}
	
	//==============================
	//     PLUGIN LOADING/LISTING (Change in the .cpp file)
	//==============================
	static BaseAnimGroup* NewAnimation(QString type, QWidget *parent, QSettings *set);
	static QStringList KnownAnimations();
	
};

#endif
