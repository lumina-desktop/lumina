//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the dialog for catching keyboard events and converting them to X11 keycodes
//===========================================
#ifndef _LUMINA_FILE_MANAGER_APP_SELECT_DIALOG_H
#define _LUMINA_FILE_MANAGER_APP_SELECT_DIALOG_H

#include <QDialog>
#include <QString>
#include <QList>
#include <QPoint>
#include <QDesktopWidget>
#include <QCursor>
#include <LuminaXDG.h>

#include "ui_AppDialog.h"

namespace Ui{
	class AppDialog;
};

class AppDialog : public QDialog{
	Q_OBJECT
private:
	Ui::AppDialog *ui;
	QList<XDGDesktop> APPS;

public:
	AppDialog(QWidget *parent = 0, QList<XDGDesktop> applist = QList<XDGDesktop>()) : QDialog(parent), ui(new Ui::AppDialog){
	  ui->setupUi(this); //load the designer file
	  APPS = applist; //save this for later
	  ui->comboBox->clear();
	  for(int i=0; i<APPS.length(); i++){
	    ui->comboBox->addItem( LXDG::findIcon(APPS[i].icon,"application-x-executable"), APPS[i].name );
	  }
	  this->setWindowIcon( LXDG::findIcon("system-search","") );
	  QPoint center = QApplication::desktop()->screenGeometry(QCursor::pos()).center();
	  this->move(center.x()-(this->width()/2), center.y()-(this->height()/2) );
	}
	
	~AppDialog(){}

	XDGDesktop appselected;
	
private slots:
	void on_buttonBox_accepted(){
	  appselected = APPS[ ui->comboBox->currentIndex() ];
	  this->close();
	}
	void on_buttonBox_rejected(){
	  this->close();
	}

};

#endif
