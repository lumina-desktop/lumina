//===========================================
//  Lumina-desktop source code
//  Copyright (c) 2017, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// This is a simple class for launching/managing an external process in a non-interactive manner
//  This object will clean itself up when finished and log all output to a particular file if designated
//    otherwise it will suppress all output from the process
//===========================================
#ifndef _LUMINA_EXTERNAL_PROCESS_H
#define _LUMINA_EXTERNAL_PROCESS_H

#include <QProcess>
#include <QString>

class ExternalProcess : public QProcess{
	Q_OBJECT
private slots:
	void processFinished(){
	  //Clean up this object
          this->deleteLater();
	}
public:
	ExternalProcess(QString logfile = "") : QProcess(){
	  this->setProcessChannelMode(QProcess::MergedChannels);
	  if(logfile.isEmpty()){
	    this->setStandardOutputFile(QProcess::nullDevice());
	  }else{
	    this->setStandardOutputFile(logfile);
	  }
	  //Setup the connection for automatic cleanup
	  connect(this, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished()) );
	}
	~ExternalProcess(){
	}

	static void launch(QString program, QStringList args = QStringList()){
	  //Quick launch of a process with logging disabled and automatic cleanup
	  ExternalProcess *tmp = new ExternalProcess();
	  if(args.isEmpty()){ tmp->start(program); }
	  else{ tmp->start(program, args); }
	}
};
#endif
