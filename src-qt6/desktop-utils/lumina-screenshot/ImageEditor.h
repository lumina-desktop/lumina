//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This is a simple widget for viewing a QImage and allowing the user to 
//   highlight/select portions of the image with possible actions on the selection
//===========================================
#ifndef _LUMINA_SCREENSHOT_IMAGE_EDITOR_H
#define _LUMINA_SCREENSHOT_IMAGE_EDITOR_H

#include <QImage>
#include <QWidget>
#include <QRect>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QMenu>

class ImageEditor : public QWidget{
	Q_OBJECT
public:
	ImageEditor(QWidget*parent = 0);
	~ImageEditor();

	void LoadImage(QImage img);
	void setDefaultSize(QSize sz);
	bool hasSelection();
	QImage image();

	int getScalingValue();

private:
	QImage fullIMG, scaledIMG; //Note: the aspect ratio between the two images must be preserved!!
	QSize defaultSize; //for loading new images
	QRect selRect; //selection rectangle (scaledIMG coordinates)
	QPoint selPoint; //initial selection point (used during click/drags)
	QMenu *contextMenu;

	//simplification functions
	qreal getScaleFactor(){
         //return the scale factor between the full/scaled images
        return ( ( (qreal) scaledIMG.height()) / ( (qreal) fullIMG.height()) );
	}

	void rescaleImage(qreal scfactor){
        if(fullIMG.isNull()){ return; }
	  scaledIMG = fullIMG.scaled( fullIMG.width()*scfactor, fullIMG.height()*scfactor, Qt::KeepAspectRatio,Qt::SmoothTransformation);
	  selRect = QRect();
	  emit selectionChanged(false);
        this->update(); //trigger a repaint event
	}

private slots:
	void showMenu();

public slots:
	void setScaling(int perc); //10% <--> 200% range is valid
	void scaleUp(int val = 10); //10% change by default
	void scaleDown(int val = 10); //10% change by default

	void cropImage();
	void resizeImage();

protected:
	void mousePressEvent(QMouseEvent *ev);
	void mouseMoveEvent(QMouseEvent *ev);
	void mouseReleaseEvent(QMouseEvent *);
	void paintEvent(QPaintEvent*);

signals:
	void selectionChanged(bool); //true if there is a selection
	void scaleFactorChanged(int);
};
#endif
