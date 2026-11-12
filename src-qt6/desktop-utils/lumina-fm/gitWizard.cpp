//===========================================
//  Lumina-DE source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the dialog for cloning a git repository
//===========================================
#include "gitWizard.h"
#include "ui_gitWizard.h"

#include "gitCompat.h"
#include <QDebug>
#include <QThread>

GitWizard::GitWizard(QWidget *parent) : QWizard(parent), ui(new Ui::GitWizard){
  ui->setupUi(this); //load the designer form
  proc = 0; //not initialized yet
  connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(pageChanged(int)) );
  //connect(this, SIGNAL(finished(int)), this, SLOT(finished(int)) );
  connect(ui->line_repo_org, SIGNAL(textChanged(const QString&)), this, SLOT(validateRepo()) );
  connect(ui->line_repo_name, SIGNAL(textChanged(const QString&)), this, SLOT(validateRepo()) );
  connect(ui->line_ssh_pass, SIGNAL(textChanged(const QString&)), this, SLOT(validateType()) );
  connect(ui->line_user, SIGNAL(textChanged(const QString&)), this, SLOT(validateType()) );
  connect(ui->line_pass, SIGNAL(textChanged(const QString&)), this, SLOT(validateType()) );
  connect(ui->radio_type_ssh, SIGNAL(clicked()), this, SLOT(validateType()) );
  connect(ui->radio_type_login, SIGNAL(clicked()), this, SLOT(validateType()) );
  connect(ui->radio_type_anon, SIGNAL(clicked()), this, SLOT(validateType()) );
  connect(ui->check_depth, SIGNAL(clicked()), this, SLOT(validateType()) );
  connect(ui->check_branch, SIGNAL(clicked()), this, SLOT(validateType()) );
  validateRepo();
}

GitWizard::~GitWizard(){
  if(proc!=0){ proc->deleteLater(); }
}

//Input values;
void GitWizard::setWorkingDir(QString path){
  inDir = path;
}

//============
//     PRIVATE
// ============
QString GitWizard::assembleURL(){
  QString repo = ui->line_repo_org->text()+"/"+ui->line_repo_name->text()+".git";
  QString url;
  if(ui->radio_type_ssh->isChecked()){ url = "git@github.com:"+repo; }
  else if(ui->radio_type_anon->isChecked()){ url = "https://github.com/"+repo; }
  else if(ui->radio_type_login->isChecked()){
    url = "https://"+ui->line_user->text()+":"+ui->line_pass->text()+"@github.com/"+repo; 
  }

  return url;
}

/*void GitWizard::showDownload(GitProcess *P){
  
  //P->closeWriteChannel();
  //P->closeReadChannel(GitProcess::StandardOutput);
  //P->closeReadChannel(GitProcess::StandardError);
  while(P->state()!=QProcess::NotRunning){
    this->thread()->usleep(50000); //50 ms
    QApplication::processEvents();
  }
  P->deleteLater();
}*/

//================
//   PRIVATE SLOTS
// ================
void GitWizard::pageChanged(int newpage){
   //called when the "next" button is clicked
  if(this->page(newpage)==ui->page_repo){
    validateRepo();
  }else if(this->page(newpage)==ui->page_type){
    //Need to adjust items on this page based on info on last page
    ui->radio_type_anon->setEnabled( !ui->check_privaterepo->isChecked() );
    ui->radio_type_ssh->setEnabled( QFile::exists(QDir::homePath()+"/.ssh/id_rsa") ); //TODO - Disable for now until SSH passphrases can be used properly
    //Now set the preferred type of login based on which are available
    if(ui->radio_type_ssh->isEnabled()){ ui->radio_type_ssh->setChecked(true); } //SSH is preferred if that is available
    else if(ui->radio_type_anon->isEnabled()){ ui->radio_type_anon->setChecked(true); } //anonymous next since it is a public repo - no creds really needed
    else{ ui->radio_type_login->setChecked(true); }
    //Clear any of the UI as needed
    ui->line_user->clear(); ui->line_pass->clear(); ui->line_ssh_pass->clear();
    validateType();
  }else if(this->page(newpage)==ui->page_download){
    qDebug() << "Run git clone";
    QString url  = assembleURL();
    QString branch; if(ui->check_branch->isChecked()){ branch = ui->line_branch->text(); }
    int depth = -1; if(ui->check_depth->isChecked()){ depth = ui->spin_depth->value(); }
    proc = GIT::setupClone(inDir, url, branch, depth);
    if(proc!=0){
     connect(proc, SIGNAL(readyReadStandardOutput()), this, SLOT(readProc()) );
     connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(procFinished(int)) );
      if(ui->radio_type_ssh->isChecked()){ 
        proc->setSSHPassword(ui->line_ssh_pass->text());
      }
      proc->start(QIODevice::ReadOnly);
      this->button(QWizard::FinishButton)->setEnabled(false);
    }
  }
}

//Page validation slots
void GitWizard::validateRepo(){
  bool ok = !ui->line_repo_org->text().isEmpty() && !ui->line_repo_name->text().isEmpty();
  this->button(QWizard::NextButton)->setEnabled(ok);
}

void GitWizard::validateType(){
  bool ok = false;
  //Check types first
  if(ui->radio_type_login->isChecked()){ ok = !ui->line_user->text().isEmpty() && !ui->line_pass->text().isEmpty(); }
  else{ ok = true; }
  //Now check optional settings
  if(ui->check_branch->isChecked()){ ok = ok && !ui->line_branch->text().isEmpty(); }
  if(ui->check_branch->isChecked()){ ok = ok && !ui->line_branch->text().isEmpty(); }
  //Now make interface boxes appear/disappear as needed
  ui->line_ssh_pass->setVisible(ui->radio_type_ssh->isChecked());
  ui->line_user->setVisible(ui->radio_type_login->isChecked());
  ui->line_pass->setVisible(ui->radio_type_login->isChecked());
  ui->spin_depth->setVisible(ui->check_depth->isChecked());
  ui->line_branch->setVisible(ui->check_branch->isChecked());

  //Update the button as needed 
  this->button(QWizard::NextButton)->setEnabled(ok);
}

void GitWizard::readProc(){
  ui->text_procOutput->append( proc->readAllStandardOutput() );
}

void GitWizard::procFinished(int retcode){
  this->button(QWizard::FinishButton)->setEnabled(true);
}

