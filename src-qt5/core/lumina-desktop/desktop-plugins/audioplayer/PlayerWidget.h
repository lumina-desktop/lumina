//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This plugin is a simple audio player on the desktop
//===========================================
#ifndef _LUMINA_DESKTOP_PLUGIN_AUDIO_PLAYER_WIDGET_H
#define _LUMINA_DESKTOP_PLUGIN_AUDIO_PLAYER_WIDGET_H

#include <QMediaPlaylist>
#include <QMediaPlayer>
#include <QTimer>
#include <QWidget>
#include <QMenu>

#include "../LDPlugin.h"

namespace Ui{
	class PlayerWidget;
};

class PlayerWidget : public QWidget{
	Q_OBJECT
public:
	PlayerWidget(QWidget *parent = 0);
	~PlayerWidget();

public slots:
	void LoadIcons();

private:
	Ui::PlayerWidget *ui;
	QMediaPlaylist *PLAYLIST;
	QMediaPlayer *PLAYER;
	QMenu *configMenu, *addMenu;
	bool updatinglists;

private slots:
	void playClicked();
	void pauseClicked();
	void stopClicked();
	void nextClicked();
	void prevClicked();

	void AddFilesToPlaylist();
	void AddDirToPlaylist();
	void AddURLToPlaylist();
	void ClearPlaylist();
	void ShufflePlaylist();
	void userlistSelectionChanged(); //front-end combobox was changed by the user
	void playerStateChanged();
	void playlistChanged(); //list of items changed
	void currentSongChanged();
	void updateProgress(qint64 val);
	void updateMaxProgress(qint64 val);
};

// Wrapper class to put this into a desktop plugin container
class AudioPlayerPlugin : public LDPlugin{
	Q_OBJECT
public:
	AudioPlayerPlugin(QWidget* parent, QString ID);
	~AudioPlayerPlugin();

	virtual QSize defaultPluginSize(){
	  // The returned QSize is in grid points (typically 100 or 200 pixels square)
	  return QSize(3,1);
	}
	
private:
	PlayerWidget *player;

public slots:
	void LocaleChange(){
	  QTimer::singleShot(0,player, SLOT(LoadIcons()));
	}
	void ThemeChange(){
	  QTimer::singleShot(0,player, SLOT(LoadIcons()));
	}
};

#endif
