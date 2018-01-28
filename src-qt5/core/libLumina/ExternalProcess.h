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
#include <QTimer>
#include <QApplication>
#include <QDebug>

class ExternalProcess : public QProcess{
	Q_OBJECT
private:
	bool cursorRestored;
	QString logoutput;

private slots:
	void resetCursor(){
	  //qDebug() << "External Process: Reset Mouse Cursor =" << !cursorRestored;
	  if(!cursorRestored){
	    QApplication::restoreOverrideCursor();
	    cursorRestored = true;
	  }
	}
	void processStarting(){
	  //qDebug() << "Starting External Process: Mouse Notification =" << !cursorRestored;
	  if(!cursorRestored){
	    QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
	    QTimer::singleShot(3000, this, SLOT(resetCursor()) );
	  }
	}
	void processFinished(){
	   //qDebug() << "External Process Finished: Reset Mouse Cursor =" << !cursorRestored;
	  if(!cursorRestored){
	    QApplication::restoreOverrideCursor();
	    cursorRestored = true;
	  }
	  //Clean up this object
          this->deleteLater();
	}
	void updateLog(){
	  logoutput.append( QString(this->readAllStandardOutput()) );
	}

public:
	ExternalProcess(QString logfile = "", bool manageCursors = false) : QProcess(){
	  this->setProcessChannelMode(QProcess::MergedChannels);
	  cursorRestored = !manageCursors;
	  if(logfile.isEmpty()){
	    this->setStandardOutputFile(QProcess::nullDevice());
	  }else if(logfile=="stdout"){
	    connect(this, SIGNAL(readyReadStandardOutput()), this, SLOT(updateLog()) );
	  }else{
	    this->setStandardOutputFile(logfile);
	  }
	  //Setup the connection for automatic cleanup
	  connect(this, SIGNAL(started()), this, SLOT(processStarting()) );
	  connect(this, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished()) );
	}

	~ExternalProcess(){
	  /*if(this->state() == QProcess::Running){
	    this->detach(); //about to close down the QProcess - detach the other program so it can continue functioning normally.
	  }*/
	}

	QString log(){
	  //NOTE: This will only contain output if the "stdout" argument is used as the logfile
	  return logoutput;
	}

	static void launch(QString program, QStringList args = QStringList(), bool manageCursors = true){
	  //Quick launch of a process with logging disabled and automatic cleanup
	  ExternalProcess *tmp = new ExternalProcess("", manageCursors);
	  if(args.isEmpty()){ tmp->start(program); }
	  else{ tmp->start(program, args); }
	}
};
#endif
