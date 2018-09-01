//===========================================
//  Lumina-Desktop source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "imgDialog.h"
#include "ui_imgDialog.h"

#include <QMessageBox>

#include <LuminaOS.h>
#include <LuminaXDG.h>

#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

imgDialog::imgDialog(QWidget *parent) : QDialog(parent), ui(new Ui::imgDialog()){
  ui->setupUi(this); //load the designer form
  QString title = tr("Burn Disk Image to Device");
  if( getuid()==0){ title.append(" ("+tr("Admin Mode")+")"); }
  this->setWindowTitle(title);
  ui->frame_running->setVisible(false);
  ui->frame_setup->setVisible(true);
  ddProc = 0;
  unitdiv = 1;
  //Setup the signals/slots
  ui->tool_refresh->setIcon( LXDG::findIcon("view-refresh","drive-removable-media-usb-pendrive") );
  connect(ui->push_cancel, SIGNAL(clicked()), this, SLOT(cancel()) );
  connect(ui->push_start, SIGNAL(clicked()), this, SLOT(start_process()) );
  connect(ui->tool_refresh, SIGNAL(clicked()), this, SLOT(loadDeviceList()) );
  loadDeviceList(); //do the initial load of the available devices
  //Setup the possible transfer rate units
  ui->combo_rate_units->clear();
  ui->combo_rate_units->addItem(tr("Kilobyte(s)"), "k");
  ui->combo_rate_units->addItem(tr("Megabyte(s)"), "m");
  ui->combo_rate_units->addItem(tr("Gigabyte(s)"), "g");
  ui->combo_rate_units->setCurrentIndex(1); //MB
  //Setup the Process Timer
  procTimer = new QTimer(this);
    procTimer->setInterval(1000); //1 second updates
  connect(procTimer, SIGNAL(timeout()), this, SLOT(getProcStatus()) );
  //Determine which type of system this is for the process status signal
  BSD_signal = LOS::OSName().contains("BSD"); //assume everything else is Linux-like
}

imgDialog::~imgDialog(){

}

void imgDialog::loadIMG(QString filepath){
  ui->label_iso->setText(filepath.section("/",-1)); //only show the filename
  ui->label_iso->setWhatsThis(filepath); //save the full path for later
}

//============================
//  PRIVATE SLOTS
//============================
void imgDialog::start_process(){
  //Sanity Checks
  if( !QFile::exists(ui->combo_devices->currentData().toString()) ){ loadDeviceList(); return; } //USB device no longer available
  if(!QFile::exists(ui->label_iso->whatsThis()) ){ return; } //IMG file no longer available
  qDebug() << "Start Process...";
  //Read the size of the img file
  QString units = ui->combo_rate_units->currentData().toString();
  if(units=="k"){ unitdiv = 1024; }
  else if(units=="m"){ unitdiv = 1024*1024; }
  else if(units=="g"){ unitdiv = 1024*1024*1024; }
  qint64 bytes = QFileInfo(ui->label_iso->whatsThis()).size();
  //qDebug() << "IMG File size:" << bytes;
  //Set the progressBar maximum
  ui->progressBar->setRange(0, qRound(bytes/unitdiv) );
  ui->progressBar->setValue(0);
  ui->label_dev->setText( ui->combo_devices->currentText() );
  ui->label_time->setText("0:00");
  ui->frame_running->setVisible(true);
  ui->frame_setup->setVisible(false);
  //qDebug() << "Blocks:" << ui->progressBar->maximum();
  //Initialize the process
  if(ddProc==0){
    ddProc = new QProcess(this);
    connect(ddProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(procFinished()) );
    connect(ddProc, SIGNAL(readyRead()), this, SLOT(procInfoAvailable()) );
    ddProc->setProcessChannelMode(QProcess::MergedChannels);
  }
  //Generate the command
  QString prog; QStringList args;
  //if( ::getuid()!=0){ prog = "qsudo";  args<<"dd"; }
  //else{ 
    prog = "dd"; 
  //}
   args << "if="+ui->label_iso->whatsThis();
   args << "of="+ui->combo_devices->currentData().toString();
   args << "bs="+QString::number(ui->spin_rate_num->value())+units;
   if(ui->check_sync->isChecked()){ args << "conv=sync"; }
  //Start the process
  startTime = QDateTime::currentDateTime();
  ddProc->start(prog, args);
  //Start the timer to watch for updates
  procTimer->start();
  ui->push_start->setEnabled(false);
}

void imgDialog::cancel(){
  if(ddProc==0 || ddProc->state()==QProcess::NotRunning){
    this->close();
  }else{
    //Prompt if the transfer should be cancelled
    if(QMessageBox::Yes == QMessageBox::question(this, tr("Cancel Image Burn?"), tr("Do you wish to stop the current disk image burn process?")+"\n\n"+tr("Warning: This will leave the USB device in an inconsistent state")) ){
      ddProc->kill();
    }
  }
}

void imgDialog::loadDeviceList(){
  ui->combo_devices->clear();
  //Probe the system for USB devices
  QDir devDir("/dev");
  QString filter = (BSD_signal) ? "da*" : "sd*";
  QStringList usb = devDir.entryList(QStringList() << filter, QDir::System, QDir::Name);
  //Filter out any devices which are currently mounted/used

  //Create the list
  for(int i=0; i<usb.length(); i++){
    ui->combo_devices->addItem(usb[i], devDir.absoluteFilePath(usb[i]));
  }
}

void imgDialog::getProcStatus(){
  if(ddProc==0 || ddProc->state()!=QProcess::Running ){ return; }
  QStringList pidlist = LUtils::getCmdOutput("pgrep -S -f \"dd if="+ui->label_iso->whatsThis()+"\"");
  if(pidlist.isEmpty()){ return; }
  int pid = pidlist.first().simplified().toInt(); //just use the first pid - the pgrep should be detailed enough to only match one
  //qDebug() << "Sending signal to show status on PID:" << pid;
#ifndef __linux__
  ::kill(pid, SIGINFO); //On BSD systems, the INFO signal is used to poke dd for status updates
#else
  //LINUX systems do not even have a SIGINFO defined - need to block this off with defines...
  ::kill(pid, SIGUSR1);  //On linux systems, the USR1 signal is used to poke dd for status updates
#endif
  //Now update the elapsed time on the UI
    int elapsed = startTime.secsTo( QDateTime::currentDateTime() );
    int min = elapsed/60;
    int secs = elapsed%60;
    ui->label_time->setText( QString::number(min)+":"+ (secs < 10 ? "0" : "")+QString::number(secs) );
}

void imgDialog::procInfoAvailable(){
  lastmsg = ddProc->readAll();
  if(lastmsg.endsWith("\n")){ lastmsg.chop(1); }
  //qDebug() << "Got Process Info:" << lastmsg;
  //Now look for the " bytes transferred" line
  QStringList records = lastmsg.split("\n").filter(" bytes transferred ");
  if(!records.isEmpty()){
    //Update the progress bar
    //qDebug() << "Got status update:" << records.last();
    ui->progressBar->setValue( qRound(records.last().section(" bytes",0,0).toDouble()/unitdiv) );
  }
}

void imgDialog::procFinished(){
  qDebug() << "Process Finished:" << ddProc->exitCode();
  procTimer->stop();
  ui->frame_running->setVisible(false);
  ui->frame_setup->setVisible(true);
  if(ddProc->exitStatus()==QProcess::NormalExit){
    if(ddProc->exitCode() !=0 ){
      if(lastmsg.contains("permission denied", Qt::CaseInsensitive) && LUtils::isValidBinary("qsudo") ){
        if(QMessageBox::Yes == QMessageBox::question(this, tr("Administrator Permissions Needed"), tr("This operation requires administrator priviledges.")+"\n\n"+tr("Would you like to enable these priviledges?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) ){
          QProcess::startDetached("qsudo", QStringList() << "lumina-archiver" << "--burn-img" << ui->label_iso->whatsThis());
          exit(0);
        }
      }else{
        QMessageBox::warning(this, tr("ERROR"), tr("The process could not be completed:")+"\n\n"+lastmsg);
      }
    }else{
      QMessageBox::information(this, tr("SUCCESS"), tr("The image was successfully burned to the device") );
      this->close();
    }
  }
}
