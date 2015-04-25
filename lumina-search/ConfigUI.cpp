//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "ConfigUI.h"
#include "ui_ConfigUI.h"

#include <QFileDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QListView>
#include <QTreeView>
#include <QPoint>
#include <QInputDialog>
#include <QMessageBox>
#include <QFile>
#include <QIODevice>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDir>

#include <LuminaXDG.h>
#include "Settings.h"

ConfigUI::ConfigUI(QWidget *parent) : QDialog(parent), ui(new Ui::ConfigUI){
  ui->setupUi(this);
  //Make sure this dialog is centered on the parent
  if(parent!=0){
    QPoint ctr = parent->geometry().center();
    this->move( ctr.x()-(this->width()/2), ctr.y()-(this->height()/2) );
  }
  ui->tool_getStartDir->setIcon( LXDG::findIcon("folder","") );
  ui->tool_adddirs->setIcon( LXDG::findIcon("list-add","") );
  ui->tool_rmdir->setIcon( LXDG::findIcon("list-remove","") );
  ui->tool_addSetName->setIcon( LXDG::findIcon("list-add","") );
  ui->tool_removeSetName->setIcon( LXDG::findIcon("list-remove","") );
  
  connect(ui->tool_addSetName, SIGNAL(clicked()), this, SLOT(on_tool_addSetName()) );
  connect(ui->tool_removeSetName, SIGNAL(clicked()), this, SLOT(on_tool_removeSetName()) );
  connect(ui->cbSetNames, SIGNAL(currentIndexChanged(int)), this, SLOT(on_cbSetName_changed(int)) );
  connect(ui->cbSetNames, SIGNAL(currentTextChanged(QString)), this, SLOT(on_cbSetName_text_changed(QString)) );
  
  currentJsonIndex = 0;
}

ConfigUI::~ConfigUI(){
	
}

void ConfigUI::loadInitialValues(){
  if (!JSonSettings::loadJsonSettings(jsonObject)) {
	  int ret = QMessageBox::critical(this, (tr("Issue to load the settings")) ,
	                        tr("We have encountered issue during the load of the settings.\nDo you want to continue with a new file?"),
	                        QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
	  //TODO: add the config filename
	  if (ret == QMessageBox::Ok) {
		  //we start with a scratch config
		  ui->label_start->setText(QDir::homePath()); //TODO: it does not work :(
		  ui->cbSetNames->addItem(tr("Default"),tr("Default"));
		  ui->list_excludes->clear();
	  } else {
		  qDebug() << "we close" ;
		  exit(1);
	  }
  } else {
      QStringList setNames = JSonSettings::getSetNames(jsonObject);
      ui->cbSetNames->addItems(setNames);
      ui->cbSetNames->setEditable(true);
  }
}

void ConfigUI::on_tool_getStartDir_clicked(){
  QString dir = QFileDialog::getExistingDirectory(this, tr("Select Search Directory"), QDir::homePath() );
  if(dir.isEmpty()){ return; }
  ui->label_start->setText(dir);

}

void ConfigUI::on_tool_adddirs_clicked(){
  QFileDialog dlg(this);
  dlg.setFileMode(QFileDialog::DirectoryOnly);
  QListView *l = dlg.findChild<QListView*>("listView");
  if(l){ l->setSelectionMode(QAbstractItemView::MultiSelection); }
  QTreeView *t = dlg.findChild<QTreeView*>();
  if(t){ t->setSelectionMode(QAbstractItemView::MultiSelection); }
  dlg.setDirectory(QDir::homePath());
  dlg.setWindowTitle( tr("Excluded Directories") ); 
  if(dlg.exec()){
    //Directories selected
    QStringList paths = dlg.selectedFiles();
    ui->list_excludes->addItems(paths);
  }
}

void ConfigUI::on_tool_rmdir_clicked(){
  qDeleteAll(ui->list_excludes->selectedItems());
}

void ConfigUI::on_list_excludes_itemSelectionChanged(){
  ui->tool_rmdir->setEnabled( !ui->list_excludes->selectedItems().isEmpty() );
}

void ConfigUI::on_buttonBox_accepted(){
  updateJsonObject(currentJsonIndex);
  if (!JSonSettings::saveJsonSettings(jsonObject)) {
	  QMessageBox::critical(this, tr("Issue to save the settings"), tr("We have encountered issue to save the settings.\nCan you check this?"));
  } else {
	  newStartDir = ui->label_start->text();
	  QStringList dirs;
	  for(int i=0; i<ui->list_excludes->count(); i++){
		dirs << ui->list_excludes->item(i)->text();
	  }
	  dirs.removeDuplicates();
	  newSkipDirs = dirs;
	  this->close();
  }
}

void ConfigUI::on_buttonBox_rejected(){
  this->close();
}

void ConfigUI::on_tool_addSetName() {
	bool ok;
    QString text = QInputDialog::getText(this, tr("Create a new Set Name"),
                                         tr("Name:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty()) {
        ui->cbSetNames->addItem(text,text);
        ui->cbSetNames->setCurrentIndex(ui->cbSetNames->count()-1);
	}
}

void ConfigUI::on_tool_removeSetName() {
	int selIndex = ui->cbSetNames->currentIndex();
	qDebug() << selIndex;
	if (selIndex == 0) {
		QMessageBox::critical(this, tr("Default cannot be deleted"), tr("You cannot delete this Set Name"));
	} else {
	    ui->cbSetNames->removeItem(selIndex);
	    QJsonArray newArray = jsonObject["Sets"].toArray();
	    newArray.removeAt(selIndex);
	    jsonObject["Sets"] = newArray;
    }
}

void ConfigUI::on_cbSetName_changed(int index){
	QString startDir;
	QStringList excludedDirs;
	if (currentJsonIndex != index)
	{
		//before update the fields, we save them
		if (!updateJsonObject(currentJsonIndex)) {
			QMessageBox::critical(this, tr("Start directory") + tr(" is not set"), tr("Please set the ") + tr("Start directory") + tr(" before coninuing"));
			ui->cbSetNames->setCurrentIndex(currentJsonIndex);
			return;
		}
	}
	currentJsonIndex = index;
	if (JSonSettings::getSetDetails(jsonObject, index, startDir, excludedDirs)) {
		ui->label_start->setText(startDir);
		ui->list_excludes->clear();
		ui->list_excludes->addItems(excludedDirs);
	}
}

void ConfigUI::on_cbSetName_text_changed(QString newText)
{
	QJsonArray entries = jsonObject["Sets"].toArray();
	QJsonObject fstObject = entries[0].toObject();
	if ((ui->cbSetNames->currentIndex() == 0) && (fstObject["Name"].isString()) && (fstObject["Name"].toString() != newText)) {
		qWarning("You cannot change this Set's name"); //TODO: add translate
		QMessageBox::critical(this, tr("Default cannot be changed"), tr("You cannot change this Set Name.\nThe change you perform will not be taken into account"));
		return;
	}
	ui->cbSetNames->setItemText(ui->cbSetNames->currentIndex(),newText);
}

bool ConfigUI::updateJsonObject(int index)
{
    if (ui->label_start->text().isEmpty()) {
		return false;
	}
	QJsonObject entry;
	entry["Name"] = QJsonValue(ui->cbSetNames->itemText(index));
	entry["StartDir"]= ui->label_start->text();
	QJsonArray jsonExcludedDirs;
	for (int i = 0; i < ui->list_excludes->count(); ++i) {
		jsonExcludedDirs.append(QJsonValue(ui->list_excludes->item(i)->text()));
	}
	entry["ExcludedDirs"] = jsonExcludedDirs;
	QJsonArray newArray = jsonObject["Sets"].toArray();
	if (index < newArray.count() ) {
	    newArray.replace(currentJsonIndex,entry);
	} else {
		newArray.append(entry);
	}
	jsonObject["Sets"] = newArray;
	return true;
}
