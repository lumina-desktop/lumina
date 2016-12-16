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
  emit ChangePageTitle( tr("Input Device Settings") );

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
    if(!devices[i]->isExtension() || devices[i]->isPointer()){
      if(devices[i]->isPointer() && devices[i]->listProperties().count() <4){ continue; } //filter out all the trivial/static mouse devices
      //Make a new tab for this device
      tree = new QTreeWidget(this);
      tree->setHeaderHidden(true);
      tree->setColumnCount(2);
      connect(tree, SIGNAL(itemActivated(QTreeWidgetItem*, int)), this, SLOT(itemClicked(QTreeWidgetItem*,int)) );
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
    if(device->propertyName(props[i]).toLower().contains("matrix")){ continue; } //skip this one - can not change from UI and most people will never want to anyway
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
    if(value.toInt() < 2 && (it->text(0).toLower().contains("enable") || it->text(0).toLower().contains("emulation") || it->text(0)==("XTEST Device") ) ){
      //Just use a checkable column within the item
      bool enabled = (value.toInt()==1);
      it->setText(1,"");
      it->setWhatsThis(1, "bool:"+id);
      it->setCheckState(1, enabled ? Qt::Checked : Qt::Unchecked);
    }else{
      //Use a QSpinBox
      QSpinBox *box = new QSpinBox();
        box->setRange(0,100);
        box->setValue( value.toInt() );
	box->setWhatsThis("int:"+id);
      tree->setItemWidget(it, 1, box);
      connect(box, SIGNAL(valueChanged(int)), this, SLOT(valueChanged()) );
    }
  }else if(value.type()==QVariant::Double){
  //Use a QDoubleSpinBox
  QDoubleSpinBox *box = new QDoubleSpinBox();
      box->setRange(0,1000);
      box->setValue( value.toInt() );
      box->setWhatsThis("double:"+id);
    tree->setItemWidget(it, 1, box);
    connect(box, SIGNAL(valueChanged(double)), this, SLOT(valueChanged()) );

  }else if(value.canConvert< QList<QVariant> >()){
    //Not Modifiable - just use the label in the item
    QList<QVariant> list = value.toList();
    QStringList txtList;
    for(int i=0; i<list.length(); i++){ txtList << list[i].toString(); }
    it->setText(1, txtList.join(", ") );
    it->setToolTip(1, txtList.join(", "));
  }else if( value.canConvert<QString>() ){
    //Not Modifiable - just use the label in the item
    it->setText(1, value.toString());
  }
}
//=================
//    PRIVATE SLOTS
//=================
void page_mouse::valueChanged(){
  //Now get the currently focused widget
   QWidget *foc = this->focusWidget();
   if(foc==0){ return; }
  //qDebug() << "Focus Widget:" << foc->whatsThis();
  //Now pull out the value and device/property numbers
  unsigned int dev = foc->whatsThis().section(":",1,1).toInt();
  int prop = foc->whatsThis().section(":",2,2).toInt();
  QVariant value;
  if(foc->whatsThis().startsWith("int:")){ value.setValue( static_cast<QSpinBox*>(foc)->value() ); }
  else if(foc->whatsThis().startsWith("double:")){ value.setValue( static_cast<QDoubleSpinBox*>(foc)->value() ); }
  //Now change the property for the device
  qDebug() << " - Device:" <<dev << "prop:" << prop << "value:" << value;
  for(int i=0; i<devices.length(); i++){
    if(devices[i]->devNumber() == dev){
      bool ok = devices[i]->setPropertyValue(prop, value);
      if(ok){ foc->setStyleSheet(""); }
      else{ foc->setStyleSheet("background: red"); }
      //qDebug() << " - Changed property:" << (ok ? "success" : "failure");
      break;
    }
  }
}

void page_mouse::itemClicked(QTreeWidgetItem *it, int col){
  if(col!=1){ return; } //only care about value changes
  if(it->whatsThis(1).isEmpty()){ return; }//not a checkable item
  qDebug() << "item Clicked:" << it->whatsThis(1) << it->text(0);
  //Now pull out the value and device/property numbers
  unsigned int dev = it->whatsThis(1).section(":",1,1).toInt();
  int prop = it->whatsThis(1).section(":",2,2).toInt();
  QVariant value( (it->checkState(1)==Qt::Checked) ? 1 : 0 );
 //Now change the property for the device
  qDebug() << " - Device:" <<dev << "prop:" << prop << "value:" << value;
  for(int i=0; i<devices.length(); i++){
    if(devices[i]->devNumber() == dev){
      //Since this "clicked" signal can get sent out even if the value has not changed, go ahead and make sure we have a different value first
      QVariant current = devices[i]->getPropertyValue(prop);
      //qDebug() << " - Current Value:" << current;
      if(value.toInt()!=current.toInt()){
        bool ok = devices[i]->setPropertyValue(prop, value);
        //if(ok){ foc->setStyleSheet(""); }
        //else{ foc->setStyleSheet("background: red"); }
        qDebug() << " - Changed property:" << (ok ? "success" : "failure");
      }
      break;
    }
  }
}
