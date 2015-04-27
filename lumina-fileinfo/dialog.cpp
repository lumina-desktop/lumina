#include "dialog.h"
#include "ui_dialog.h"
#include <QFileDialog>
#include <QRegExp>
#include <QTemporaryFile>
#include <QMessageBox>
#include "LuminaUtils.h"
#include <LuminaOS.h>



//this function is just like a regexp.
//we just change the required lines and we don't touch to the rest of the file and copy it back.
void Dialog::textReplace(QString &origin, QString from, QString to, QString topic)
{
  if (origin.contains(QRegExp("\n" + topic + "\\s*=\\s*" + from + "\n",Qt::CaseInsensitive))) {
    origin.replace(QRegExp("\n" + topic + "\\s*=\\s*" + from + "\n",Qt::CaseInsensitive),"\n" + topic + "=" + to + "\n");
  } else {
    origin.append(topic + "=" + to + "\n");
  }
}

//get the template from the user home directory or from the qrc files
void Dialog::copyTemplate(QString templateType)   
{
  if ((templateType == "-link") or (templateType == "-app")) {
    if (QFile::exists(QDir::homePath() + "/.lumina/LuminaDE/fileinfo" + templateType + ".template")) {
      //We take the template from homedir
      QFile::copy(QDir::homePath() + "/.lumina/LuminaDE/fileinfo" + templateType + ".template", desktopFileName);
    } else {
      //last possibility is to use the qrc template. 
      //But based on the initialisation, this should never occurs
      QFile::copy(":defaults/fileinfo" + templateType + ".template", desktopFileName);
    }
  } else {
    //error message for developpers
    qDebug() << "copyTemplate only accept '-link' or '-app' as parameter";
  }
}




Dialog::Dialog(QWidget *parent) :
      QDialog(parent),
      ui(new Ui::Dialog)
{
  ui->setupUi(this);
  desktopType="Application"; //default value

  //Setup all the icons using libLumina
  setupIcons();

  //we copy qrc templates in the home dir of the user. 
  //this allow the user to adapt those template to is own whishes
  QString templateFile = QDir::homePath() + "/.lumina/LuminaDE/fileinfo-link.template";
  if (!QFile::exists(templateFile)) {
    QFile::copy(":defaults/fileinfo-link.template", templateFile);
    QFile(templateFile).setPermissions(QFileDevice::ReadUser|QFileDevice::WriteUser);
  }
  templateFile = QDir::homePath() + "/.lumina/LuminaDE/fileinfo-app.template";
  if (!QFile::exists(templateFile)) {
    QFile::copy(":defaults/fileinfo-app.template", templateFile);
    QFile(templateFile).setPermissions(QFileDevice::ReadUser|QFileDevice::WriteUser);
  }
}


Dialog::~Dialog()
{
  delete ui;
}

void Dialog::setupIcons(){   
  this->setWindowIcon( LXDG::findIcon("unknown","") );
  ui->pbWorkingDir->setIcon( LXDG::findIcon("folder","") );
  ui->pbCommand->setIcon( LXDG::findIcon("system-search","") );
}

//Inform the user that required input parameters are missing
void Dialog::MissingInputs()
{
  qDebug() << "We cannot continue without a desktop file !!!";
  QMessageBox::critical(this, tr("Error"), tr("Lumina-fileinfo requires inputs:")+"\n"+QString(tr("Example: \"%1\"")).arg("lumina-fileinfo <-application>|<-link> desktopfile") );
  exit(1);
}


//Initialise the layout of the screen.
void Dialog::Initialise(QString param)
{
  //in case of "link", several objects are no required
  if (param.startsWith("-link")) {
    ui->cbRunInTerminal->setVisible(false);
    ui->cbStartupNotification->setVisible(false);
    ui->lCommand->setVisible(false);
    ui->pbCommand->setVisible(false);
    ui->lblCommand->setVisible(false);
    ui->lblOptions->setVisible(false);
    ui->lblWorkingDir->setText(tr("URL")); //we use the WorkingDir boxes for URL
    desktopType="link";
  }
  if (param.startsWith("-app")) {
    ui->cbRunInTerminal->setVisible(true);
    ui->cbStartupNotification->setVisible(true);
    ui->lCommand->setVisible(true);
    ui->pbCommand->setVisible(true);
    ui->lblCommand->setVisible(true);
    ui->lblOptions->setVisible(true);
    ui->lblWorkingDir->setText(tr("Working dir"));
    desktopType="app";
  }
}

//load the desktop file or the required template
void Dialog::LoadDesktopFile(QString input)
{
  //if we have "-" as 1st char, it means that this is not a desktop file, but a parameter
  desktopFileName = input;
  if (input.startsWith("-")) {
    QMessageBox::critical(this,tr("Error"),tr("The filename cannot start with a \"-\"."));
    exit(1);
  }
  //if proposed file does not exist, than we will create one based on the templates
  if (!QFile::exists(input)) {
    if (desktopType=="link") {
    copyTemplate("-link");
    } else {
      copyTemplate("-app");
    }
  }

  //use the standard LXDG object and load the desktop file
  bool ok = false;
  if(desktopFileName.endsWith(".desktop")){
    DF = LXDG::loadDesktopFile(desktopFileName, ok);
  }
  if( ok ) {
    if ((DF.type == XDGDesktop::LINK) && (desktopType!="link" )) {
      //we open a desktop type "link" but it was not mentionned by parameters
        Dialog::Initialise("-link");
    }
    ui->lName->setText(DF.name);
    ui->lComment->setText(DF.comment);
    ui->lCommand->setText(DF.exec);
    //in case of "link" desktop, we populate the correct content in lWorkingDir
    if (desktopType=="link") {
      ui->lWorkingDir->setText(DF.url);
    } else {
      ui->lWorkingDir->setText(DF.path);
    }
    if (DF.startupNotify) ui->cbStartupNotification->setChecked(true); else ui->cbStartupNotification->setChecked(false);
    if (DF.useTerminal) ui->cbRunInTerminal->setChecked(true); else ui->cbRunInTerminal->setChecked(false);
    iconFileName="";
    ui->pbIcon->setIcon(LXDG::findIcon(DF.icon,""));
    this->setWindowTitle(DF.filePath.section("/",-1));
  } else {
    QMessageBox::critical(this, tr("Error"), tr("Invalid File Supplied:") +"\n"+desktopFileName );
    exit(1);
  }

  //we load the file in memory and will adapt it before saving it to disk
  QFile file(desktopFileName);
  inMemoryFile="";
  if (file.open(QFile::ReadOnly)) {
    QTextStream fileData(&file);
    inMemoryFile = fileData.readAll();
    file.close();
    //perform some validation checks
    //this will allow checks improvements without compilation of the file
    if ((inMemoryFile.contains(QRegExp(".*\\[Desktop Entry\\].*\n"))) &&
      (inMemoryFile.contains(QRegExp("\n\\s*Type\\s*=.*\n"))) &&
      (inMemoryFile.contains(QRegExp("\n\\s*Name\\s*=.*\n")))) {
      //qDebug() << "sounds a good file"; 
    } else {
      //qDebug() << "wrong file!!!!";
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Question);
      msgBox.setText(tr("There are some issues with this file !!!!"));
      msgBox.setInformativeText(tr("Either you correct this file your self with an editor, or you start from scratch using the link or app template.\nPlease note that this process will update the file called:") + desktopFileName);
      QPushButton *linkButton = msgBox.addButton("Link",QMessageBox::AcceptRole);
      QPushButton *appButton = msgBox.addButton("App",QMessageBox::ResetRole);
      QPushButton *cancelButton = msgBox.addButton("Cancel",QMessageBox::NoRole);
      msgBox.exec();
      if (msgBox.clickedButton() == linkButton) {
        QFile::remove(desktopFileName);
        copyTemplate("-link");
        Initialise("-link");
        LoadDesktopFile(desktopFileName);
      }
      if (msgBox.clickedButton() == appButton) {
        QFile::remove(desktopFileName);
        copyTemplate("-app");
        Initialise("-app");
        LoadDesktopFile(desktopFileName);
      }
      if (msgBox.clickedButton() == cancelButton) {
        //we stop here
        exit(0);
      }
    }
  }
}


void Dialog::on_pbCommand_clicked()
{
  //the default directory is the user's home directory
  QString commandFolder = QDir::homePath();
  if (!ui->lCommand->text().isEmpty()) commandFolder = ui->lCommand->text().section('/', 0, -2);
  if (commandFolder.isEmpty()) commandFolder = QDir::homePath();

  QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open command"), commandFolder, tr("All Files (*)"));
  if (!fileName.isEmpty()) {
    ui->lCommand->setText(fileName);
    ui->lCommand->setModified(true);
  }
}


void Dialog::on_pbWorkingDir_clicked()
{
  //the default directory is /
  QString workingDir = "/";
  if (ui->lWorkingDir->text().isEmpty()) workingDir = "/";
  else workingDir = ui->lWorkingDir->text();
  QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
  QString directory = QFileDialog::getExistingDirectory(this,
                                    tr("Working Directory"),
                                    workingDir,
                                    options);
  if (!directory.isEmpty()) {
    ui->lWorkingDir->setText(directory);
    ui->lWorkingDir->setModified(true);
  }
}

//we save the changes to the destination file
void Dialog::on_pbApply_clicked()
{
  QString from,to;
  QString desktopTypeVal="Application";
  if (DF.type == XDGDesktop::APP) { desktopTypeVal="Application"; }
  else if (DF.type == XDGDesktop::LINK) { desktopTypeVal="Link"; }
  else if (DF.type == XDGDesktop::DIR) { desktopTypeVal="Dir"; }
  textReplace(inMemoryFile, desktopTypeVal, desktopType, "Type");

  if (ui->lName->isModified()) { textReplace(inMemoryFile, DF.name, ui->lName->text(), "Name");}
  if (ui->lComment->isModified()) { textReplace(inMemoryFile, DF.comment, ui->lComment->text(), "Comment");}
  if (ui->lCommand->isModified()) { textReplace(inMemoryFile, DF.exec, ui->lCommand->text(),"Exec");}
  if (desktopType=="link") {
    //incase of "link" layout WorkingDir is corresponding to the URL
    if (ui->lWorkingDir->isModified()) { textReplace(inMemoryFile, DF.url, ui->lWorkingDir->text(),"URL");}
  } else {
    if (ui->lWorkingDir->isModified()) { textReplace(inMemoryFile, DF.path, ui->lWorkingDir->text(),"Path");}
  }
  if (ui->cbStartupNotification->isChecked() != DF.startupNotify) {
    if (DF.startupNotify) {from="true"; to="false";} else {from="false"; to="true";}
    textReplace(inMemoryFile, from, to,"StartupNotify");
  }
  if (ui->cbRunInTerminal->isChecked() != DF.useTerminal) {
    if (DF.useTerminal) {from="true"; to="false";} else {from="false"; to="true";}
    textReplace(inMemoryFile, from, to,"Terminal");
  }
  if (!iconFileName.isEmpty()) {
    from=DF.icon;
    to=iconFileName;
    textReplace(inMemoryFile, from, to,"Icon");
  }

  QFile file(desktopFileName);
  if (file.open(QFile::ReadWrite)) {
    file.seek(0);
    file.write(inMemoryFile.toUtf8());

    file.resize(file.pos());//remove possible trailing lines

    file.close();
  } else {
    //problem to write to the disk
    QMessageBox::critical(this, tr("Problem to write to disk"), tr("We have a problem to write the adapted desktop file to the disk. Can you re-try the modification after solving the issue with the disk ?"));
  }

  //hack required to update the icon on the desktop
  QTemporaryFile tempFile ;
  tempFile.setAutoRemove(false);
  tempFile.setFileTemplate("/tmp/lumina-XXXXXX");
  tempFile.open();
  tempFile.close();
  
  QString cmd = "mv";
  cmd = cmd + " " + desktopFileName + " " + tempFile.fileName();
  int ret = LUtils::runCmd(cmd);
  if (ret !=0 ) {
    qDebug() << "Problem to execute:" << cmd;
    QMessageBox::critical(this, tr("Problem to write to disk"), tr("We have a problem to execute the following command:") + cmd);
  }

  cmd = "mv";
  cmd = cmd + " " + tempFile.fileName() + " " + desktopFileName;
  ret = LUtils::runCmd(cmd);
  if (ret !=0 ) {
    qDebug() << "Problem to execute:" << cmd;
    QMessageBox::critical(this, tr("Problem to write to disk"), tr("We have a problem to execute the following command:") + cmd);
  }
}


void Dialog::on_pbIcon_clicked()
{
  //the default directory is local/share/icons
  QString iconFolder = LOS::AppPrefix()+"/share/icons";
  if (!iconFileName.isEmpty()) iconFolder = iconFileName.section('/', 0, -2);
  else if (!DF.icon.isEmpty()) iconFolder = DF.icon.section('/', 0, -2);
  if (iconFolder.isEmpty()) iconFolder = LOS::AppPrefix()+"/share/icons";

  QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open command"), iconFolder, tr("Image Files (*.png *.jpg *.bmp)"));
  if (!fileName.isEmpty()) {
    ui->pbIcon->setIcon(QPixmap(fileName));
    iconFileName=fileName;
  }
}

void Dialog::on_lName_textChanged(QString text)
{
  if (text != DF.name && inMemoryFile.contains(QRegExp("\nName\\[\\S+\\]\\s*=",Qt::CaseInsensitive))) {
    QMessageBox msgBox;
    msgBox.setText(tr("By modifying this value, you will loose all translated versions"));
    msgBox.setInformativeText(tr("The field: Name is translated in several other languages. If you want to continue, you will loose all translated versions"));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    int answer = msgBox.exec();
    if (answer==QMessageBox::Ok) {
      //remove all translated versions. The lang cannot be null, but the value can be.
      inMemoryFile.replace(QRegExp("\nName\\[\\S+\\]\\s*=[^\n]*",Qt::CaseInsensitive), "");
    } else {
      ui->lName->setText(DF.name);
      }
  }
}

void Dialog::on_lComment_textChanged(QString text)
{
  if (text != DF.name && inMemoryFile.contains(QRegExp("\nComment\\[\\S+\\]\\s*=",Qt::CaseInsensitive))) {
    QMessageBox msgBox;
    msgBox.setText(tr("By modifying this value, you will loose all translated versions"));
    msgBox.setInformativeText(tr("The field: Comment is translated in several other languages. If you want to continue, you will loose all translated versions"));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    int answer = msgBox.exec();
    if (answer==QMessageBox::Ok) {
      //remove all translated versions. The lang cannot be null, but the value can be.
      inMemoryFile.replace(QRegExp("\nComment\\[\\S+\\]\\s*=[^\n]*",Qt::CaseInsensitive), "");
    } else {
      ui->lName->setText(DF.comment);
    }
  }
}

