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

#include <LuminaXDG.h>

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
  loadJsonSettings();
  QStringList setNames = getSetNames();
  ui->cbSetNames->addItems(setNames);
  ui->cbSetNames->setEditable(true);
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
  qDebug() << "click save button";
  saveJsonSettings();
  newStartDir = ui->label_start->text();
  QStringList dirs;
  for(int i=0; i<ui->list_excludes->count(); i++){
    dirs << ui->list_excludes->item(i)->text();
  }
  dirs.removeDuplicates();
  newSkipDirs = dirs;
  this->close();
}

void ConfigUI::on_buttonBox_rejected(){
  this->close();
}

void ConfigUI::on_tool_addSetName() {
	qDebug("add a set name");
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
	qDebug("remove a set Name");
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
	qDebug() << "SetName changed: index" << index << " currentJsonIndex:" << currentJsonIndex;
	QString startDir;
	QStringList excludedDirs;
	if (currentJsonIndex != index)
	{
		//before update the fields, we save them
		updateJsonObject(currentJsonIndex);
	}
	currentJsonIndex = index;
	if (getSetDetails(index, startDir, excludedDirs)) {
		qDebug() << "Set details:" << startDir << ";" << excludedDirs;
		ui->label_start->setText(startDir);
		ui->list_excludes->clear();
		ui->list_excludes->addItems(excludedDirs);
	}
}

void ConfigUI::on_cbSetName_text_changed(QString newText)
{
	qDebug() << "combo text changed:" << newText;
	if (ui->cbSetNames->currentIndex() == 0) {
		qWarning("You cannot change this Set's name");
		return;
	}
	ui->cbSetNames->setItemText(ui->cbSetNames->currentIndex(),newText);
}

bool ConfigUI::loadJsonSettings() {
	qDebug("loadJsonSettings");
	QFile loadFile(QStringLiteral("search.settings"));
	if (!loadFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qWarning("Problem to read the settings!!!");
		return false;
	}
	QByteArray jsonData = loadFile.readAll();
	loadFile.close();
	QJsonParseError parseError;
	QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
	if (parseError.error)
	{
		//TODO: such message could maybe in a log file
        qWarning() << "Problem to parse the setting file here:" << jsonData.mid( parseError.offset - 10, parseError.offset + 10).replace('\n', ' ');    
        qWarning() << parseError.errorString()  ;
        jsonObject = jsonDoc.object();
        return false;
	}
    jsonObject = jsonDoc.object();
    qDebug() << "json root object:" << jsonObject["Sets"].toArray();
    QStringList names = getSetNames();
    if (names.at(0) != "Default") {
		qWarning("settings incorrect, it does not contains 'Default'");
		return false;
	}
    return true;
}

void ConfigUI::updateJsonObject(int index)
{
	qDebug("updateJsonObject");
	QJsonObject entry;
	entry["Name"] = QJsonValue(ui->cbSetNames->itemText(index));
	qDebug() << "index name:" << QJsonValue(ui->cbSetNames->itemText(index));
	qDebug() << "current name" << QJsonValue(ui->cbSetNames->currentText());
	entry["StartDir"]= ui->label_start->text();
	QJsonArray jsonExcludedDirs;
	for (int i = 0; i < ui->list_excludes->count(); ++i) {
		jsonExcludedDirs.append(QJsonValue(ui->list_excludes->item(i)->text()));
	}
	entry["ExcludedDirs"] = jsonExcludedDirs;
	qDebug() << "entry updated:" << currentJsonIndex;
	qDebug() << entry;
	QJsonArray newArray = jsonObject["Sets"].toArray();
	if (index < newArray.count() ) {
		qDebug("Update the entry");
	    newArray.replace(currentJsonIndex,entry);
	} else {
		qDebug("Append the entry");
		newArray.append(entry);
	}
	jsonObject["Sets"] = newArray;
	qDebug() << "New root object:" << jsonObject["Sets"].toArray();
}

bool ConfigUI::saveJsonSettings() {
	qDebug("saveJsonSettings");
	updateJsonObject(currentJsonIndex);
	qDebug() << "Json Root object:" << jsonObject;
	QFile saveFile("search.settings");
	if (!saveFile.open(QIODevice::WriteOnly)) {
		qWarning("Problem to read the settings!!!");
		return false;
	}
	QJsonDocument saveDoc(jsonObject);
	saveFile.write(saveDoc.toJson());
	saveFile.close();
	return true;
}

QStringList ConfigUI::getSetNames() {
	qDebug("getSetNames");
	QStringList setNames;
	qDebug() << "json root object:" << jsonObject["Sets"].toArray();
	QJsonArray jsonEntries = jsonObject["Sets"].toArray();
	for (int i = 0; i < jsonEntries.size(); ++i) {
		QJsonObject entry = jsonEntries[i].toObject();
		setNames.append(entry["Name"].toString());
	}
	qDebug() << setNames;
	return setNames;
}

bool ConfigUI::getSetDetails(int index, QString &startDir, QStringList &excludedDirs){
	qDebug() << "getSetDetails:" << index;
	excludedDirs.clear();
	QJsonArray jsonEntries = jsonObject["Sets"].toArray();
	QJsonObject entry = jsonEntries[index].toObject();
    startDir = entry["StartDir"].toString();
    QJsonArray jsonExcludedDirs = entry["ExcludedDirs"].toArray();
    for (int i = 0; i < jsonExcludedDirs.size(); ++i) {
		excludedDirs.append(jsonExcludedDirs[i].toString());
	}
	return true;
}
