/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QString>
#include <QProcess>
#include <QTextStream>
#include <QApplication>
#include <QDialogButtonBox>
#include <QPushButton>
#include <unistd.h>
#include <sys/types.h>
#include "mainwindow.h"

void MainWindow::ProgramInit()
{
  if( ! checkUserGroup() ) {
    QMessageBox::critical(this, tr("Access Denied"),
                                tr("This user does not have administrator permissions on this system!"),
                                QMessageBox::Ok,
                                QMessageBox::Ok);
    exit(1);
  }

  // Grab our arguments
  args = qApp->arguments();
  if(args.length()>1){ args.removeAt(0); } //remove the "qsudo" line
  QString commText = args.join(" ");

  // Check if we can bypass the GUI and use saved creds
  if ( checkSudoCache() )
     return;

  tries=3;
  connect(buttonBox, SIGNAL(clicked(QAbstractButton *)), this, SLOT(slotButtonClicked(QAbstractButton *)));
  connect(passwordLineEdit, SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()));
  connect(passwordLineEdit, SIGNAL(textChanged(QString)), this, SLOT(slotPwdTextChanged(QString)));
  connect(commandGroupBox,  SIGNAL(clicked(bool)), this, SLOT(slotExpandCommandClicked(bool)));

  QPushButton* btn= buttonBox->button(QDialogButtonBox::Ok);
  if (btn)
      btn->setEnabled(false);

  // Set command text
  commandLabel->setVisible(false);
  commandLabel->setText(commText);
  //Initialize the settings file for this user
  settings = new QSettings("TrueOS", "qsudo");
  //if(!settings->contains("qsudosamplevalue")){ settings->setValue("qsudosamplevalue","-1"); }
  //qDebug() << "Settings File:" << settings->fileName() << commText;
  //Check that there is not a dialog already open for this command
  if(settings->contains(commText)){
    //Also verify the PID is still active
    QString PID = settings->value(commText).toString();
    QStringList proclist = runQuickCmd("ps -p "+PID+" -j");
    //qDebug() << "PID Match:" << PID << proclist;
    if( proclist.length()>1 ){ //Make sure this PID is active
      if(proclist[1].contains(commText)){ //Make sure the PID is the same qsudo command
	qDebug() << "An identical process command is currently active: closing this one.";
        exit(0); //A QSudo process for this command is already running (stop this one)
      }
    }
  }
  //Save a process blocker
  //qDebug() << "Saving process blocker:" << commText << qApp->applicationPid();
  settings->setValue(commText, QString::number(qApp->applicationPid()) );
  settings->sync();
  //qDebug() << "Settings Status:" << settings->status();
  //Make sure to activate this window (in case the WM does not do it)
  this->setWindowState( this->windowState() | Qt::WindowActive );
}

void MainWindow::slotReturnPressed()
{
    if (passwordLineEdit->text().length())
        testPass();
}

void MainWindow::testPass()
{

  QString program = "sudo";
  QStringList arguments;
  arguments << "-S";
  arguments << "-k";
  arguments << "true";

  QProcess *tP = new QProcess(this);
  tP->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
  tP->setProcessChannelMode(QProcess::MergedChannels);
  tP->start(program, arguments);
  tP->write(passwordLineEdit->text().toLatin1() + "\n");
  tP->write(passwordLineEdit->text().toLatin1() + "\n");
  tP->write(passwordLineEdit->text().toLatin1() + "\n");
  while(tP->state() == QProcess::Starting || tP->state() == QProcess::Running ) {
     tP->waitForFinished(500);
     QCoreApplication::processEvents();
  }
  if ( tP->exitCode() != 0 )
  {
     QString tmp;
     tmp.setNum(tries-1);
     labelBadPW->setText(tr("Invalid Password! Tries Left: %1").arg(tmp) );
     tries--;
     if ( tries == 0 )
       exit(1);
     passwordLineEdit->setText("");
  } else {
    //Passes test
     settings->remove(commandLabel->text()); //Remove the process blocker
     settings->sync();
     startSudo();
  }
}

void MainWindow::startSudo()
{
  //qDebug() << "Start Sudo:";
  setVisible(false);
  QString program = "sudo";
  QStringList arguments;
  arguments << "-S";
  arguments << args; //saved input arguments
  //qDebug() << " - Arguments:" << arguments;
  /*for ( int i = 1; i< qApp->argc() ; i++)
    arguments << qApp->argv()[i];*/

  sudoProc = new QProcess(this);
  sudoProc->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
  sudoProc->start(program, arguments);
  sudoProc->write(passwordLineEdit->text().toLocal8Bit() + "\n");
  connect( sudoProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotProcDone() ) );
  connect( sudoProc, SIGNAL(readyReadStandardError()), this, SLOT(slotPrintStdErr() ) );
  connect( sudoProc, SIGNAL(readyReadStandardOutput()), this, SLOT(slotPrintStdOut() ) );
  while(sudoProc->state() == QProcess::Starting ) {
     sudoProc->waitForFinished(500);
     QCoreApplication::processEvents();
  }
}

void MainWindow::slotPrintStdErr()
{
  QTextStream cout(stderr);
  cout << sudoProc->readAllStandardError();
}

void MainWindow::slotPrintStdOut()
{
  QTextStream cout(stdout);
  cout << sudoProc->readAllStandardOutput();
}

void MainWindow::slotPwdTextChanged(const QString &text)
{
    QPushButton* btn= buttonBox->button(QDialogButtonBox::Ok);
    if (btn)
        btn->setEnabled(text.length()?true:false);
}

void MainWindow::slotExpandCommandClicked(bool isChecked)
{
    commandLabel->setVisible(isChecked);
}

void MainWindow::slotProcDone()
{
   exit(sudoProc->exitCode());
}

void MainWindow::slotButtonClicked(QAbstractButton *myBut)
{
  if ( buttonBox->buttonRole(myBut) == QDialogButtonBox::AcceptRole ) {
     testPass();
     close();
  }else{
    QApplication::exit(1);
  }

}

bool MainWindow::checkUserGroup()
{
   QString loginName = getlogin();
   QString groupName = "wheel"; // group to check
   QStringList gNames;
   if ( loginName == "root" )
     return true;

   QStringList info = runQuickCmd("getent group").filter(groupName); //need to support AD/LDAP settings
   /*QString tmp;
   QFile iFile("/etc/group");
   if ( ! iFile.open(QIODevice::ReadOnly | QIODevice::Text))
     return true; //or FALSE?*/

   for(int i=0; i<info.length(); i++){
     if(info[i].section(":",0,0)==groupName){
       gNames = info[i].section(":",3,3).split(",");
       break;
     }
   }
   /*while ( !iFile.atEnd() ) {
     tmp = iFile.readLine().simplified();
     if ( tmp.indexOf(groupName) == 0 ) {
        gNames = tmp.section(":", 3, 3).split(",");
        break;
     }
   }
   iFile.close();

   if ( gNames.isEmpty() )
      return false;*/

   for ( int i = 0; i < gNames.size(); ++i ){
      if ( gNames.at(i) == loginName ){
	return true;
      }
   }
   return false;
}

QStringList MainWindow::runQuickCmd(QString cmd){
   QProcess p;
   //Make sure we use the system environment to properly read system variables, etc.
   p.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
   //Merge the output channels to retrieve all output possible
   p.setProcessChannelMode(QProcess::MergedChannels);
   p.start(cmd);
   while(p.state()==QProcess::Starting || p.state() == QProcess::Running){
     p.waitForFinished(200);
     QCoreApplication::processEvents();
   }
   QString tmp = p.readAllStandardOutput();
   return tmp.split("\n", QString::SkipEmptyParts);
}

bool MainWindow::checkSudoCache(){
  int check = QProcess::execute("sudo", QStringList() << "-n" << "-v" );
  if ( check != 0 )
     return false;

  // We have a cached credential! Lets bypass the entire GUI
  setVisible(false);
  QString program = "sudo";
  QStringList arguments;
  arguments << args; //saved input arguments

  sudoProc = new QProcess(this);
  sudoProc->start(program, arguments);
  connect( sudoProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotProcDone() ) );
  connect( sudoProc, SIGNAL(readyReadStandardError()), this, SLOT(slotPrintStdErr() ) );
  connect( sudoProc, SIGNAL(readyReadStandardOutput()), this, SLOT(slotPrintStdOut() ) );
  while(sudoProc->state() == QProcess::Starting ) {
     sudoProc->waitForFinished(500);
     QCoreApplication::processEvents();
  }
  return true;
}
