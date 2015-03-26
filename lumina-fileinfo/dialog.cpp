#include "dialog.h"
#include "ui_dialog.h"
#include <QFileDialog>
#include <QRegExp>
#include <QTemporaryFile>
#include <QMessageBox>
#include "LuminaUtils.h"
#include <LuminaOS.h>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    desktopType="Application"; //default value
    
    //Setup all the icons using libLumina
    this->setWindowIcon( LXDG::findIcon("preferences-desktop-default-applications","") );
    ui->pbWorkingDir->setIcon( LXDG::findIcon("folder","") );
    ui->pbCommand->setIcon( LXDG::findIcon("system-search","") );
}


Dialog::~Dialog()
{
    delete ui;
}

//Inform the user that required input parameters are missing
void Dialog::MissingInputs()
{
    qDebug() << "We cannot continue without a desktop file !!!";
    QMessageBox::critical(this, tr("Error"), tr("The application requires inputs: <-application>|<-link> desktopfile") );
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
        ui->lblWorkingDir->setText("URL"); //we use the WorkingDir boxes for URL
        desktopType="link";
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
    //TODO: have a config directory to store templates
    if (!QFile::exists(input)) {
        if (desktopType=="link") { QFile::copy("./fileinfo-link.template", desktopFileName);}
        else { QFile::copy("./fileinfo-app.template", desktopFileName);}
    }

    //use the standard LXDG object and load the desktop file
    bool ok = false;
    DF = LXDG::loadDesktopFile(desktopFileName, ok);
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
        ui->pbIcon->setIcon(QPixmap(DF.icon));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Problem to read the desktop file called:") + desktopFileName );
        exit(1);
    }
}


void Dialog::on_pbCommand_clicked()
{
	//the default directory is the user's home directory
 	QString commandFolder = "~"; 
    if (!ui->lCommand->text().isEmpty()) commandFolder = ui->lCommand->text().section('/', 0, -2); 
    if (commandFolder.isEmpty()) commandFolder = "~";
   
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

//this function is just like a regexp.
//we just change the required lines and we don't touch to the rest of the file and copy it back.
void Dialog::textReplace(QString &origin, QString from, QString to, QString topic)
{
    if (origin.contains(QRegExp("\n" + topic + "\\[\\S+\\]\\s*=",Qt::CaseInsensitive))) {
		QMessageBox msgBox;
		msgBox.setText(tr("By modifying this value, you will loose all translated versions"));
		msgBox.setInformativeText(tr("The field:") + topic + tr( "is translated in several other languages. If you want to continue, you will loose all translated versions"));
		msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
		int answer = msgBox.exec();
		if (answer==QMessageBox::Ok) {
			//remove all translated versions. The lang cannot be null, but the value can be.
			origin.replace(QRegExp("\n" + topic + "\\[\\S+\\]\\s*=[^\n]*",Qt::CaseInsensitive), "");
		}
		else return;
	}
    if (!from.isEmpty()) {
        origin.replace(QRegExp("\n" + topic + "\\s*=\\s*" + from + "\n",Qt::CaseInsensitive),"\n" + topic + "=" + to + "\n");
    } else {
        //TODO: check if last char in \n. If not add it
        origin.append(topic + "=" + to + "\n");
    }
}

//we save the changes to the destination file
void Dialog::on_pbApply_clicked()
{
    
    QByteArray fileData;
    QFile file(desktopFileName);
    if (file.open(QFile::ReadWrite)) {
        QString from,to;
        fileData = file.readAll();
        QString text(fileData);

        QString desktopTypeVal="Application";
        if (DF.type == XDGDesktop::APP) { desktopTypeVal="Application"; }
        else if (DF.type == XDGDesktop::LINK) { desktopTypeVal="Link"; }
        else if (DF.type == XDGDesktop::DIR) { desktopTypeVal="Dir"; }
        textReplace(text, desktopTypeVal, desktopType, "Type");

        if (ui->lName->isModified()) { textReplace(text, DF.name, ui->lName->text(), "Name");}
        if (ui->lComment->isModified()) { textReplace(text, DF.comment, ui->lComment->text(), "Comment");}
        if (ui->lCommand->isModified()) { textReplace(text, DF.exec, ui->lCommand->text(),"Exec");}
        if (desktopType=="link") {
            //incase of "link" layout WorkingDir is corresponding to the URL
            if (ui->lWorkingDir->isModified()) { textReplace(text, DF.url, ui->lWorkingDir->text(),"URL");}
        } else {
            if (ui->lWorkingDir->isModified()) { textReplace(text, DF.path, ui->lWorkingDir->text(),"Path");}
        }
        if (ui->cbStartupNotification->isChecked() != DF.startupNotify) {
            if (DF.startupNotify) {from="true"; to="false";} else {from="false"; to="true";}
            textReplace(text, from, to,"StartupNotify");
        }
        if (ui->cbRunInTerminal->isChecked() != DF.useTerminal) {
            if (DF.useTerminal) {from="true"; to="false";} else {from="false"; to="true";}
            textReplace(text, from, to,"Terminal");
        }
        if (!iconFileName.isEmpty()) {
            from=DF.icon;
            to=iconFileName;
            textReplace(text, from, to,"Icon");
        }

        file.seek(0);
        file.write(text.toUtf8());

        file.resize(file.pos());//remove possible trailing lines

        file.close();

        //hack required to update the icon on the desktop
            QTemporaryFile tempFile ;
            tempFile.setAutoRemove(false);
            tempFile.open();
            tempFile.close();

            //TODO: capture errors
            QString cmd = "mv";
            cmd = cmd + " " + desktopFileName + " " + tempFile.fileName();
            int ret = LUtils::runCmd(cmd);

            cmd = "mv";
            cmd = cmd + " " + tempFile.fileName() + " " + desktopFileName;
            ret = LUtils::runCmd(cmd);
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
