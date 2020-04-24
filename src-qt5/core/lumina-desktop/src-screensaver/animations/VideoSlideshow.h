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

class VideoAnimation: public BaseAnimGroup{
	Q_OBJECT
private:
	QString videoPath, singleVideo;
	QVideoWidget *videoWidget;
	QMediaPlayer *video;
	QStringList videoFiles;
	bool multiple;

private slots:

public:
	VideoAnimation(QWidget *parent) : BaseAnimGroup(parent){}

	~VideoAnimation(){
	  this->stop();
	}

	void LoadAnimations(){
	  canvas->setStyleSheet("background: black;");

	  //Load the path of the videos from the configuration file (default /usr/local/videos/)
	  videoPath = readSetting("path","/usr/local/videos").toString();
    singleVideo = readSetting("videoLocation","").toString();
    multiple = readSetting("multiple",true).toBool();
	  if(!videoPath.endsWith("/")){ videoPath.append("/"); }

	  //Set whether to copy videos on two monitors or play different videos
	  //multimonitor = settings->value("videoSlideshow/multimonitor",true).toBool();

    //Set up the VideoWidget
	  video = new QMediaPlayer(canvas, QMediaPlayer::VideoSurface);
	  videoWidget = new QVideoWidget(canvas);
	  video->setVideoOutput(videoWidget);
	  videoWidget->setGeometry(QRect(QPoint(0,0), canvas->size()));

	  //Generate the list of files in the directory
	  videoFiles = QDir(videoPath).entryList(QDir::Files);
	  if(videoFiles.empty()){
	    qDebug() << "Current video file path has no files:" << videoPath;
	    return;
	  }

    if(singleVideo.isNull())
      singleVideo = videoPath+videoFiles[0];

	  //Loading a random file from a directory
	  QDesktopWidget *dw = new QDesktopWidget();
    QMediaPlaylist *playlist = new QMediaPlaylist();
    if(multiple) {
      for(int i = 0; i < videoFiles.size(); i++){
        playlist->addMedia(QUrl::fromLocalFile(videoPath+videoFiles[i]));
      }
      playlist->shuffle();
    }else{
      playlist->addMedia(QUrl::fromLocalFile(singleVideo));
      playlist->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
    }
	  videoWidget->show();
    if(multiple)
      video->setPlaylist(playlist);

	  //Only play sound for one monitor to prevent messed up audio
	  if(dw->screenNumber(canvas) == 0)
	    video->setVolume(100);
	  else
	    video->setVolume(0);

	  video->play();
	}

};
#endif
