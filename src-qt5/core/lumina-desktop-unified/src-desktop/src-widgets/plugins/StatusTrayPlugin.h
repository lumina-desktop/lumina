//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_PLUGIN_STATUSTRAY_H
#define _LUMINA_DESKTOP_PLUGIN_STATUSTRAY_H

#include <Plugin.h>
#include <SystemTrayMenu.h>
#include <VolumeButton.h>
#include <BatteryButton.h>
#include <NetworkButton.h>
#include <UpdatesButton.h>

#include <global-includes.h>

class StatusTrayPlugin : public PluginButton{
	Q_OBJECT

private:

private slots:
	void updateIcons(){
	  QStringList icons; icons << "open-menu" << "show-menu" << "view-grid" << "up" << "arrow-up";
	  for(int i=0; i<icons.length(); i++){
	    if(QIcon::hasThemeIcon(icons[i])){ button->setIcon( QIcon::fromTheme(icons[i])); break; }
	  }
	}

public:
	StatusTrayPlugin(QWidget *parent, QString id, bool panelplug, bool vertical) : PluginButton(parent, id, panelplug, vertical){
	  //Setup the popup menu for the system tray
	  static QMenu *systrayMenu = 0;
	  if(systrayMenu == 0){
	    systrayMenu = new SystemTrayMenu();
	  }
	  button->setMenu(systrayMenu);
	  connect(systrayMenu, SIGNAL(hasTrayIcons(bool)), button, SLOT(setVisible(bool)) );
	  updateIcons();
	  //Now create/add all the other buttons
	  if(OSInterface::instance()->OS_volumeSupported()){ boxLayout->addWidget( new VolumeButton(this) ); }
	  boxLayout->addWidget( new NetworkButton(this) );
	  if(OSInterface::instance()->batteryAvailable()){ boxLayout->addWidget( new BatteryButton(this) ); }
	  if(OSInterface::instance()->updatesSupported()){ boxLayout->addWidget( new UpdatesButton(this) ); }
	}
	~StatusTrayPlugin(){ }

};

#endif
