// QT Includes
#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QTimer>
#include <QDebug>

#include <LuminaXDG.h>

class TrayApp : public QSystemTrayIcon {
   Q_OBJECT

private:
  QTimer *timer;
  int iconnum;

private slots:
   void ChangeIcon(){
    this->setToolTip("Icon Number:"+QString::number(iconnum));
    QString ico;
    //Rotate the icon every time
    qDebug() << "Changing Icon:" << iconnum;
    if(iconnum <=0){ ico = "arrow-left"; iconnum=1; }
    else if(iconnum==1){ ico = "arrow-up"; iconnum=2; }
    else if(iconnum==2){ ico = "arrow-right"; iconnum=3; }
    else{ico = "arrow-down"; iconnum=0; }
    this->setIcon( LXDG::findIcon(ico,"") );
    this->showMessage("Title","Some random message", QSystemTrayIcon::Information, 1500); //1.5 second popup
   }

   void StopTest(){
      QApplication::exit(0);
   }

public:
   TrayApp() : QSystemTrayIcon(){
      iconnum = 0;
      this->setContextMenu(new QMenu());
      this->contextMenu()->addAction("Stop Test", this, SLOT(StopTest()) );
      timer = new QTimer(this);
        timer->setInterval(3000); //change every 3 seconds
        connect(timer, SIGNAL(timeout()), this, SLOT(ChangeIcon()) );
      ChangeIcon(); //get it updated now
      timer->start();
   }
   virtual ~TrayApp(){}

};
