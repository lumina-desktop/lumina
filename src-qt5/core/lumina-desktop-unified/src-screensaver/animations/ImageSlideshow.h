//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_SCREEN_SAVER_IMAGESLIDESHOW_ANIMATION_H
#define _LUMINA_DESKTOP_SCREEN_SAVER_IMAGESLIDESHOW_ANIMATION_H

#include "global-includes.h"
#include "BaseAnimGroup.h"

class ImageSlideshow: public QParallelAnimationGroup{
	Q_OBJECT
private:
	QLabel *image;
	QPropertyAnimation *bounce, *fading;
	QPixmap pixmap;
	QStringList imageFiles;
	QString imagePath, scriptPath, curpixmap;
	QSize screenSize;
	bool animate, scriptLoad;

private:
	void setupAnimation() {
	  //Choose between starting from top or bottom at random
	  if(qrand() % 2) {
		  bounce->setKeyValueAt(0, QPoint(0,screenSize.height()-image->height()));
		  bounce->setKeyValueAt(0.25, QPoint((screenSize.width()-image->width())/2,0));
		  bounce->setKeyValueAt(0.5, QPoint(screenSize.width()-image->width(),screenSize.height()-image->height()));
		  bounce->setKeyValueAt(0.75, QPoint((screenSize.width()-image->width())/2,0));
		  bounce->setKeyValueAt(1, QPoint(0,screenSize.height()-image->height()));
	  }else{
		  bounce->setKeyValueAt(0, QPoint(0,0));
		  bounce->setKeyValueAt(0.25, QPoint((screenSize.width()-image->width())/2,screenSize.height()-image->height()));
		  bounce->setKeyValueAt(0.5, QPoint(screenSize.width()-image->width(),0));
		  bounce->setKeyValueAt(0.75, QPoint((screenSize.width()-image->width())/2,screenSize.height()-image->height()));
		  bounce->setKeyValueAt(1, QPoint(0,0));
	  }
	}

	void chooseImage() {
	  /*if(scriptLoad){
	    QProcess process;
	    process.start("/home/zwelch/test.sh");
	    process.waitForFinished(1000);
	    QByteArray output = process.readAllStandardOutput();
	    //qDebug() << output;
	    //pixmap.load(randomFile);
	  }else{*/
	    //File Load
	    QString randomFile = curpixmap;
	    if(imageFiles.size()>1 || curpixmap.isEmpty()){
              while(curpixmap==randomFile){ randomFile = imagePath+imageFiles[qrand() % imageFiles.size()]; }
	    }
            if(curpixmap!=randomFile){
              curpixmap = randomFile; //save this for later
	      //no need to load the new file or change the label
              pixmap.load(randomFile);
	      //If the image is larger than the screen, then shrink the image down to 3/4 it's size (so there's still some bounce)
		//Scale the pixmap to keep the aspect ratio instead of resizing the label itself
	      if(pixmap.width() >= (screenSize.width()-10) || pixmap.height() >= (screenSize.height()-10) ){
	        pixmap = pixmap.scaled(screenSize*(3.0/4.0), Qt::KeepAspectRatio);
	      }
	      //Set pixmap to the image label
	      image->setPixmap(pixmap);
	      image->resize(pixmap.size());
	    }
	//}

	}

private slots:
	void LoopChanged(){
	  //Load a new random image. Resize the label based on the image's size
	  chooseImage();
	  setupAnimation();
	}
	void stopped(){ image->hide();}

public:
	ImageSlideshow(QWidget *parent, QString path, bool animate, bool scriptLoad, QString scriptPath) : QParallelAnimationGroup(parent){
	  imagePath = path;
	  image = new QLabel(parent);
	  screenSize = parent->size();
	  this->animate = animate;
	  this->scriptLoad = scriptLoad;
	  this->scriptPath = scriptPath;

	  //Generate the list of files in the directory
	  imageFiles = QDir(imagePath).entryList(QDir::Files);
	  //Ensure all the files are actually images
	  for(int i=0; i<imageFiles.length(); i++){
	    if(QImageReader::imageFormat(imagePath+"/"+imageFiles[i]).isEmpty()){ imageFiles.removeAt(i); i--; }
	  }
	  if(imageFiles.empty()){
	    qDebug() << "Current image file path has no files.";
	    image->setText("No image files found:\n"+imagePath);
	  }else{
	    //Change some default settings for the image. If scaledContents is false, the image will be cut off if resized
	    image->setScaledContents(true);
	    image->setAlignment(Qt::AlignHCenter);
	    //Load a random initial image
	    chooseImage();
	  }

	  //Create the animation that moves the image across the screen
	  bounce = new QPropertyAnimation(image, "pos", parent);

	  //Add the animation that fades the image in and out
	  QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(parent);
	  image->setGraphicsEffect(eff);
	  fading = new QPropertyAnimation(eff,"opacity");
	  fading->setKeyValueAt(0, 0);
	  fading->setKeyValueAt(0.20, 1);
	  fading->setKeyValueAt(0.80, 1);
	  fading->setKeyValueAt(1, 0);
	  this->addAnimation(fading);

	  setupAnimation();
	  image->show();
	  //Only add the animation if set in the configuration file
	  if(animate)
		  this->addAnimation(bounce);
	  else
		  //If no animation, center the image in the middle of the screen
		  image->move(QPoint((parent->width()-image->width())/2,(parent->height()-image->height())/2));

	  //Loop through 15 times for a total for 2 minutes
	  this->setLoopCount(15);
	  bounce->setDuration(8000);
	  fading->setDuration(8000);

	  connect(this, SIGNAL(currentLoopChanged(int)), this, SLOT(LoopChanged()) );
	  connect(this, SIGNAL(finished()), this, SLOT(stopped()) );
	}
	~ImageSlideshow(){}

};

class ImageAnimation: public BaseAnimGroup{
	Q_OBJECT
public:
	ImageAnimation(QWidget *parent) : BaseAnimGroup(parent){}
	~ImageAnimation(){
	  this->stop();
	}

	void LoadAnimations(){
	  canvas->setStyleSheet("background: black;");
	  //Load the path of the images from the configuration file (default /usr/local/backgrounds/)
	  QString imagePath = readSetting("path", LOS::LuminaShare()+"../wallpapers/").toString();
	  //Load whether to animate the image (default true)
	  bool animate = readSetting("animate", true).toBool();
	  bool scriptLoad = readSetting("scriptLoad", true).toBool();
	  QString scriptPath;
	  if(scriptLoad){
	    scriptPath = readSetting("scriptPath", "/usr/local/backgrounds/script.sh").toString();
	  }
	  ImageSlideshow *tmp = new ImageSlideshow(canvas, imagePath, animate, scriptLoad, scriptPath);
	  this->addAnimation(tmp);
	}

};
#endif
