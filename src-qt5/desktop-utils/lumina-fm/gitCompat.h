//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the backend classe for interacting with the "git" utility
//===========================================
#ifndef _LUMINA_FM_GIT_COMPAT_H
#define _LUMINA_FM_GIT_COMPAT_H

#include <QProcess>
#include <QString>
#include <QProcessEnvironment>

#include <LuminaUtils.h>

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
	static QProcess* setupClone(QString indir, QString url, QString branch = "", int depth = -1){
	  //NOTE: The returned QProcess needs to be cleaned up when finished
          QProcess *P = new QProcess();
	  P->setProcessEnvironment( QProcessEnvironment::systemEnvironment() );
	  P->setWorkingDirectory(indir);
	  P->setProgram("git");
          QStringList args;
	    args << "clone";
	    if(!branch.isEmpty()){ args << "-b" << branch; }
	    if(depth>0){ args << "--depth" << QString::number(depth); }
	    args << url;
	  P->setArguments(args);
 	  return P;
	}
};
#endif
