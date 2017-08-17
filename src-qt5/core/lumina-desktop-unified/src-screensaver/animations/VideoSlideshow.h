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
	  this->setDuration(1000000);
	  this->setLoopCount(-1);
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
	bool multimonitor;

private slots:
	void startVideo(QAbstractAnimation::State state) {
	  qDebug() << "Status: " << video->mediaStatus() << "New Animation State:" << state;
	  if(state==QAbstractAnimation::Running){
	    video->setVolume(100);
	    video->play();
	  }
	  if(state==QAbstractAnimation::Stopped && video->state()!=QMediaPlayer::StoppedState){
        video->stop();
      }
	}

	void stopVideo() { 
		if(video->state() == QMediaPlayer::StoppedState) { 
			qDebug() << "Stopping Animation"; 
			//this->deleteLater();
			videoWidget->hide(); 
			tmp->stop(); 
			//tmp->deleteLater();
			videoWidget->deleteLater();
			video->deleteLater();
		} 
	}

public:
	VideoAnimation(QWidget *parent, QSettings *set) : BaseAnimGroup(parent, set){}

	~VideoAnimation(){
	  this->stop();
	}

	void LoadAnimations(){
	  canvas->setStyleSheet("background: black;");

	  //Load the path of the videos from the configuration file (default /usr/local/videos/)
	  videoPath = settings->value("videoSlideshow/path","/usr/local/videos").toString();
	  if(!videoPath.endsWith("/")){ videoPath.append("/"); }

	  //Set whether to copy videos on two monitors or play different videos
	  multimonitor = settings->value("videoSlideshow/multimonitor",true).toBool();

	  video = new QMediaPlayer(canvas, QMediaPlayer::VideoSurface);
	  videoWidget = new QVideoWidget(canvas);
	  videoWidget->setGeometry(QRect(QPoint(0,0), canvas->size()));

	  tmp = new VideoSlideshow(canvas, videoWidget);
	  this->addAnimation(tmp);
	  
	  //Generate the list of files in the directory
	  videoFiles = QDir(videoPath).entryList(QDir::Files);
	  if(videoFiles.empty())
		  qDebug() << "Current video file path has no files.";

	  this->setLoopCount(1);

	  QUrl url = QUrl::fromLocalFile(videoPath+videoFiles[qrand() % videoFiles.size()]);
	  video->setMedia(url);
	  qDebug() << url;
	  video->setVideoOutput(videoWidget);
	  videoWidget->show();

	  qDebug() << "VideoWidget Displayed";
	  connect(tmp, SIGNAL(stateChanged(QAbstractAnimation::State, QAbstractAnimation::State)), this, SLOT(startVideo(QAbstractAnimation::State)) );
	  //connect(video, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(startVideo()) );
	  connect(video, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(stopVideo()) );
	}

};
#endif
