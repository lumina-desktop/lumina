//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This class is the generic container layout for all panel plugins
//  Simply subclass this when creating a new plugin to enable correct
//    visibility and usage within a panel
//===========================================
#ifndef _LUMINA_DESKTOP_PANEL_PLUGIN_H
#define _LUMINA_DESKTOP_PANEL_PLUGIN_H

#include <QObject>
#include <QWidget>
#include <QString>
#include <QBoxLayout>
#include <QApplication>

class LPPlugin : public QWidget{
	Q_OBJECT
	
private:
	QBoxLayout *LY;
	QString plugintype;

public:
	LPPlugin(QWidget *parent = 0, QString ptype="unknown", bool horizontal = true) : QWidget(parent){
	  plugintype=ptype;
	  this->setContentsMargins(1,1,1,1);
	  this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	  this->setFocusPolicy(Qt::NoFocus); //no keyboard focus on the panel/plugins
	  if(horizontal){LY = new QBoxLayout(QBoxLayout::LeftToRight, this); }
	  else{ LY = new QBoxLayout(QBoxLayout::TopToBottom, this); }
	  this->setObjectName(ptype.section("---",0,0));
	  LY->setContentsMargins(0,0,0,0);
	  LY->setSpacing(1);
	  this->setLayout(LY);
	  connect(QApplication::instance(), SIGNAL(LocaleChanged()), this, SLOT(LocaleChange()) );
	  connect(QApplication::instance(), SIGNAL(IconThemeChanged()), this, SLOT(ThemeChange()) );
	}
	
	~LPPlugin(){
	}
	
	QBoxLayout* layout(){
	  return LY;
	}
	
	QString type(){
	  return plugintype;
	}

	virtual void AboutToClose(){
	  //This needs to be re-implemented in the subclasses plugin
	    //This is for any last-minute cleanup before the plugin gets deleted
	}
	
public slots:
	virtual void LocaleChange(){
	  //This needs to be re-implemented in the subclassed plugin
	    //This is where all text is set/translated
	}
	virtual void ThemeChange(){
	  //This needs to be re-implemented in the subclasses plugin
	    //This is where all the visuals are set if using Theme-dependant icons.
	}
	virtual void OrientationChange(){
	  //This needs to be re-implemented in the subclasses plugin
	    //This is where any horizontal/vertical orientations can be changed appropriately		
	}

signals:
	void MenuClosed(); //This needs to be emitted when any plugin's menu is closed for some reason (check/set focus properly)
};

#endif
