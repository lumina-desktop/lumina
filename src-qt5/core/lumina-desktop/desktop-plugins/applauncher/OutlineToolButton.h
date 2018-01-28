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
#include <QDebug>
#include <QMouseEvent>


class OutlineToolButton : public QToolButton{
	Q_OBJECT
public:
	OutlineToolButton(QWidget *parent=0) : QToolButton(parent){
	  //This button needs slightly different font settings - do this in the constructor so that other widgets can take it into account.
	  QFont font = this->font();
	  font.setStyleStrategy(QFont::PreferAntialias); //Always set the font strategy (just in case it starts working down the road)
	  this->setFont(font);
	  this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	}
	~OutlineToolButton(){}
		
signals:
	void DoubleClicked();
		
protected:
	void mouseDoubleClickEvent(QMouseEvent *ev){
	  ev->accept();
	  emit DoubleClicked();
	}
	void mousePressEvent(QMouseEvent *ev){
	  ev->ignore();
	}
	void mouseReleaseEvent(QMouseEvent *ev){
	  ev->ignore();
	}
	
	void paintEvent(QPaintEvent*){
	  /* NOTE: This is what a standard QToolButton performs (peeked at Qt source code for this tidbit)
	  QStylePainter p(this);
	  QStyleOptionToolButton opt;
	  initStyleOption(&opt);
	  p.drawComplexControl(QStyle::CC_ToolButton, opt); 
	  */
	  
	  //Modify the standard QToolButton routine to paint the text differently
	  QStylePainter p(this);
	  QStyleOptionToolButton opt;
	  initStyleOption(&opt);
	    opt.font = this->property("font").value<QFont>(); //This ensures that the stylesheet values are incorporated
	    opt.font.setStyleStrategy(QFont::PreferAntialias); //Always set the font strategy (just in case it starts working down the road)
	    opt.font.setKerning(true);
	    opt.fontMetrics = QFontMetrics(opt.font);
	    opt.text.clear(); //Don't paint the text yet - just the background/icon
	  p.drawComplexControl(QStyle::CC_ToolButton, opt);  //This does all the normal QToolButton stuff - just not text
	    //Now get the text rectangle for the widget
	    QRect box = p.style()->itemTextRect(opt.fontMetrics, opt.rect, Qt::AlignHCenter | Qt::AlignBottom, true, this->text());
	    //Get the QColors for the outline/text
	    QColor textC = opt.palette.text().color().toHsl(); //need the lightness value in a moment
	    QColor outC = textC;
	      //qDebug() << "Font Color Values:" << textC << textC.lightness() << textC.lightnessF();
	      if(textC.lightnessF() > 0.5){ outC.setHsl(textC.hue(), textC.hslSaturation(), 0, 90); }
	      else{outC.setHsl(textC.hue(), textC.hslSaturation(), 255, 50); }
	      //qDebug() << "Outline Color Values:" << outC;
	    //Now get the size of the outline border (need to scale for high-res monitors)
	    qreal OWidth = opt.fontMetrics.width("o")/2.0;
	      //qDebug() << "Outline Width:" << OWidth;
	    //Now generate a QPainterPath for the text
	    QPainterPath path;
	    QStringList txt = this->text().split("\n"); //need each line independently, the newline actually gets painted otherwise
	    for(int i=0; i<txt.length(); i++){
	      path.addText(box.center().x() - (opt.fontMetrics.width(txt[i])/2), box.y()+((i+1)*(box.height()/txt.length()))-opt.fontMetrics.descent(), opt.font, txt[i] );
	    }
	    path.setFillRule(Qt::WindingFill);
	    //Now paint the text 
	    QRadialGradient RG(box.center(), box.width()*1.5); //width is always going to be greater than height
	      RG.setColorAt(0, outC);
	      RG.setColorAt(1, Qt::transparent);
	    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing); //need antialiasing for this to work well (sub-pixel painting)
	    p.strokePath(path, QPen(QBrush(RG),OWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin) ); //This will be the outline - 1pixel thick, semi-transparent
	    p.fillPath(path, QBrush(textC)); //this will be the inside/text color
	      
	}

};
#endif
