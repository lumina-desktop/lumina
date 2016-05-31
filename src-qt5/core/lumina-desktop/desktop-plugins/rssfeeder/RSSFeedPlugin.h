//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This plugin is a simple RSS feed reader for the desktop
//===========================================
#ifndef _LUMINA_DESKTOP_RSS_FEEDER_PLUGIN_H
#define _LUMINA_DESKTOP_RSS_FEEDER_PLUGIN_H

#include <QTimer>
#include "../LDPlugin.h"

namespace Ui{
	class RSSFeedPlugin;
};

class RSSFeedPlugin : public LDPlugin{
	Q_OBJECT
public:
	RSSFeedPlugin(QWidget* parent, QString ID);
	~RSSFeedPlugin();
	
	virtual QSize defaultPluginSize(){
	  // The returned QSize is in grid points (typically 100 or 200 pixels square)
	  return QSize(3,3);
	}
private:
	Ui::RSSFeedPlugin *ui;
	QMenu *optionsMenu;
	QString setprefix; //settings prefix

	void updateOptionsMenu();

	//Simplification functions for loading feed info onto widgets
	void updateFeed(QString ID);
	void updateFeedInfo(QString ID);

private slots:
	void loadIcons();

	//GUI slots
	// - Page management
	void backToFeeds();
	void openFeedInfo();
	void openFeedNew();
	void openSettings();
	// - Feed Management
	void addNewFeed(); // the "add" button (current url in widget on page)
	void removeFeed(); // the "remove" button (current feed for page)
	// - Feed Interactions
	void currentFeedChanged();
	void openFeedPage(); //Open the website in a browser
	void saveSettings();

	//Feed Object interactions
	void UpdateFeedList();
	void RSSItemChanged(QString ID);

public slots:
	void LocaleChange();
	void ThemeChange();

};
#endif
