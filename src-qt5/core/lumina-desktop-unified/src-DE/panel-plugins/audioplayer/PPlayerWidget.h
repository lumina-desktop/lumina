//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This plugin is a simple audio player on the desktop
//===========================================
#ifndef _LUMINA_PANEL_PLUGIN_AUDIO_PLAYER_WIDGET_H
#define _LUMINA_PANEL_PLUGIN_AUDIO_PLAYER_WIDGET_H

#include <QMediaPlaylist>
#include <QMediaPlayer>
#include <QTimer>
#include <QWidget>
#include <QMenu>


namespace Ui{
	class PPlayerWidget;
};

class PPlayerWidget : public QWidget{
	Q_OBJECT
public:
	PPlayerWidget(QWidget *parent = 0);
	~PPlayerWidget();

public slots:
	void LoadIcons();

private:
	Ui::PPlayerWidget *ui;
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

#endif
