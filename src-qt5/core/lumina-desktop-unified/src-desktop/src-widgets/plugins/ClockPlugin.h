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
	    while(textL.length()!=textRows){
	      if(textL.length() > textRows){
	        //Need to get a bit more complicated - join the shorter sections together
	        //Look for an am/pm section and combine that with the previous one
	        int index = textL.lastIndexOf(QRegExp("(AM|am|PM|pm)"));
	        //qDebug() << "Got AM/PM index:" << index;
		 if(index>0){ textL[index-1] = textL[index-1]+" "+textL[index]; textL.removeAt(index); }
	        else{
	          //TO-DO
	          break;
	        }
	      }else{
	        //Need to get a lot more complicated - need to break up sections based on widget width
	        // TO-DO
	        break;
	      }
	    } //end of loop
	    text = textL.join("\n");
	  }
	  //qDebug() << "Got Text:" << text;
	  this->button->setText(text);
	}

public:
	ClockPlugin(QWidget *parent, QString id, bool panelplug) : PluginButton(parent, id, panelplug){
	  connect(RootDesktopObject::instance(), SIGNAL(currentTimeChanged()), this, SLOT(updateTime()) );
	  /*QFont tmp = button->font();
	    tmp.setBold(true);
	  button->setFont( tmp );*/
	  textRows = 1;
	  QTimer::singleShot(0, this, SLOT(updateTime()) );
	}
	~ClockPlugin(){ }

protected:
	void resizeEvent(QResizeEvent *ev){
	  Plugin::resizeEvent(ev);
	  //Re-calculate the text sizing for display
	  int tmp = button->height() / button->fontMetrics().height();
	  //qDebug() << "Got height/font ratio:" << tmp;
         if(tmp!=textRows){
	    textRows = tmp;
	    if(textRows<1){ textRows = 1; }
	    QTimer::singleShot(0, this, SLOT(updateTime()) );
	  }
	}
};

#endif
