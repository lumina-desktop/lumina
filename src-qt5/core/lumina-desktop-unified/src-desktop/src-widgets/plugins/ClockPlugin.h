//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2018, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#ifndef _LUMINA_DESKTOP_PLUGIN_CLOCK_H
#define _LUMINA_DESKTOP_PLUGIN_CLOCK_H

#include <Plugin.h>
#include <RootDesktopObject.h>
#include <CalendarMenu.h>

class ClockPlugin : public PluginButton{
	Q_OBJECT

private:
	int textRows;

private slots:
	void updateTime(){
	  //qDebug() << "Clock: Update Time";
	  QString text = RootDesktopObject::instance()->currentTime();
	  //Adjust the text to fix the widget as needed
	  //qDebug() << "Clock: Update Time:" << textRows << text << text.split(" ");
	  if(textRows>1){
	    QStringList textL = text.split(" "); //natural breaks
	    int max = button->width() + (isVertical ? -1 : 200); //button->height() for vertical panels once text rotation is worked out
	    while(textL.length()!=textRows){
	      if(textL.length() > textRows){
	        //Need to get a bit more complicated - join the shorter sections together
	        //Find the smallest section and merge it into the previous one
	        //Note: This is horribly inefficient, but due to the inherently short length of textL it is acceptable
	        int min = -1; int index = -1;
	        for(int i=1; i<textL.length(); i++){
	          int combined = button->fontMetrics().width(textL[i-1]+" "+textL[i]);
                 int tmp = button->fontMetrics().width(textL[i]);
	          if( max>combined && (min<0 || tmp<min)){ min = tmp; index = i; }
	        }
	        if(index>0){ textL[index-1] = textL[index-1]+" "+textL[index]; textL.removeAt(index); }
	        else{ break; }
	      }else{
	        //Need to get a lot more complicated - need to break up sections mid-word based on widget width
	        // NOT WORKING - can cause infinite loop somehow (perhaps max is 0 on init?)
	        /*for(int i=0; i<textL.length(); i++){
	          int tmp = button->fontMetrics().width(textL[i]);
	          if(tmp>max){
	            //qDebug() << "CLOCK:" << i << tmp << max << textL[i];
	            QString tmps = button->fontMetrics().elidedText(textL[i], Qt::ElideRight, max).section("...",0,-2); //remove the three dots at end
	            QString rem = textL[i].remove(tmps);
                   textL[i] = tmps; textL.insert(i+1, rem);
	          }
	        }*/
	        //Now go ahead and break the main loop - text all fits width-wise now
	        break;
	      }
	    } //end of loop
	    text = textL.join("\n");
	  }
	  //qDebug() << "Got Text:" << text;
	  this->button->setText(text);
	}

public:
	ClockPlugin(QWidget *parent, QString id, bool panelplug, bool vertical) : PluginButton(parent, id, panelplug, vertical){
	  connect(RootDesktopObject::instance(), SIGNAL(currentTimeChanged()), this, SLOT(updateTime()) );
	  QFont tmp = button->font();
	    tmp.setBold(true);
	  button->setFont( tmp );
	  textRows = 1;
	  //Setup the popup menu
	  static QMenu *calMenu = 0;
	  if(calMenu == 0){
	    calMenu = new CalendarMenu();
	  }
	  button->setMenu(calMenu);
	  QTimer::singleShot(0, this, SLOT(updateTime()) );
	}
	~ClockPlugin(){ }

protected:
	void resizeEvent(QResizeEvent *ev){
	  Plugin::resizeEvent(ev);
	  //Re-calculate the text sizing for display
	  int tmp = button->height() / button->fontMetrics().height();
	  if(isVertical && isPanelPlugin){ tmp+=5; } //non-rotated text - need more columns
	  //qDebug() << "Got height/font ratio:" << tmp;
         if(tmp!=textRows){
	    textRows = tmp;
	    if(textRows<1){ textRows = 1; }
	    if(!button->text().isEmpty()){ QTimer::singleShot(30, this, SLOT(updateTime()) ); }
	  }
	}
};

#endif
