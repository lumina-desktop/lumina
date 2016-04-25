#include "MessageCenter.h"

#include <LuminaXDG.h>
#include  <QVBoxLayout>
#include <QHBoxLayout>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

MessageCenterPlugin::MessageCenterPlugin(QWidget* parent, QString ID) : LDPlugin(parent, ID){
  //Setup the UI
  QVBoxLayout *vlay = new QVBoxLayout();
  this->setLayout( new QVBoxLayout() );
    this->layout()->setContentsMargins(0,0,0,0);
    vlay->setContentsMargins(3,3,3,3);
    frame = new QFrame(this);
      frame->setObjectName("messagecenterbase");
    this->layout()->addWidget(frame);
    frame->setLayout(vlay);
   
	
  //Setup the title bar header buttons
  QHBoxLayout *hlay = new QHBoxLayout();
  tool_clearall = new QToolButton(this);
    tool_clearall->setAutoRaise(true);
  tool_clearone = new QToolButton(this);
    tool_clearone->setAutoRaise(true);
  QWidget *spacer = new QWidget(this);
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    hlay->addWidget(spacer);
    hlay->addWidget(tool_clearone);
    hlay->addWidget(tool_clearall);
    vlay->addLayout(hlay);
	
  //Setup the main text widget
  list_messages = new QListWidget(this);
    list_messages->setSelectionMode(QAbstractItemView::SingleSelection);
    list_messages->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    vlay->addWidget(list_messages);
	
  //Now setup the initial values
  this->setInitialSize(200,300);
  //Setup the button connections
  connect(tool_clearall, SIGNAL(clicked()), this, SLOT(clearAllMessages()) );
  connect(tool_clearone, SIGNAL(clicked()), this, SLOT(clearSelectedMessage()) );
  
  //Setup the DBUS signals/slots
  if(QDBusConnection::sessionBus().isConnected()){
    if( QDBusConnection::sessionBus().registerService("org.freedesktop.Notifications") ){
      //Was able to register this service, also register everything it can do...
      //SUPPORTED: "body", "body-hyperlinks", "body-markup", "icon-static"
      
	    
    }
    QDBusConnection::sessionBus().connect("", "", "org.freedesktop.Notifications", "Notify", this, SLOT(newMessage(QString, uint, QString, QString, QString, QStringList, dict, int)) );
    QDBusConnection::sessionBus().interface().call("AddMatch", "interface='org.freedesktop.Notifications',member='Notify',type='method_call',eavesdrop='true'");
    qDebug() << "Available Session DBUS Services:" << QDBusConnection::sessionBus().interface()->registeredServiceNames().value();
    //connect(QString(), QString(), 
  }
  if(QDBusConnection::systemBus().isConnected()){
    qDebug() << "Available System DBUS Services:" << QDBusConnection::systemBus().interface()->registeredServiceNames().value();
  }
  
  QTimer::singleShot(0,this, SLOT(loadIcons()) );
}

MessageCenterPlugin::~MessageCenterPlugin(){

}

void MessageCenterPlugin::newMessage(QString summary, QString body){
  qDebug() << "New Message:" << summary, body;
}

void MessageCenterPlugin::clearAllMessages(){
 list_messages->clear();
}

void MessageCenterPlugin::clearSelectedMessage(){
  if( list_messages->currentItem()==0){ return; } //nothing selected
  list_messages->removeItemWidget( list_messages->currentItem() );
}


void MessageCenterPlugin::loadIcons(){
  tool_clearall->setIcon( LXDG::findIcon("edit-clear-list","") );
    tool_clearall->setToolTip( tr("Clear all messages") );
  tool_clearone->setIcon( LXDG::findIcon("edit-delete","") );
    tool_clearone->setToolTip( tr("Clear selected message") );

}
