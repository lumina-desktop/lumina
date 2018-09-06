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
#include <QX11Info>

#include <unistd.h> //for getuid()

LSingleApplication::LSingleApplication(int &argc, char **argv, QString appname) : QApplication(argc, argv){
  //Load the proper translation systems
  this->setAttribute(Qt::AA_UseHighDpiPixmaps);
  if(appname!="lumina-desktop"){ cTrans = LUtils::LoadTranslation(this, appname); }//save the translator for later
  //Initialize a couple convenience internal variables
  cfile = getLockfileName(this->applicationName()); //do not allow masking the utility name
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
  isActive = isBypass = false;
  lserver = 0;
  //Now check for the manual CLI flag to bypass single-instance forwarding (if necessary)
  if(inputlist.contains("-new-instance")){
    isBypass = true;
    inputlist.removeAll("-new-instance");
  }
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

QString LSingleApplication::getLockfileName(QString appname){
  QString path = QDir::tempPath()+"/.LSingleApp-%1-%2-%3";
  QString username = QString::number(getuid());
  QString display = QString(getenv("DISPLAY"));
  if(display.startsWith(":")){ display.remove(0,1); }
  display = display.section(".",0,0);
  path = path.arg( username, appname, display );
  return path;
}

void LSingleApplication::removeLocks(QString appname){
  QString path = getLockfileName(appname);
  if(QFile::exists(path+"-lock")){
    QFile::remove(path+"-lock");
  }
  if(QFile::exists(path)){
    QFile::remove(path);
  }
}

bool LSingleApplication::isPrimaryProcess(){
  return (isActive || isBypass);
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
      qDebug() << " - Cleaning stale single-instance lock:";
      if(lockfile->removeStaleLockFile() ){
        if(QFile::exists(cfile)){ QLocalServer::removeServer(cfile); } //also remove stale socket/server file
      }else{
        qDebug() << " -- Could not remove lock file";
      }
      //Now re-try to create the lock
      primary = lockfile->tryLock();
      //qDebug() << " - Try Lock Again:" << primary;
    }
  }
  if(primary || !QFile::exists(cfile) ){
    //Create the server socket
    //qDebug() << "Create Local Server";
    if(QFile::exists(cfile)){ QLocalServer::removeServer(cfile); } //stale socket/server file
    lserver = new QLocalServer(this);
      connect(lserver, SIGNAL(newConnection()), this, SLOT(newInputsAvailable()) );
     if( lserver->listen(cfile) ){
	qDebug() << " - Created new single-instance lock";
        lserver->setSocketOptions(QLocalServer::UserAccessOption);
	//qDebug() << " - Success";
	isActive = true;
     }else{
	qDebug() << " - WARNING: Could not create single-instance framework";
	qDebug() << "  - Falling back on standard application startup";
	lockfile->unlock();
	isActive = true;
     }

  }else if(!isBypass){
    //forward the current inputs to the locked process for processing and exit
    //Check the connection to the local server first
    qDebug() << "Single-instance lock found";
    QLocalSocket socket(this);
	socket.connectToServer(cfile);
	socket.waitForConnected();
	if(!socket.isValid() || socket.state()!=QLocalSocket::ConnectedState){
	  //error - could not forward info for some reason
	  qDebug() << " - Could not connect to locking process: exiting...";
		exit(1);
	}

    qDebug() << " - Forwarding inputs to locking process and closing down this instance...";
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
