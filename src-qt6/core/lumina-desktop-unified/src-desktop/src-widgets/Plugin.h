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

	//These static functions are defined in "Plugin.cpp"
	static QStringList built_in_plugins();
	static Plugin* createPlugin(QWidget *parent, QString id, bool panelplug = false, bool vertical = false);

	Plugin(QWidget *parent, QString id, bool panelplug = false, bool vertical = false) : QWidget(parent){
	  isPanelPlugin = panelplug;
	  isVertical = vertical;
	  _id = id;
	  boxLayout = new QBoxLayout(QBoxLayout::LeftToRight);
	  this->setLayout( boxLayout );
	  boxLayout->setContentsMargins(0,0,0,0);
	  this->setContentsMargins(0,0,0,0);
	  updateLayoutOrientation();
	  connect(this, SIGNAL(orientationChanged()), this, SLOT(updateLayoutOrientation()) );
	}

	void setVertical(bool set){
	  if(set!=isVertical){ isVertical = set; emit orientationChanged(); }
	  setupSizing();
	}

	QString id(){ return _id; }

	virtual void setupSizing(){
	  if(isPanelPlugin){
	    if(!isVertical){ this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding); }
	    else{ this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred); }
	  }else{
	    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	  }
	}

private slots:
	void updateLayoutOrientation(){
	  boxLayout->setDirection( this->isVertical ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight );
	}
};

//Special subclass for a button-based plugin
class PluginButton : public Plugin{
	Q_OBJECT

public:
	QToolButton *button;
	PluginButton(QWidget *parent, QString id, bool panelplug=false, bool vertical = false) : Plugin(parent, id, panelplug, vertical) {
	  button = new QToolButton(this);
	  button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	  button->setAutoRaise(true);
	  button->setPopupMode(QToolButton::InstantPopup);
	  this->layout()->addWidget(button);
	}

	~PluginButton(){}
};

#endif
