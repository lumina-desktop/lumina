//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "MainUI.h"
#include "ui_MainUI.h"

#include <LuminaOS.h>
#include <LuminaUtils.h>
#include <LuminaXDG.h>

#include <QPixmap>
#include <QMessageBox>
#include <QProcess>
#include <QDebug>

MainUI::MainUI() : QMainWindow(), ui(new Ui::MainUI){
  ui->setupUi(this);
  //qDebug() << "Update UI";
  updateUI();
  //qDebug() << "Load Tab Widget";
  ui->tabWidget->setCurrentWidget(ui->tab); //start on the "General" tab
}

MainUI::~MainUI(){

}

void MainUI::updateUI(){
  //Load the UI from all the resources / library info
  this->setWindowIcon( LXDG::findIcon("lumina","") );
  ui->label_icon->setPixmap( QPixmap(":Images/Lumina-logo.png").scaledToHeight(ui->label_icon->height(), Qt::SmoothTransformation) );
  connect(ui->push_close, SIGNAL(clicked()), this, SLOT(close()) );
  //General Tab
  ui->label_version->setText( LUtils::LuminaDesktopVersion() );
  ui->label_OS->setText( LOS::OSName() );
  connect(ui->tool_aboutQt, SIGNAL(clicked()), this, SLOT(showQtInfo()) );
  connect(ui->label_doc, SIGNAL(linkActivated(const QString&)), this, SLOT(LinkClicked(QString)) );
  connect(ui->label_source, SIGNAL(linkActivated(const QString&)), this, SLOT(LinkClicked(QString)) );
  //License Tab
  ui->text_license->setPlainText( LUtils::readFile(":LICENSE").join("\n") );
  //Acknowledgements Tab
  //  Note: contents set within the designer form itself
  connect(ui->list_sponsors, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(showSponsor(QListWidgetItem*)) );
  connect(ui->label_proj_lead, SIGNAL(linkActivated(const QString&)), this, SLOT(LinkClicked(QString)) );
  connect(ui->label_proj_contrib, SIGNAL(linkActivated(const QString&)), this, SLOT(LinkClicked(QString)) );
	
}

void MainUI::showQtInfo(){
  QMessageBox::aboutQt(this);
}

void MainUI::showSponsor(QListWidgetItem *item){
  if(item->whatsThis().isEmpty()){ return; }
  QProcess::startDetached("lumina-open \""+item->whatsThis()+"\"");
}

void MainUI::LinkClicked(QString url){
  QProcess::startDetached("lumina-open \""+url+"\"");
}
