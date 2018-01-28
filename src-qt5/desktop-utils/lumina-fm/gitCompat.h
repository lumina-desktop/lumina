//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the backend class for interacting with the "git" utility
//===========================================
#ifndef _LUMINA_FM_GIT_COMPAT_H
#define _LUMINA_FM_GIT_COMPAT_H

#include <QProcess>
#include <QString>
#include <QProcessEnvironment>
#include <QDebug>
#include <QTemporaryFile>
#include <LUtils.h>

#include <unistd.h>

class GitProcess : public QProcess{
	Q_OBJECT
private:
	QString log;
	QFile tmpfile;
public:
	GitProcess();
	~GitProcess();

	//Optional Inputs
	void setSSHPassword(QString pass);  //This is only used for SSH access

private slots:
	void cleanup();
	//void printoutput(){ qDebug() << "Proc Output:" << this->readAllStandardOutput(); }

protected:
	virtual void setupChildProcess(){
	  //Need to disable the controlling terminal within this process
	  setsid();  //Make current process new session leader - resulting in no controlling terminal for this session 
	}
};

class GIT{
public:
	//Check if the git utility is installed and available
	static bool isAvailable(){
	  QString bin = "git";
	  return LUtils::isValidBinary(bin);
	}

	//Return if the current directory is a git repository
	static bool isRepo(QString dir){
	  QProcess P;
	  P.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
	  P.setWorkingDirectory(dir);
	  P.start("git",QStringList() <<"status" << "--porcelain" );
	  P.waitForFinished();
          return (0==P.exitCode());
	}

	//Return the current status of the repository
	static QString status(QString dir){
	  QProcess P;
	  P.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
	  P.setWorkingDirectory(dir);
	  P.setProcessChannelMode(QProcess::MergedChannels);
	  P.start("git",QStringList() <<"status" );
	  P.waitForFinished();
          return P.readAllStandardOutput();	  
	}

	//Setup a process for running the clone operation (so the calling process can hook up any watchers and start it when ready)
	static GitProcess* setupClone(QString indir, QString url, QString branch = "", int depth = -1){
	  //NOTE: The returned QProcess needs to be cleaned up when finished
          GitProcess *P = new GitProcess();
	  P->setWorkingDirectory(indir);
	  P->setProgram("git");
          QStringList args;
	    args << "clone" << "--progress";
	    if(!branch.isEmpty()){ args << "-b" << branch; }
	    if(depth>0){ args << "--depth" << QString::number(depth); }
	    args << url;
	  P->setArguments(args);
 	  return P;
	}
};
#endif
