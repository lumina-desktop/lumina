//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "BMMDialog.h"
#include "ui_BMMDialog.h"

BMMDialog::BMMDialog(QWidget *parent) : QDialog(parent), ui(new Ui::BMMDialog){
  ui->setupUi(this); //load the designer file
  this->setWindowIcon( LXDG::findIcon("bookmarks-organize","") );
  ui->tool_remove->setIcon( LXDG::findIcon("edit-delete","") );
  ui->tool_rename->setIcon( LXDG::findIcon("edit-rename","") );
  ui->push_done->setIcon( LXDG::findIcon("dialog-ok","") );
  connect(ui->tool_remove, SIGNAL(clicked()), this, SLOT(RemoveItem()) );
  connect(ui->tool_rename, SIGNAL(clicked()), this, SLOT(RenameItem()) );
  connect(ui->push_done, SIGNAL(clicked()), this, SLOT(close()) );	
}

BMMDialog::~BMMDialog(){
}

void BMMDialog::loadSettings(QSettings *set){
  settings = set; //save this pointer for later
  //Now fill the tree with the items
  QStringList BM = settings->value("bookmarks", QStringList()).toStringList();
  ui->treeWidget->clear();
  for(int i=0; i<BM.length(); i++){
    ui->treeWidget->addTopLevelItem( new QTreeWidgetItem(BM[i].split("::::")) );
  }
  //Now expand to encompass all the items
  ui->treeWidget->resizeColumnToContents(0);
  ui->treeWidget->resizeColumnToContents(1);
}
// ==== PRIVATE ====

// ==== PRIVATE SLOTS ====
void BMMDialog::RemoveItem(){
  //Get the currently selected item
  if(ui->treeWidget->currentItem()==0){ return; } //nothing selected
  QString item = ui->treeWidget->currentItem()->text(0)+"::::"+ui->treeWidget->currentItem()->text(1);
  //Remove it from the widget
  delete ui->treeWidget->takeTopLevelItem( ui->treeWidget->indexOfTopLevelItem( ui->treeWidget->currentItem() ) );
  //Remove it from the saved bookmarks
  QStringList BM = settings->value("bookmarks",QStringList()).toStringList();
  BM.removeAll(item);
  settings->setValue("bookmarks",BM);
  settings->sync();
}

void BMMDialog::RenameItem(){
  //Get the currently selected item
  if(ui->treeWidget->currentItem()==0){ return; } //nothing selected
  QString olditem = ui->treeWidget->currentItem()->text(0)+"::::"+ui->treeWidget->currentItem()->text(1);
  //Prompt for the new name
  bool ok = false;
  QString name = QInputDialog::getText(this, tr("Rename Bookmark"), tr("Name:"), QLineEdit::Normal, olditem.section("::::",0,0), \
		&ok, 0, Qt::ImhFormattedNumbersOnly | Qt::ImhUppercaseOnly | Qt::ImhLowercaseOnly);
  if(!ok || name.isEmpty()){ return; } //cancelled
  //Check if this name already exists
  QStringList BM = settings->value("bookmarks",QStringList()).toStringList();
  if(BM.filter(name+"::::").length() >0){
    QMessageBox::warning(this, tr("Invalid Name"), tr("This bookmark name already exists. Please choose another.") );
    QTimer::singleShot(0,this, SLOT(RenameItem()));
    return;
  }
  //Rename it in the widget
  ui->treeWidget->currentItem()->setText(0,name);
  //Replace it in the saved bookmarks
  BM.removeAll(olditem);
  BM.append(name+"::::"+olditem.section("::::",1,3));
  settings->setValue("bookmarks",BM);
  settings->sync();	
}