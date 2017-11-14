//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_VIDEO_ITEM_WIDGET_H
#define _LUMINA_VIDEO_ITEM_WIDGET_H

#include <QWidget>
#include <QVideoWidget>
#include <QMediaPlayer>
#include <QString>
#include <QObject>
#include <QLabel>
#include <QTimer>
#include <QHBoxLayout>
#include <QResizeEvent>

#include <LVideoSurface.h>

class RolloverVideoWidget : public QVideoWidget{
	Q_OBJECT
public:
	RolloverVideoWidget(QWidget *parent = 0) : QVideoWidget(parent){
	  this->setMouseTracking(true);
	}
signals:
	void OverWidget();
	void NotOverWidget();

protected:
	/*void mouseMoveEvent(QMouseEvent *ev){
	  QWidget::mouseMoveEvent(ev);
	  qDebug() << "Got video rollover";
	  emit OverWidget();
	}*/
	void enterEvent(QEvent *ev){
	  QWidget::enterEvent(ev);
	  qDebug() << "Got video enter";
	  emit OverWidget();
	}
	void leaveEvent(QEvent *ev){
	  QWidget::leaveEvent(ev);
	  if(!this->underMouse()){
	    qDebug() << "Got video leave";
	    emit NotOverWidget();
	  }
	}
};

class VideoItemWidget : public QWidget{
	Q_OBJECT
private:
	QLabel *textLabel;
	QMediaPlayer *mplayer;
	LVideoSurface *vwidget;
	QString file, text;
	QSize fixedsize;

public:
	VideoItemWidget(QString filepath, QWidget *parent = 0) : QWidget(parent){
	  file = filepath;
	  text = filepath.section("/",-1);
	  textLabel = 0;
	  mplayer = 0;
	  vwidget = 0;
	  this->setMouseTracking(true);
	  QTimer::singleShot(5, this, SLOT(setupPlayer()) ); //make sure this creation routine is thread-safe
       }
	~VideoItemWidget(){ mplayer->stop(); }

	//Optional overloads - it will try to automatically adjust as needed
	void setText(QString txt){
	  text = txt;
         if(textLabel!=0){
	    textLabel->setText(txt);
	    textLabel->setVisible( !txt.isEmpty() );
	  }
	}
	void setIconSize(QSize sz){
	  fixedsize = sz;
	  if(vwidget!=0 && fixedsize.isValid()){ vwidget->setFixedSize(sz); }
	}

private slots:
	void setupPlayer(){
	  textLabel = new QLabel(this);
	  vwidget = new LVideoSurface(this); //RolloverVideoWidget(this);
	  vwidget->setAspectRatioMode(Qt::IgnoreAspectRatio);
	  if(fixedsize.isValid()){ vwidget->setFixedSize(fixedsize); }
         else{ vwidget->setMinimumSize(QSize(16,16)); }
	  mplayer = new QMediaPlayer(this, QMediaPlayer::VideoSurface);
 	  mplayer->setVideoOutput(vwidget);
	  mplayer->setPlaybackRate(3); // 3x playback speed
	  mplayer->setMuted(true);
	  QHBoxLayout *tmpL = new QHBoxLayout(this);
	  tmpL->setAlignment(Qt::AlignLeft | Qt::AlignCenter);
         tmpL->addWidget(vwidget);
	  tmpL->addWidget(textLabel);
	  tmpL->setStretchFactor(textLabel, 1); //make sure this always occupies all extra space
	  //connect(mplayer, SIGNAL(durationChanged(qint64)), this, SLOT(durationChanged(qint64)) );
	  connect(mplayer, SIGNAL(durationChanged(qint64)), this, SLOT(stopVideo()) );
	  //connect(vwidget, SIGNAL(OverWidget()), this, SLOT(startVideo()) );
	  //connect(vwidget, SIGNAL(NotOverWidget()), this, SLOT(stopVideo()) );
	  //Now load the file
	  textLabel->setText(text);
	  mplayer->setMedia(QUrl::fromLocalFile(file) );
	  mplayer->play();
	}
	void durationChanged(qint64 max){
	  qDebug() << "Got Duration Changed:" << max;
	  mplayer->setPosition( qRound(max/2.0));
	}

	void startVideo(){
	  if(mplayer==0){ return; }
	  if(mplayer->state()!=QMediaPlayer::PlayingState){
	    qDebug() << "Start Video";
	    mplayer->setPosition(0);
	    mplayer->play();
	  }
	}

	void stopVideo(){
	  if(mplayer==0){ return; }
	  if(mplayer->state()==QMediaPlayer::PlayingState){
	    qDebug() << "Stop Video";
	    mplayer->pause();
	    mplayer->setPosition( qRound(mplayer->duration()/2.0) );
	  }
	}

protected:
	void resizeEvent(QResizeEvent *ev){
        if(vwidget!=0 && !fixedsize.isValid()){ vwidget->setFixedSize( QSize(ev->size().height(), ev->size().height()) ); }
	}
	/*void mouseMoveEvent(QMouseEvent *ev){
	  QWidget::mouseMoveEvent(ev);
	  stopVideo(); //not over the video widget any more
	}*/
};

#endif
