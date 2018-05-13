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
	QString _id;

signals:
	void orientationChanged();

public:
	QBoxLayout *boxLayout;
	bool isPanelPlugin;
	bool isVertical; //only used for panel plugins

	Plugin(QWidget *parent, QString id, bool panelplug = false) : QWidget(parent){
	  isPanelPlugin = panelplug;
	  isVertical = false;
	  _id = id;
	  boxLayout = new QBoxLayout(QBoxLayout::LeftToRight);
	  this->setLayout( boxLayout );
	  boxLayout->setContentsMargins(0,0,0,0);
	  updateLayoutOrientation();
	  connect(this, SIGNAL(orientationChanged()), this, SLOT(updateLayoutOrientation()) );
	}

	void setVertical(bool set){
	  if(set!=isVertical){ isVertical = set; emit orientationChanged(); }
	}

	QString id(){ return _id; }

private slots:
	void updateLayoutOrientation(){
	  boxLayout->setDirection( this->isVertical ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight );
	}
};

//Special subclass for a button-based plugin
class PluginButton : public Plugin{
	Q_OBJECT
private:
	QToolButton *button;

public:
	PluginButton(QWidget *parent, QString id, bool panelplug=false) : Plugin(parent, id, panelplug) {
	  button = new QToolButton(this);
	  this->layout()->addWidget(button);
	}

	~PluginButton(){}
};

#endif
