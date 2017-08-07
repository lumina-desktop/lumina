//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_SCREEN_SAVER_VIDEOSLIDESHOW_ANIMATION_H
#define _LUMINA_DESKTOP_SCREEN_SAVER_VIDEOSLIDESHOW_ANIMATION_H

#include "global-includes.h"
#include "BaseAnimGroup.h"

class VideoSlideshow: public QPropertyAnimation{
	Q_OBJECT
public:
	VideoSlideshow(QWidget *parent, QVideoWidget *videoWidget) : QPropertyAnimation(videoWidget, "pos", parent){
	  this->setKeyValueAt(0,QPoint(0,0));
	  this->setKeyValueAt(1,QPoint(0,0));
	}
	~VideoSlideshow(){}

};

class VideoAnimation: public BaseAnimGroup{
	Q_OBJECT
private:
	QString videoPath;
	VideoSlideshow *tmp;
	QVideoWidget *videoWidget;
	QMediaPlayer *video;
	QStringList videoFiles;
	QMediaPlaylist *playlist;
	bool multimonitor, random;

private slots:
	void startVideo() {
	  this->addAnimation(tmp);
	  tmp->setDuration(video->duration());
	  qDebug() << "Status: " << video->mediaStatus();
	  video->setPlaylist(playlist);
	  video->setVolume(100);
	  video->play();
	}

	void LoopChanged(){
		qDebug() << "New Video";
		if(random)
			playlist->setCurrentIndex(qrand() % videoFiles.size());
		else
			playlist->setCurrentIndex(playlist->currentIndex()+1);
	}

	void stopped(){qDebug() << "Video Stopped"; videoWidget->hide();}

public:
	VideoAnimation(QWidget *parent, QSettings *set) : BaseAnimGroup(parent, set){}

	~VideoAnimation(){
	  this->stop();
	}

	void LoadAnimations(){
	  canvas->setStyleSheet("background: black;");

	  //Load the path of the videos from the configuration file (default /usr/local/videos/)
	  videoPath = settings->value("videoSlideshow/path","/usr/local/videos/").toString();

	  //Set whether to copy videos on two monitors or play different videos
	  multimonitor = settings->value("videoSlideshow/multimonitor",true).toBool();

	  //Set whether to play random videos or in order
	  random = settings->value("videoSlideshow/random",false).toBool();

	  video = new QMediaPlayer(canvas, QMediaPlayer::VideoSurface);
	  videoWidget = new QVideoWidget(canvas);

	  tmp = new VideoSlideshow(canvas, videoWidget);

	  //Generate the list of files in the directory
	  videoFiles = QDir(videoPath).entryList(QDir::Files);
	  if(videoFiles.empty())
		  qDebug() << "Current video file path has no files.";

	  this->setLoopCount(videoFiles.size());

	  //Load a random initial video
	  playlist = new QMediaPlaylist();
	  for(int i = 0; i < videoFiles.size(); i++)
		  playlist->addMedia(QUrl::fromLocalFile(videoFiles[i]));
	  if(random)
		  playlist->setCurrentIndex(qrand() % videoFiles.size());

	  video->setVideoOutput(videoWidget);
	  videoWidget->show();
	  qDebug() << "VideoWidget Displayed";
	  connect(video, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(startVideo()));
	  connect(this, SIGNAL(currentLoopChanged(int)), this, SLOT(LoopChanged()) );
	  connect(this, SIGNAL(finished()), this, SLOT(stopped()) );
	}

};
#endif
