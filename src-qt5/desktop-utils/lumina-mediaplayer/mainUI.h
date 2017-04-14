//===========================================
//  Lumina-Desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_MEDIA_PLAYER_MAIN_UI_H
#define _LUMINA_MEDIA_PLAYER_MAIN_UI_H

#include <QMainWindow>
#include <QAction>
#include <QString>
#include <QStringList>
#include <QSystemTrayIcon>
#include <QCloseEvent>

//QMultimedia classes
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QVideoWidget>

#include "PianoBarProcess.h"

namespace Ui{
	class MainUI;
};

class MainUI : public QMainWindow{
	Q_OBJECT
public:
	MainUI();
	~MainUI();

	void loadArguments(QStringList);

private:
	Ui::MainUI *ui;
	PianoBarProcess *PANDORA;
	QMediaPlayer *PLAYER;
	QVideoWidget *VIDEO;
	QMediaPlaylist *PLAYLIST;
	QSystemTrayIcon *SYSTRAY;
	bool closing;

	void setupPlayer();
	void setupPandora();
	void setupConnections();
	void setupIcons();
	void setupTrayIcon();
	void closeTrayIcon();

private slots:
	void closeApplication();
	void PlayerTypeChanged(bool active = true);

	//Toolbar actions
	void playToggled();
	void pauseToggled();
	void stopToggled();
	void nextToggled();
	void backToggled();
	void volupToggled();
	void voldownToggled();

	//Local Playback UI slots
	void setLocalPosition(int pos){ PLAYER->setPosition(pos); }
	void addLocalMedia();
	void rmLocalMedia();
	void localPlaybackSettingsChanged();

	//Local Playlist Feedback
	void LocalListIndexChanged(int); //item being played just changed
	void LocalListMediaChanged(int,int);
	void LocalListMediaInserted(int,int);
	void LocalListMediaRemoved(int,int);

	//Local Player Feedback
	//void LocalAudioAvailable(bool);
	void LocalVideoAvailable(bool);
	void LocalIsSeekable(bool);
	void LocalNowMuted(bool);
	void LocalError(QMediaPlayer::Error);
	void LocalMediaChanged(const QMediaContent&);
	void LocalMediaStatusChanged(QMediaPlayer::MediaStatus);
	void LocalStateChanged(QMediaPlayer::State);
	void LocalDurationChanged(qint64);
	void LocalPositionChanged(qint64);
	void LocalVolumeChanged(int);

	//Pandora Options
	void showPandoraSongInfo();
	void changePandoraStation(QString);
	void checkPandoraSettings();
	void applyPandoraSettings();
	//Pandora Process Feedback
	void PandoraStateChanged(PianoBarProcess::State);
	void NewPandoraInfo(QString);
	void PandoraStationChanged(QString);
	void PandoraSongChanged(bool, QString, QString, QString, QString, QString); //[isLoved, title, artist, album, detailsURL, fromStation]
	void PandoraTimeUpdate(int,int); //current secs, total secs
	void PandoraStationListChanged(QStringList);
	void PandoraListInfo(QStringList);

	//System Tray interactions
	void toggleVisibility();
	void trayMessageClicked();
	void trayClicked(QSystemTrayIcon::ActivationReason);

protected:
	void closeEvent(QCloseEvent *ev);
	
};
#endif
