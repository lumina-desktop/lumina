//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is a simple subclass for a QToolButton with black/white text (for transparent backgrounds)
//===========================================
#ifndef _LUMINA_DESKTOP_PLUGIN_APPLAUNCHER_OUTLINE_TOOLBUTTON_H
#define _LUMINA_DESKTOP_PLUGIN_APPLAUNCHER_OUTLINE_TOOLBUTTON_H

#include <QToolButton>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QStyle>
#include <QStyleOption>
#include <QStylePainter>
#include <QFont>

class OutlineToolButton : public QToolButton{
	Q_OBJECT
public:
	OutlineToolButton(QWidget *parent=0) : QToolButton(parent){
	  QFont font = this->font();
	    font.setStyleStrategy(QFont::ForceOutline);// | QFont::PreferQuality);
	  this->setFont(font);	
	}
	~OutlineToolButton(){}
		
protected:
	void paintEvent(QPaintEvent*){

	  //QPainter painter(this);
	  //QPainterPath path;
	  //QPen pen;
	    //pen.setWidth(2);
	    //pen.setColor(Qt::red);
	  //painter.setFont(this->font());
	  //painter.setPen(pen);
	  //path.addText(10 , 60, this->font(), this->text()); //Adjust the position
	  //painter.drawPath(path);
	  QFont font = this->font();
	    font.setStyleStrategy(QFont::ForceOutline);// | QFont::PreferQuality);
	  //This is what a QToolButton performs
	  QStylePainter p(this);
	    //p.setPen(pen);
	  QStyleOptionToolButton opt;
	  initStyleOption(&opt);
	    opt.font = font; //Use the font which forces outlines
	  //p.style()->drawControl(QStyle::CE_ToolButtonLabel, &opt, &p, this); //this does the outline underneath
	  p.drawComplexControl(QStyle::CC_ToolButton, opt); //This does the normal painting on top
	  //Now do the normal paint event over the top
	  //QToolButton::paintEvent(ev); 
	}

};
#endif
