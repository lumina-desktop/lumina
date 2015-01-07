//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "LuminaSingleApplication.h"
#include <QDir>
#include <QFile>
#include <QLocalSocket>
#include <QDebug>

#include <unistd.h> //for getlogin()

LSingleApplication::LSingleApplication(int &argc, char **argv, QString appname) : QApplication(argc, argv){
  //Load the proper translation systems
  LUtils::LoadTranslation(this, appname);
  //Initialize a couple convenience internal variables
  cfile = QDir::tempPath()+"/.LSingleApp-%1-%2";
  QString username = QString(getlogin());
  //For locking the process use the official process name - not the user input (no masking)
  appname = this->applicationName();
  //Obscure the user/app in the filename (TO DO)
  //qDebug() << username << appname;
  //bool junk;
  //qDebug() << QString::number( username.toInt(&junk,16) );
  cfile = cfile.arg( username, appname );
  lockfile = new QLockFile(cfile+"-lock");
    lockfile->setStaleLockTime(0); //long-lived processes
  for(int i=1; i<argc; i++){ 
    QString path = QString::fromLocal8Bit(argv[i]);
    //do few quick conversions for relative paths and such as necessary
    // (Remember: this is only used for secondary processes, not the primary)
      if(path=="."){
	//Insert the current working directory instead
	path = QDir::currentPath();
      }else{
	if(!path.startsWith("/") && !path.startsWith("-") ){ path.prepend(QDir::currentPath()+"/"); }
      }
    inputlist << path; 
  }
  isActive = false;
  lserver = 0;
  PerformLockChecks();
}

LSingleApplication::~LSingleApplication(){
  if(lserver != 0 && lockfile->isLocked() ){ 
    //currently locked instance: remove the lock now
    lserver->close();
    QLocalServer::removeServer(cfile);
    lockfile->unlock(); 
  }
}

bool LSingleApplication::isPrimaryProcess(){
  return isActive;	
}

void LSingleApplication::PerformLockChecks(){
  bool primary = lockfile->tryLock();
  //qDebug() << "Try Lock: " << primary;
  if(!primary){
    //Pre-existing lock - check it for validity
    QString appname, hostname;
    qint64 pid;
    lockfile->getLockInfo(&pid, &hostname, &appname); //PID already exists if it gets this far, ignore hostname
    //qDebug() << " - Lock Info:" << pid << hostname << appname;
    if( appname!=this->applicationName() || !QFile::exists(cfile) ){
      //Some other process has the same PID or the server does not exist - stale lock
      //qDebug() << " - Stale Lock";
      lockfile->removeStaleLockFile();
      //Now re-try to create the lock
      primary = lockfile->tryLock();
      //qDebug() << " - Try Lock Again:" << primary;
    }
  }
  if(primary){
    //Create the server socket
    //qDebug() << "Create Local Server";
    if(QFile::exists(cfile)){ QLocalServer::removeServer(cfile); } //stale socket/server file
    lserver = new QLocalServer(this);
      connect(lserver, SIGNAL(newConnection()), this, SLOT(newInputsAvailable()) );
     if( lserver->listen(cfile) ){
        lserver->setSocketOptions(QLocalServer::UserAccessOption);
	//qDebug() << " - Success";
	isActive = true;
     }else{
	//qDebug() << " - Failure:" << lserver->errorString();
	lockfile->unlock();
     }
      
  }else{
    //forward the current inputs to the locked process for processing and exit
    //qDebug() << "Forward inputs to locking process:" << inputlist;
    QLocalSocket socket(this);
	socket.connectToServer(cfile);
	socket.waitForConnected();
	if(!socket.isValid()){ exit(1); } //error - could not forward info
	socket.write( inputlist.join("::::").toLocal8Bit() );
	socket.waitForDisconnected(500); //max out at 1/2 second (only hits this if no inputs)
  }
  
}

//New messages detected
void LSingleApplication::newInputsAvailable(){
  while(lserver->hasPendingConnections()){
    QLocalSocket *sock = lserver->nextPendingConnection();
    QByteArray bytes;
    sock->waitForReadyRead();
    while(sock->bytesAvailable() > 0){ //if(sock->waitForReadyRead()){
	//qDebug() << "Info Available";
	bytes.append( sock->readAll() );
    }
    sock->disconnectFromServer();
    QStringList inputs = QString::fromLocal8Bit(bytes).split("::::");
    //qDebug() << " - New Inputs Detected:" << inputs;
    emit InputsAvailable(inputs);
  }
}