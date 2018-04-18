//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This is the Widgets version of a generic plugin
//===========================================
#ifndef _DESKTOP_WIDGETS_GENERIC_PLUGIN_H
#define _DESKTOP_WIDGETS_GENERIC_PLUGIN_H

#include <global-includes.h>

//Base plugin type for a canvas/widget
class Plugin : public QWidget{
	Q_OBJECT
private:
	bool isPanelPlugin;
	bool isVertical; //only used for panel plugins
	QString _id;

signals:
	void orientationChanged();

public:
	Plugin(QWidget *parent, QString id, bool panelplug = false) : QWidget(parent){
	  isPanelPlugin = panelplug;
	  isVertical = false;
	  _id = id;
	}

	void setVertical(bool set){
	  if(set!=isVertical){ isVertical = set; emit orientationChanged(); }
	}

	QString id(){ return _id; }

private slots:

};

//Special subclass for a button-based plugin
class PluginButton : public Plugin{
	Q_OBJECT
private:
	QToolButton *button;

public:
	PluginButton(QWidget *parent, QString id, bool panelplug=false) : Plugin(parent, id, panelplug) {
	  button = new QToolButton(this);
	  this->setLayout( new QBoxLayout(QBoxLayout::LeftToRight) );
	  this->layout()->setContentsMargins(0,0,0,0);
	  this->layout()->addWidget(button);
	}

	~PluginButton(){}
};
#endif
