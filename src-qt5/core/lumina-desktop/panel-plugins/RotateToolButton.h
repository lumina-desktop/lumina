//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This is a simple subclass of a QToolButton so it can
// provice text rotated vertically as needed
//===========================================
#ifndef _LUMINA_DESKTOP_ROTATE_TOOLBUTTON_H
#define _LUMINA_DESKTOP_ROTATE_TOOLBUTTON_H

#include <QStylePainter>
#include <QStyleOptionToolButton>
#include <QToolButton>
#include <QTransform>

class RotateToolButton : public QToolButton{
	Q_OBJECT

private:
	int rotate_degrees;
	void paintEvent(QPaintEvent*){
	  /* NOTE: This is what a standard QToolButton performs (peeked at Qt source code for this tidbit)
	  QStylePainter p(this);
	  QStyleOptionToolButton opt;
	  initStyleOption(&opt);
	  p.drawComplexControl(QStyle::CC_ToolButton, opt); 
	  */
	  QStylePainter p(this);
	  QStyleOptionToolButton opt;
	  initStyleOption(&opt);
	  //Apply the rotation matrix to the painter before starting the paint
	  QTransform trans = QTransform( p.transform() ).rotate(rotate_degrees);
	  p.setTransform(trans, false); //merging already taken care of
	  //Now do the normal painting procedure
	  p.drawComplexControl(QStyle::CC_ToolButton, opt); 
	}

public:
	RotateToolButton(QWidget *parent = Q_NULLPTR) : QToolButton(parent){
	  rotate_degrees = 0; //no rotation initially
	}

	void setRotation(int degrees){
	  rotate_degrees = degrees;
	  this->update(); //trigger a paint event
	}

	/*virtual void setText(QString text){
	  this->setText(text);
	  if(rotate_degrees !=0){
	    this->setSizeHint( this->sizeHint()
	  }
	}*/
};

#endif
