//===========================================
//  Copyright (c) 2017, q5sys (JT)
//  Available under the MIT license
//  See the LICENSE file for full details    
//===========================================

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDir"
#include "QFile"
#include "QTextStream"
#include "QImageReader"
#include "QFileDialog"
#include "QMessageBox"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->pushButton_executable, SIGNAL(clicked()), this, SLOT(setExec()) );
    connect(ui->pushButton_icon, SIGNAL(clicked()), this, SLOT(setIcon()) );
    connect(ui->pushButton_save, SIGNAL(clicked()), this, SLOT(save()) );
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(save()) );
    connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(close()) );

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setIcon(){
  //Prompt for a new image file
  QStringList imgformats;
  QList<QByteArray> fmts = QImageReader::supportedImageFormats();
  for(int i=0; i<fmts.length(); i++){
    imgformats << "*."+QString(fmts[i]);
  }
  QString iconpath = QFileDialog::getOpenFileName(this, tr("Select an image"), QDir::homePath(), \
                tr("Images")+" ("+imgformats.join(" ")+")");
  ui->lineEdit_icon->setText(iconpath);
  icon = ui->lineEdit_icon->text();
  }


void MainWindow::setExec(){
  //Prompt for a new executable file
  QString execpath = QFileDialog::getOpenFileName(this, tr("Select File"), QDir::homePath(), tr("All Files (*)") );
  ui->lineEdit_executable->setText(execpath);
  executable = ui->lineEdit_executable->text();
  }

void MainWindow::setCategories(){
if(ui->checkBox_audio->isChecked()){
catList = catList + "Audio;";}
if(ui->checkBox_video->isChecked()){
catList = catList + "Video;";}
if(ui->checkBox_development->isChecked()){
catList = catList + "Development;";}
if(ui->checkBox_education->isChecked()){
catList = catList + "Education;";}
if(ui->checkBox_game->isChecked()){
catList = catList + "Game;";}
if(ui->checkBox_graphics->isChecked()){
catList = catList + "Graphics;";}
if(ui->checkBox_network->isChecked()){
catList = catList + "Network;";}
if(ui->checkBox_office->isChecked()){
catList = catList + "Office;";}
if(ui->checkBox_science->isChecked()){
catList = catList + "Science;";}
if(ui->checkBox_settings->isChecked()){
catList = catList + "Settings;";}
if(ui->checkBox_system->isChecked()){
catList = catList + "System;";}
if(ui->checkBox_utility->isChecked()){
catList = catList + "Utility;";}
categories = catList;
}

void MainWindow::setOtherValues(){
name = ui->lineEdit_name->text();
genericname = ui->lineEdit_genericname->text();
keywords = ui->lineEdit_keywords->text();
comment = ui->lineEdit_comment->text();
if(ui->checkBox_terminal->isChecked()){
terminal = "true";} else{terminal = "false";};
}

void MainWindow::setDesktopFields(){
setCategories();
setOtherValues();
namefield = "Name=" + name;
genericnamefield = "GenericName=" + genericname;
commentfield = "Comment=" + comment;
iconfield = "Icon=" + icon;
terminalfield = "Terminal=" + terminal;
execfield = "Exec=" + executable;
categoriesfield = "Categories=" + categories;
keywordfield = "Keywords=" + keywords;
}

void MainWindow::save(){
setDesktopFields();
QString path = QDir::homePath();
QString filename;
filename = path + "/" + name + ".desktop";

QFile file(filename);
file.open(QIODevice::WriteOnly | QIODevice::Text);
QTextStream stream(&file);
stream << "{Desktop Entry]" << endl;
stream << "Type=Application" << endl;
stream << "Version=1.0" << endl;
stream << namefield << endl;
stream << genericnamefield << endl;
stream << commentfield << endl;
stream << iconfield << endl;
stream << terminalfield << endl;
stream << execfield << endl;
stream << categoriesfield << endl;
stream << keywordfield << endl;
if(file.isOpen()){
    QMessageBox *messageBox = new QMessageBox;
        messageBox->setText(tr("File Saved"));
        QPushButton *pushButtonOk = messageBox->addButton(tr("Ok"), QMessageBox::YesRole);
        messageBox->QDialog::setWindowTitle(tr("Successful"));
    messageBox->show();}
else{    QMessageBox *messageBox = new QMessageBox;
    messageBox->setText(tr("File Not Saved"));
    QPushButton *pushButtonOk = messageBox->addButton(tr("Ok"), QMessageBox::YesRole);
    messageBox->QDialog::setWindowTitle(tr("Unsuccessful"));
messageBox->show();}
file.close();
}
