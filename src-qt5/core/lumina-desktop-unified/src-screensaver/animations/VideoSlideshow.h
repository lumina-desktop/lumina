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
	QString videoPath;
	QVideoWidget *videoWidget;
	QMediaPlayer *video;
	QStringList videoFiles;
	//bool multimonitor;

private slots:

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
	  //multimonitor = settings->value("videoSlideshow/multimonitor",true).toBool();

    //Set up the VideoWidget
	  video = new QMediaPlayer(canvas, QMediaPlayer::VideoSurface);
	  videoWidget = new QVideoWidget(canvas);
	  video->setVideoOutput(videoWidget);
	  videoWidget->setGeometry(QRect(QPoint(0,0), canvas->size()));
	  
	  //Generate the list of files in the directory
	  videoFiles = QDir(videoPath).entryList(QDir::Files);
	  if(videoFiles.empty())
		  qDebug() << "Current video file path has no files.";

    //Loading a random file from a directory
    QDesktopWidget *dw = new QDesktopWidget();
    QMediaPlaylist *playlist = new QMediaPlaylist();
    for(int i = 0; i < videoFiles.size(); i++)
      playlist->addMedia(QUrl::fromLocalFile(videoPath+videoFiles[i]));
    qsrand(QTime::currentTime().msec());
    playlist->setCurrentIndex(qrand() % videoFiles.size());
    playlist->setPlaybackMode(QMediaPlaylist::Random);
	  videoWidget->show();
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
