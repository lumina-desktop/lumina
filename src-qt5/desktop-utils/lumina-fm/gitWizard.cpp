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

GitWizard::GitWizard(QWidget *parent) : QWizard(parent), ui(new Ui::GitWizard){
  ui->setupUi(this); //load the designer form
  connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(pageChanged(int)) );
  connect(this, SIGNAL(finished(int)), this, SLOT(finished(int)) );
}

GitWizard::~GitWizard(){

}

//Input values;
void GitWizard::setWorkingDir(QString path){
  inDir = path;
}

//============
//     PRIVATE
// ============
QString GitWizard::assembleURL(){

}

void GitWizard::showDownload(QProcess *P){

}

//================
//   PRIVATE SLOTS
// ================
void GitWizard::pageChanged(int newpage){
   //called when the "next" button is clicked
  if(this->page(newpage)==ui->page_repo){

  }else if(this->page(newpage)==ui->page_type){
    //Need to adjust items on this page based on info on last page
    ui->radio_type_anon->setEnabled( !ui->check_privaterepo->isChecked() );
    ui->radio_type_ssh->setEnabled( QFile::exists(QDir::homePath()+"/.ssh/id_rsa") );
    //Now set the preferred type of login based on which are available
    if(ui->radio_type_ssh->isEnabled()){ ui->radio_type_ssh->setChecked(true); } //SSH is preferred if that is available
    else if(ui->radio_type_anon->isEnabled()){ ui->radio_type_anon->setChecked(true); } //anonymous next since it is a public repo - no creds really needed
    else{ ui->radio_type_login->setChecked(true); }
    //Clear any of the UI as needed
    ui->line_user->clear(); ui->line_pass->clear(); ui->line_ssh_pass->clear();
     
  }else{
    //qDebug() << "Unknown page!" << newpage;
  }
}

void GitWizard::finished(int res){
   //called when the "finish" button is clicked:
  // res==0: window closed (rejected state)
  // res==1: "finish" clicked (accepted state)
  //qDebug() << "Got Finished:" << res;
  if(res == QDialog::Accepted){ 
    qDebug() << "Run git clone";
    QString url  = assembleUrl();
    QString branch; if(ui->check_brach->isChecked()){ branch = ui->line_branch->text(); }
    int depth = -1; if(ui->check_depth->isChecked()){ depth = ui->spin_depth->value(); }
    QProcess *proc = GIT::setupClone(inDir, url, branch, depth);
    if(proc!=0){
      showDownload(proc);
    }
  }
  
  this->deleteLater();
}
