//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the backend classe for interacting with the "git" utility
//===========================================
#include "gitCompat.h"
#include <QApplication>

#define TMPFILE QString("/tmp/.")
GitProcess::GitProcess() : QProcess(){
  this->setProcessChannelMode(QProcess::MergedChannels);
  tmpfile.setFileName(TMPFILE +QString::number( (rand()%8999) + 1000 ));
  //qDebug() << "Temporary File Name:" << tmpfile.fileName();
  tmpfile.setParent(this);
  connect(this, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(cleanup()) );
  connect(this, SIGNAL(readyReadStandardOutput()), this, SLOT(printoutput()) );
}

GitProcess::~GitProcess(){
  if(tmpfile.exists()){ tmpfile.remove(); } //ensure that password file never gets left behind
}

void GitProcess::setSSHPassword(QString pass){
  //Save the password into the temporary file
  if( tmpfile.open(QIODevice::WriteOnly) ){
    QTextStream in(&tmpfile);
    in << "#!/bin/sh"<<"\n";
    in << "echo \""+pass+"\"" << "\n";
    in << "rm "+tmpfile.fileName()+"\n"; //have the script clean itself up after running once
    tmpfile.close();
  }
  tmpfile.setPermissions( QFile::ReadOwner  | QFile::WriteOwner | QFile::ExeOwner );
  QApplication::processEvents();
  QProcessEnvironment env = this->processEnvironment();
    env.insert("SSH_ASKPASS", tmpfile.fileName());
    env.insert("DISPLAY",":0"); //will not actually be used - the tmp file sends the password back instantly
  this->setProcessEnvironment(env);
}

void GitProcess::cleanup(){
  if(tmpfile.exists()){ tmpfile.remove(); } //ensure that password file never gets left behind
}
