//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This is the dialog for selecting an installed application
//===========================================
#ifndef _LUMINA_FILE_MANAGER_APP_SELECT_DIALOG_H
#define _LUMINA_FILE_MANAGER_APP_SELECT_DIALOG_H

#include "globals.h"

#include "ui_AppDialog.h"

namespace Ui{
	class AppDialog;
};

class AppDialog : public QDialog{
	Q_OBJECT
private:
	Ui::AppDialog *ui;

public:
	AppDialog(QWidget *parent = 0) : QDialog(parent), ui(new Ui::AppDialog){
	  ui->setupUi(this); //load the designer file
	  appreset = false;
	  ui->comboBox->clear();
          QList<XDGDesktop*> APPS = LXDG::sortDesktopNames(APPSLIST->apps(false,false)); //Don't show all/hidden
	  for(int i=0; i<APPS.length(); i++){
	    ui->comboBox->addItem( LXDG::findIcon(APPS[i]->icon,"application-x-executable"), APPS[i]->name, APPS[i]->filePath);
	  }
	  this->setWindowIcon( LXDG::findIcon("system-search","") );
	  if(parent!=0){
	    QWidget *top = parent;
	    while(!top->isWindow()){ top = top->parentWidget(); }
	    QPoint center = top->geometry().center();
	    this->move(center.x()-(this->width()/2), center.y()-(this->height()/2) );
	  }
	}
	
	~AppDialog(){}

	void allowReset(bool allow){
	  if(allow){
	    ui->buttonBox->setStandardButtons(QDialogButtonBox::RestoreDefaults | QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	  }else{
	    ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	  }
	}

	QString appselected; //selected application
	bool appreset; //Did the user select to reset to defaults?
	
		
private slots:
	void on_buttonBox_accepted(){
	  appselected = ui->comboBox->currentData().toString();
	  this->close();
	}
	void on_buttonBox_rejected(){
	  this->close();
	}
	void on_buttonBox_clicked(QAbstractButton *button){
	  if(ui->buttonBox->standardButton(button) == QDialogButtonBox::RestoreDefaults){
	    appreset = true;
	    this->close();
	  }
	}
};

#endif
