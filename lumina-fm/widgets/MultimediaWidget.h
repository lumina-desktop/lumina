//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_FM_MULTIMEDIA_WIDGET_H
#define _LUMINA_FM_MULTIMEDIA_WIDGET_H

#include <QList>
#include <QWidget>
#include <QObject>
#include <QMediaObject>
#include <QMediaPlayer>
#include <QVideoWidget>

#include "../DirData.h"

namespace Ui{
	class MultimediaWidget;
};

class MultimediaWidget : public QWidget{
	Q_OBJECT
public:
	MultimediaWidget(QWidget *parent = 0);
	~MultimediaWidget();

public slots:
	void LoadMultimedia(QList<LFileInfo> list);

	//Theme change functions
	void UpdateIcons();
	void UpdateText();

private:
	Ui::MultimediaWidget *ui;
	QMediaPlayer *mediaObj;
	QVideoWidget *videoDisplay;
	QString playerTTime; //total time - to prevent recalculation every tick

	QString msToText(qint64 ms);

private slots:
	//Media Object functions
	void playerStatusChanged(QMediaPlayer::MediaStatus stat);
	void playerStateChanged(QMediaPlayer::State newstate);
	void playerVideoAvailable(bool showVideo);
	void playerDurationChanged(qint64 dur);
	void playerTimeChanged(qint64 ctime);
	void playerError();
	void playerFinished();

	//The UI functions
	void on_tool_player_play_clicked();
	void on_combo_player_list_currentIndexChanged(int index);
	void on_tool_player_next_clicked();
	void on_tool_player_prev_clicked();
	void on_tool_player_pause_clicked();
	void on_tool_player_stop_clicked();

	//Slider controls
	void on_playerSlider_sliderPressed();
	void on_playerSlider_sliderReleased();
	void on_playerSlider_valueChanged(int val);


};
#endif