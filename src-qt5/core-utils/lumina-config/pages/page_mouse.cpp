//===========================================
//  Lumina Desktop Source Code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "page_mouse.h"
#include "ui_page_mouse.h"
#include "getPage.h"

#include <QSpinBox>
#include <QDoubleSpinBox>

//==========
//    PUBLIC
//==========
page_mouse::page_mouse(QWidget *parent) : PageWidget(parent), ui(new Ui::page_mouse()){
  ui->setupUi(this);
  devices = LInput::listDevices();
   //DEBUG Code
    /*qDebug() << "List Devices:";
    for(int i=0; i<devices.length(); i++){
      if(!devices[i]->isPointer()){
        ::free( devices.takeAt(i));
        i--;
      }else{
        qDebug() << "Found Pointer:" << devices[i]->devNumber();
        qDebug() << " - isExtension:" << devices[i]->isExtension();
        QList<int> props = devices[i]->listProperties();
        qDebug() << " - Properties:";
        for(int j=0; j<props.length(); j++){
          qDebug() << "   --" <<devices[i]->propertyName(props[j])+" ("+QString::number(props[j])+")" <<" = " << devices[i]->getPropertyValue(props[j]);
        }
      }
    }*/
  generateUI();
}

page_mouse::~page_mouse(){
  for(int i=0; i<devices.length(); i++){ ::free(devices[i]); }
}

//================
//    PUBLIC SLOTS
//================
void page_mouse::SaveSettings(){

  emit HasPendingChanges(false);
}

void page_mouse::LoadSettings(int){
  emit HasPendingChanges(false);
  emit ChangePageTitle( tr("Mouse Settings") );

}

void page_mouse::updateIcons(){
  for(int i=0; i<ui->tabWidget->count(); i++){
    ui->tabWidget->setTabIcon( i, LXDG::findIcon( "input-"+ui->tabWidget->tabWhatsThis(i).section(":",0,0), "" ) );
  }
}

//=================
//         PRIVATE 
//=================
void page_mouse::generateUI(){
  ui->tabWidget->clear(); //remove all tabs (just in case)
  int mouse = 1;
  int keyboard = 1;
  qDebug() << "Devices Found:" << devices.length();
  for(int i=0; i<devices.length(); i++){
    QTreeWidget *tree = 0;
    if(!devices[i]->isExtension()){
      //Make a new tab for this device
      tree = new QTreeWidget(this);
      tree->setHeaderHidden(true);
      tree->setColumnCount(2);
      if(devices[i]->isPointer()){ 
        int tab = ui->tabWidget->addTab(tree, LXDG::findIcon("input-mouse",""), QString(tr("Mouse #%1")).arg(QString::number(mouse)) ); 
        ui->tabWidget->setTabWhatsThis(tab, "mouse:"+QString::number(devices[i]->devNumber()));
        mouse++;
      }else{  
        int tab = ui->tabWidget->addTab(tree, LXDG::findIcon("input-keyboard",""), QString(tr("Keyboard #%1")).arg(QString::number(keyboard)) ); 
        ui->tabWidget->setTabWhatsThis(tab, "keyboard:"+QString::number(devices[i]->devNumber()) );
        keyboard++;
      }
    }else{
      //Find the associated tab for this extension device
      int tab = 0;
      QString type = devices[i]->isPointer() ? "mouse" : "keyboard";
      int num = devices[i]->devNumber();
      for(int t=ui->tabWidget->count()-1; t>0; t--){
        if(ui->tabWidget->tabWhatsThis(t).startsWith(type) && ui->tabWidget->tabWhatsThis(t).section(":",-1).toInt() < num ){ tab = t; break; }
      }
      tree = static_cast<QTreeWidget*>( ui->tabWidget->widget(tab) );
    }
    if(tree!=0){ populateDeviceTree(tree, devices[i]); }
  }
}

void page_mouse::populateDeviceTree(QTreeWidget *tree, LInputDevice *device){
 QTreeWidgetItem *top = new QTreeWidgetItem(tree);
  if(device->isExtension()){ 
    top->setText( 0, QString(tr("Extension Device #%1")).arg(QString::number(tree->topLevelItemCount())) );
  }else{  top->setText(0, tr("Master Device")); }
  top->setWhatsThis(0, QString(device->isPointer() ? "mouse" : "keyboard")+":"+QString::number(device->devNumber()) ); //save this for later
  top->setFirstColumnSpanned(true);
  top->setExpanded(true);
  tree->addTopLevelItem(top);
  //Now add all the child properties to this item
  QList<int> props = device->listProperties();
  for(int i=0; i<props.length(); i++){
    QTreeWidgetItem *tmp = new QTreeWidgetItem(top);
      tmp->setWhatsThis(0, QString::number(props[i]) );
      tmp->setText(0, device->propertyName(props[i]));
      top->addChild(tmp);
      populateDeviceItemValue(tree, tmp, device->getPropertyValue(props[i]), QString::number(device->devNumber())+":"+QString::number(props[i]) );     
  }
  //Clean up the tree widget as needed
  top->sortChildren(0, Qt::AscendingOrder);
  tree->resizeColumnToContents(0);
}

void page_mouse::populateDeviceItemValue(QTreeWidget *tree, QTreeWidgetItem *it, QVariant value, QString id){
  if(value.type()==QVariant::Int){
    //Could be a boolian - check the name for known "enable" states
    if(it->text(0).toLower().contains("enable") || it->text(0).toLower().contains("emulation") ){
      //Just use a checkable column within the item
      bool enabled = (value.toInt()==1);
      it->setText(1,"");
      it->setCheckState(1, enabled ? Qt::Checked : Qt::Unchecked);
    }else{
      //Use a QSpinBox
      QSpinBox *box = new QSpinBox();
        box->setRange(0,100);
        box->setValue( value.toInt() );
      tree->setItemWidget(it, 1, box);
      connect(box, SIGNAL(valueChanged(int)), this, SLOT(valueChanged()) );
    }
  }else if(value.canConvert<double>()){
  //Use a QDoubleSpinBox
  QDoubleSpinBox *box = new QDoubleSpinBox();
      box->setRange(0,100);
      box->setValue( value.toInt() );
    tree->setItemWidget(it, 1, box);
    connect(box, SIGNAL(valueChanged(double)), this, SLOT(valueChanged()) );

  }else if(value.canConvert<QList<QVariant>>()){
    //Not Modifiable - just use the label in the item
    QList<QVariant> list = value.toList();
    QStringList txtList;
    for(int i=0; i<list.length(); i++){ txtList << list[i].toString(); }
    it->setText(1, txtList.join(", ") );
  }else if( value.canConvert<QString>() ){
    //Not Modifiable - just use the label in the item
    it->setText(1, value.toString());
  }
}
//=================
//    PRIVATE SLOTS
//=================
void page_mouse::valueChanged(){
  //WILL NOT WORK - the widgets within the tree item *do not* activate the item when clicked
  //  - so the current item is NOT guaranteed to be the one which was modified
  //Get the current Tab/TreeWidget
  QTreeWidget *tree = static_cast<QTreeWidget*>(ui->tabWidget->widget( ui->tabWidget->currentIndex() ) );
  if(tree==0){ return; }
  //Now get the current item in the tree
  QTreeWidgetItem *it = tree->currentItem();
  if(it==0){ return; }
  qDebug() << "Item Value Changed:" << it->text(0);
  //Now read the value of the item and save that into the device
  QVariant value;
  if(tree->itemWidget(it, 1)!=0){
    //Got Item Widget
    
  }else if(it->text(1)==""){
    //Checkbox
    value = QVariant( (it->checkState(1)==Qt::Checked) ? 1 : 0 );
  }


}
