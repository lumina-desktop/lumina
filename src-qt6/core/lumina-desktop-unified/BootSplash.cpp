#include "BootSplash.h"
#include "ui_BootSplash.h"

#include <LuminaXDG.h>
#include <LUtils.h>
#include <LDesktopUtils.h>

BootSplash::BootSplash() : QWidget(0, Qt::SplashScreen | Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus), ui(new Ui::BootSplash){
  ui->setupUi(this);
  this->setObjectName("LuminaBootSplash"); //for theme styling
  //Center the window on the primary screen
  this->show();
  QPoint ctr = QApplication::desktop()->screenGeometry().center();
  this->move( ctr.x()-(this->width()/2), ctr.y()-(this->height()/2) );
  generateTipOfTheDay();
  ui->label_version->setText( QString(tr("Version %1")).arg(LDesktopUtils::LuminaDesktopVersion()) );
}

void BootSplash::generateTipOfTheDay(){
  //Try to find a system-defined message of the day for lumina
  QStringList dirs; dirs << LOS::AppPrefix()+"/etc/" << LOS::SysPrefix()+"/etc/" << L_ETCDIR+"/";
  QString sysMOTD = "lumina-motd";
  for(int i=0; i<dirs.length(); i++){
    if(QFile::exists(dirs[i]+sysMOTD)){ sysMOTD.prepend(dirs[i]); break; }
  }

  QString tip;
  if(sysMOTD.contains("/") && LUtils::isValidBinary(sysMOTD)){
    //is binary - run it to generate text
    tip = LUtils::getCmdOutput(sysMOTD).join("\n");

  }else if(QFile::exists(sysMOTD)){
    //text file - read it to generate text
    tip = LUtils::readFile(sysMOTD).join("\n");

  }else{
    int index = qrand()%46; //Make sure this number matches the length of the case below (max value +1)
  switch(index){
    case 0:
	tip = tr("This desktop is powered by coffee, coffee, and more coffee."); break;
    case 1:
	tip = tr("Keep up with desktop news!")+"\n\nwww.lumina-desktop.org"; break;
    case 2:
	tip = tr("There is a full handbook of information about the desktop available online.")+"\n\nwww.lumina-desktop.org/handbook"; break;
    case 3:
	tip = tr("Want to change the interface? Everything is customizable in the desktop configuration!"); break;
    case 4:
	tip = tr("Lumina can easily reproduce the interface from most other desktop environments."); break;
    case 5:
	tip = tr("This desktop is generously sponsored by iXsystems")+"\n\nwww.ixsystems.com"; break;
    case 6:
	tip = "\""+tr("I have never been hurt by what I have not said")+"\"\n\n- Calvin Coolidge -"; break;
    case 7:
	tip = "\""+tr("Gotta have more cowbell!")+"\"\n\n- Christopher Walken (SNL) -"; break;
    case 8:
	tip = "\""+tr("Everything has its beauty but not everyone sees it.")+"\"\n\n- Confucius -"; break;
    case 9:
	tip = "\""+tr("Before God we are all equally wise - and equally foolish.")+"\"\n\n- Albert Einstein -"; break;
    case 10:
	tip = "\""+tr("We cannot do everything at once, but we can do something at once.")+"\"\n\n- Calvin Coolidge -"; break;
    case 11:
	tip = "\""+tr("One with the law is a majority.")+"\"\n\n- Calvin Coolidge -"; break;
    case 12:
	tip = "\""+tr("Don't expect to build up the weak by pulling down the strong.")+"\"\n\n- Calvin Coolidge -"; break;
    case 13:
	tip = "\""+tr("You can't know too much, but you can say too much.")+"\"\n\n- Calvin Coolidge -"; break;
    case 14:
	tip = "\""+tr("Duty is not collective; it is personal.")+"\"\n\n- Calvin Coolidge -"; break;
    case 15:
	tip = "\""+tr("Any society that would give up a little liberty to gain a little security will deserve neither and lose both.")+"\"\n\n- Benjamin Franklin -"; break;
    case 16:
	tip = "\""+tr("Never trust a computer you can’t throw out a window.")+"\"\n\n- Steve Wozniak -"; break;
    case 17:
	tip = "\""+tr("Study the past if you would define the future.")+"\"\n\n- Confucius -"; break;
    case 18:
	tip = "\""+tr("The way to get started is to quit talking and begin doing.")+"\"\n\n- Walt Disney -"; break;
    case 19:
	tip = "\""+tr("Ask and it will be given to you; search, and you will find; knock and the door will be opened for you.")+"\"\n\n- Jesus Christ -"; break;
    case 20:
	tip = "\""+tr("Start where you are. Use what you have.  Do what you can.")+"\"\n\n- Arthur Ashe -"; break;
    case 21:
	tip = "\""+tr("A person who never made a mistake never tried anything new.")+"\"\n\n- Albert Einstein -"; break;
    case 22:
	tip = "\""+tr("It does not matter how slowly you go as long as you do not stop.")+"\"\n\n- Confucius -"; break;
    case 23:
	tip = "\""+tr("Do what you can, where you are, with what you have.")+"\"\n\n- Theodore Roosevelt -"; break;
    case 24:
	tip = "\""+tr("Remember no one can make you feel inferior without your consent.")+"\"\n\n- Eleanor Roosevelt -"; break;
    case 25:
	tip = "\""+tr("It’s not the years in your life that count. It’s the life in your years.")+"\"\n\n- Abraham Lincoln -"; break;
    case 26:
	tip = "\""+tr("Either write something worth reading or do something worth writing.")+"\"\n\n- Benjamin Franklin -"; break;
    case 27:
	tip = "\""+tr("The only way to do great work is to love what you do.")+"\"\n\n- Steve Jobs -"; break;
    case 28:
	tip = "\""+tr("Political correctness is tyranny with manners.")+"\"\n\n- Charlton Heston -"; break;
    case 29:
	tip = "\""+tr("Only two things are infinite, the universe and human stupidity, and I'm not sure about the former.")+"\"\n\n- Albert Einstein -"; break;
    case 30:
	tip = "\""+tr("I find that the harder I work, the more luck I seem to have.")+"\"\n\n- Thomas Jefferson -"; break;
    case 31:
	tip = "\""+tr("Do, or do not. There is no 'try'.")+"\"\n\n- Yoda -"; break;
    case 32:
	tip = "\""+tr("A mathematician is a device for turning coffee into theorems.")+"\"\n\n- Paul Erdos -"; break;
    case 33:
	tip = "\""+tr("Good people do not need laws to tell them to act responsibly, while bad people will find a way around the laws.")+"\"\n\n- Plato -"; break;
    case 34:
	tip = "\""+tr("Black holes are where God divided by zero.")+"\"\n\n- Steven Wright -"; break;
    case 35:
	tip = "\""+tr("It's kind of fun to do the impossible.")+"\"\n\n- Walt Disney -"; break;
    case 36:
	tip = "\""+tr("Knowledge speaks, but wisdom listens.")+"\"\n\n- Jimi Hendrix -"; break;
    case 37:
	tip = "\""+tr("A witty saying proves nothing.")+"\"\n\n- Voltaire -"; break;
    case 38:
	tip = "\""+tr("Success usually comes to those who are too busy to be looking for it.")+"\"\n\n- Henry David Thoreau -"; break;
    case 39:
	tip = "\""+tr("Well-timed silence hath more eloquence than speech.")+"\"\n\n- Martin Fraquhar Tupper -"; break;
    case 40:
	tip = "\""+tr("I have never let my schooling interfere with my education.")+"\"\n\n- Mark Twain -"; break;
    case 41:
	tip = "\""+tr("The best way to predict the future is to invent it.")+"\"\n\n- Alan Kay -"; break;
    case 42:
	tip = "\""+tr("Well done is better than well said.")+"\"\n\n- Benjamin Franklin -"; break;
    case 43:
	tip = "\""+tr("Sometimes it is not enough that we do our best; we must do what is required.")+"\"\n\n- Sir Winston Churchill -"; break;
    case 44:
	tip = "\""+tr("The truth is more important than the facts.")+"\"\n\n- Frank Lloyd Wright -"; break;
    case 45:
	tip = "\""+tr("Better to remain silent and be thought a fool than to speak out and remove all doubt.")+"\"\n\n- Abraham Lincoln -"; break;
  } //end of switch for tips

  } //end of fallback tip generation
  ui->label_welcome->setText( tip);
}

void BootSplash::showScreen(QString loading){ //update icon, text, and progress
  QString txt, icon;
  int per = 0;
  if(loading=="init"){
    txt = tr("Initializing Session …"); per = 10;
    icon = "preferences-system-login";
  }else if(loading=="settings"){
    txt = tr("Loading System Settings …"); per = 20;	  
    icon = "user-home";
  }else if(loading=="user"){
    txt = tr("Loading User Preferences …"); per = 30; 
    icon = "preferences-desktop-user";
  }else if(loading=="systray"){
    txt = tr("Preparing System Tray …"); per = 40;
    icon = "preferences-plugin";
  }else if(loading=="wm"){
    txt = tr("Starting Window Manager …"); per = 50;
    icon = "preferences-system-windows-actions";	  
  }else if(loading=="apps"){
    txt = tr("Detecting Applications …"); per = 60;
    icon = "preferences-desktop-icons";
  }else if(loading=="menus"){
    txt = tr("Preparing Menus …"); per = 70;
    icon = "preferences-system-windows";
  }else if(loading=="desktop"){
    txt = tr("Preparing Workspace …"); per = 80;
    icon = "preferences-desktop-wallpaper";	
  }else if(loading=="final"){
    txt = tr("Finalizing …"); per = 90;
    icon = "start-here-lumina";	  
  }else if(loading.startsWith("app::")){
    txt = QString(tr("Starting App: %1")).arg(loading.section("::",1,50)); per = -1;
  }
  if(per>0){ ui->progressBar->setValue(per); }
  else{ ui->progressBar->setRange(0,0); } //loading indicator
  ui->label_text->setText(txt);
  if(!icon.isEmpty()){ui->label_icon->setPixmap( LXDG::findIcon(icon, "Lumina-DE").pixmap(64,64) ); }
  this->raise();
  this->show();
  this->update();
  QApplication::processEvents();
}
	
void BootSplash::showText(QString txt){ //will only update the text, not the icon/progress
  ui->label_text->setText(txt);
  this->show();
  this->update();
  QApplication::processEvents();
}
