//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include <QObject>
#include <QProcess>
#include <QDebug>
#include <QFileSystemWatcher>
#include <QTimer>

#include <sys/types.h>
#include <signal.h>

class LProcess : public QProcess{
	Q_OBJECT
private:
	QFileSystemWatcher *watcher;
	QString id;
private slots:
	void filechanged(QString path){
          //qDebug() << "File Changed:" << path;
	  //qDebug() << " - Program:" << this->program();
	  if(watcher==0){ return; } //just in case
	  if(this->state()==QProcess::Running){
	    if(this->program().section(" ",0,0).section("/",-1) == "fluxbox" ){
             // qDebug() << "Sending Fluxbox signal to reload configs...";
              ::kill(this->pid(), SIGUSR2); } //Fluxbox needs SIGUSR2 to reload it's configs
	    else if(this->program().section(" ",0,0).section("/",-1) == "compton" ){
              //qDebug() << "Sending Compton signal to reload configs...";
              ::kill(this->pid(), SIGUSR1); } //Compton needs SIGUSR1 to reload it's configs
	  }
	  //Now make sure this file/dir was not removed from the watcher
	  QStringList watched; watched << watcher->files() << watcher->directories();
	  if(!watched.contains(path)){ watcher->addPath(path); } //re-add it
	}

public:
	LProcess(QString ID, QStringList watchfiles) : QProcess(){
	  id=ID;
	  watcher = 0;
          if(!watchfiles.isEmpty()){
            //qDebug() << "Watch Files for changes:" << ID << watchfiles;
	    watcher = new QFileSystemWatcher(this);
	    connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(filechanged(QString)) );
	    connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(filechanged(QString)) );
	    watcher->addPaths(watchfiles);
	  }
	}
	~LProcess(){

	}
	QString ID(){ return id; }

};

class LSession : public QObject{
	Q_OBJECT
private:
	QList<QProcess*> PROCS;
	bool stopping;
	int wmfails;
	QTimer *wmTimer;

	void setupCompositor(bool force = false);

private slots:
	void stopall();

	void procFinished();

	void startProcess(QString ID, QString command, QStringList watchfiles = QStringList());

	void resetWMCounter(){ wmfails = 0; }
public:
	LSession(){
	stopping = false; wmfails = 0;
	  wmTimer = new QTimer(this);
	  wmTimer->setSingleShot(true);
	  wmTimer->setInterval(2000); //2 second timeout
           connect(wmTimer, SIGNAL(timeout()), this, SLOT(resetWMCounter()) );
	}
	~LSession(){ }

	void start(bool unified = false);

	void checkFiles();

};
