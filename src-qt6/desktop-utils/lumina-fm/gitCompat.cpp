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
#include <QTemporaryFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>

GitProcess::GitProcess() : QProcess(){
  this->setProcessChannelMode(QProcess::MergedChannels);
  //(the askpass script file is created securely in setSSHPassword() - it used to be a predictable
  // "/tmp/.NNNN" name from an unseeded rand())
  tmpfile.setParent(this);
  connect(this, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(cleanup()) );
  connect(this, SIGNAL(readyReadStandardOutput()), this, SLOT(printoutput()) );
}

GitProcess::~GitProcess(){
  if(tmpfile.exists()){ tmpfile.remove(); } //ensure that password file never gets left behind
}

void GitProcess::setSSHPassword(QString pass){
  //Save the password into a small SSH_ASKPASS script.
  // - Created atomically with an unpredictable name and mode 0600 (QTemporaryFile), in the per-user runtime
  //   dir when available: the old file was created with default permissions and only chmod'ed afterwards.
  // - The password is emitted inside single quotes via printf, so the shell interprets nothing in it
  //   (it used to be inside echo "...", where $, ` and " were interpreted).
  QString dir = QString(getenv("XDG_RUNTIME_DIR"));
  if(dir.isEmpty() || !QFileInfo(dir).isWritable()){ dir = QDir::tempPath(); }
  QTemporaryFile script(dir+"/.lumina-askpass-XXXXXXXX");
  script.setAutoRemove(false); //removed by the script itself, by cleanup() and by the destructor
  if( !script.open() ){ return; }
  QString quoted = pass; quoted.replace("'", "'\\''"); //close quote, escaped quote, reopen
  QString path = script.fileName();
  QTextStream in(&script);
  in << "#!/bin/sh\n";
  in << "printf '%s\\n' '" << quoted << "'\n";
  in << "rm -f '" << path << "'\n"; //have the script clean itself up after running once
  in.flush();
  script.close();
  tmpfile.setFileName(path);
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
