#include "mainUI.h"
#include "ui_mainUI.h"

#include <QJsonDocument>
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QMenu>

// === PUBLIC ===
mainUI::mainUI() : QMainWindow(), ui(new Ui::mainUI()){
  ui->setupUi(this);
  NETWORK = new Networking(this);
  connect(NETWORK, SIGNAL(starting_wifi_scan()), this, SLOT(starting_wifi_scan()) );
  connect(NETWORK, SIGNAL(finished_wifi_scan()), this, SLOT(finished_wifi_scan()) );
  connect(NETWORK, SIGNAL(new_wifi_scan_results()), this, SLOT(updateWifiConnections()) );
  connect(ui->tool_wifi_refresh, SIGNAL(clicked()), NETWORK, SLOT(startWifiScan()) );

  ui->actionConnections->setEnabled( QFile::exists("/var/service/dhcpcd") );
  page_group = new QActionGroup(this);
    page_group->setExclusive(true);
    page_group->addAction(ui->actionConnections);
    page_group->addAction(ui->actionFirewall);
    page_group->addAction(ui->actionVPN);
    page_group->addAction(ui->actionDNS);
  connect(page_group, SIGNAL(triggered(QAction*)), this, SLOT(pageChange(QAction*)) );

  ui->tabs_conn->setCurrentWidget(ui->tab_conn_status);
  connect(ui->combo_conn_devices, SIGNAL(currentIndexChanged(int)), this, SLOT(updateConnectionInfo()) );
  connect(ui->tool_conn_status_refresh, SIGNAL(clicked()), this, SLOT(updateConnectionInfo()) );

  //Setup the menu of shortcuts for firewall rules
  ui->tool_fw_shortcuts->setMenu(new QMenu(ui->tool_fw_shortcuts));
  ui->tool_fw_shortcuts->menu()->addAction(tr("Open Port"), this, SLOT(open_fw_port()) );
  ui->tool_fw_shortcuts->menu()->addAction(tr("Open Port for Service"), this, SLOT(open_fw_service()) );

  //Ensure all the page actions are full-width
  this->show();
  QApplication::processEvents();
  int wid = ui->toolBar->width();
  ui->toolBar->widgetForAction(ui->actionConnections)->setMinimumWidth(wid);
  ui->toolBar->widgetForAction(ui->actionFirewall)->setMinimumWidth(wid);
  ui->toolBar->widgetForAction(ui->actionVPN)->setMinimumWidth(wid);
  ui->toolBar->widgetForAction(ui->actionDNS)->setMinimumWidth(wid);
  connect(ui->tool_dns_apply, SIGNAL(clicked()), this, SLOT(apply_dns_settings()) );
  connect(ui->tool_refresh_dns_status, SIGNAL(clicked()), this, SLOT(rescan_current_dns()) );
  connect(ui->tool_wg_refresh, SIGNAL(clicked()), this, SLOT(refresh_current_wireguard()) );
}

mainUI::~mainUI(){

}

// === PUBLIC SLOTS ===
void mainUI::newInputs(QStringList args){
  if(args.isEmpty() || args.contains("-connections") ){
    ui->actionConnections->trigger();
  }else if(args.contains("-firewall")){
    ui->actionFirewall->trigger();
  }else if(args.contains("-vpn")){
    ui->actionVPN->trigger();
  }else if(args.contains("-dns")){
    ui->actionDNS->trigger();
  }

}

// === PRIVATE ===
//Initial page loading (on page change)
void mainUI::updateConnections(){
  //qDebug() << "Update Connections";
  QStringList devs = NETWORK->list_devices();
  static QStringList lastdevs;
  if(devs == lastdevs){ return; } //no change
  lastdevs = devs;
  devs.sort();
  QString cdev = ui->combo_conn_devices->currentText();
  ui->combo_conn_devices->clear();
  bool haswifi = false;
  int index = -1;
  for(int i=0; i<devs.length(); i++){
    if(devs[i].isEmpty()){ continue; }
    bool is_wifi = devs[i].startsWith("wl");
    ui->combo_conn_devices->addItem(QIcon::fromTheme( is_wifi ? "network-wireless" : "network-wired-activated"), devs[i]);
    haswifi = (haswifi || is_wifi);
    if(cdev == devs[i]){ index = i; }
    else if(cdev.isEmpty() && is_wifi){ index = i; }
  }
  ui->tab_conn_wifi->setEnabled(haswifi && QFile::exists("/var/service/wpa_supplicant"));
  if(index>=0){ ui->combo_conn_devices->setCurrentIndex(index); }
  if(cdev.isEmpty() && haswifi){
    //First time loading the device list - go ahead and start a wifi scan in the background
    QTimer::singleShot(500, NETWORK, SLOT(startWifiScan()));
  }
}

void mainUI::updateFirewall(){
  refresh_current_firewall();
}

void mainUI::updateVPN(){
  refresh_current_wireguard();
  ui->tab_vpn_wireguard->setEnabled( QFile::exists("/usr/bin/wg-quick") );
}

void mainUI::updateDNS(){
  ui->tabWidget_dns->setCurrentIndex(0);
  rescan_current_dns();
  update_dns_settings();
}

// === PRIVATE SLOTS ===
void mainUI::pageChange(QAction *triggered){
  if(triggered == ui->actionConnections){
	ui->stackedWidget->setCurrentWidget(ui->page_connections);
	updateConnections();
        loadStaticProfiles();
  }else if(triggered == ui->actionFirewall ){
	ui->stackedWidget->setCurrentWidget(ui->page_firewall);
	updateFirewall();
  }else if(triggered == ui->actionDNS ){
	ui->stackedWidget->setCurrentWidget(ui->page_dns);
	updateDNS();
  }else if(triggered == ui->actionVPN ){
	ui->stackedWidget->setCurrentWidget(ui->page_vpn);
	updateVPN();
  }
}

void mainUI::updateConnectionInfo(){
  QString cdev = ui->combo_conn_devices->currentText();
  if(cdev.isEmpty()){ return; } //no devices loaded (yet)
  //QJsonObject config = NETWORK->list_config();
  QJsonObject status = NETWORK->current_info(cdev);
  //qDebug() << "Got Info:" << cdev << config;

  // Current status display
  bool running = status.value("is_running").toBool();
  ui->tool_dev_start->setVisible(!running);
  ui->tool_dev_restart->setVisible(running);
  ui->tool_dev_stop->setVisible(running);
  //Assemble the output text for the current status
  QString skel = "<p><h3><b>%1</b></h3>%2</p>";
  QStringList textblocks;
  if(status.contains("wifi")){
    QJsonObject wifi = status.value("wifi").toObject();
    QStringList info;
      info << QString(tr("Access Point: %1")).arg("<i>"+wifi.value("ssid").toString()+"</i>");
      info << QString(tr("Security: %1")).arg("<i>"+wifi.value("key_mgmt").toString()+"</i>");
    textblocks << skel.arg( tr("Wireless Status"), "<ul><li>"+info.join("</li><li>")+"</li></ul>");
  }
/*  if(status.contains("lan")){
    QJsonObject lan = status.value("lan").toObject();
    QStringList info;
      info << QString(tr("Connection: %1")).arg("<i>"+lan.value("media").toString()+"</i>");
    textblocks << skel.arg( tr("Wired Status"), "<ul><li>"+info.join("</li><li>")+"</li></ul>");
  }*/
  if(status.contains("ipv4")){
    QStringList info;
      info << QString(tr("Address: %1")).arg("<i>"+status.value("ipv4").toString()+"</i>");
      info << QString(tr("Broadcast: %1")).arg("<i>"+status.value("ipv4_broadcast").toString()+"</i>");
      info << QString(tr("Netmask: %1")).arg("<i>"+status.value("ipv4_netmask").toString()+"</i>");
    textblocks << skel.arg( tr("IPv4"), "<ul><li>"+info.join("</li><li>")+"</li></ul>");
  }
  if(status.contains("ipv6")){
    QStringList info;
      info << QString(tr("Address: %1")).arg("<i>"+status.value("ipv6").toString()+"</i>");
      if(!status.value("ipv6_broadcast").toString().isEmpty()){ info << QString(tr("Broadcast: %1")).arg("<i>"+status.value("ipv6_broadcast").toString()+"</i>"); }
      info << QString(tr("Netmask: %1")).arg("<i>"+status.value("ipv6_netmask").toString()+"</i>");
    textblocks << skel.arg( tr("IPv6"), "<ul><li>"+info.join("</li><li>")+"</li></ul>");
  }
  QString state = tr("Inactive");
  bool auto_check = true;
  if(status.value("is_active").toBool()){ state = tr("Connected"); auto_check = false; }
  else if(status.value("is_running").toBool() && status.value("is_up").toBool()){
      state = tr("Waiting for connection");
  }
  textblocks.prepend( QString(tr("Current Status: %1")).arg("<i>"+state+"</i>") );
  //ui->text_conn_dev_status->setText(QJsonDocument(status).toJson(QJsonDocument::Indented));
  ui->text_conn_dev_status->setText(textblocks.join(""));
  if(auto_check){
    //Check again in a half second - waiting for status change
    QTimer::singleShot(500, this, SLOT(updateConnectionInfo()) );
  }
  if(status.contains("wifi")){
    //Resync the status of the wifi scan as well (does not start a new scan)
    QTimer::singleShot(10, this, SLOT(updateWifiConnections()) );
  }
}

QTreeWidgetItem* mainUI::generateWifi_item(QJsonObject obj, QJsonObject active){
  QTreeWidgetItem *it = new QTreeWidgetItem();
  QString ssid = obj.value("ssid").toString();
  it->setData(0, Qt::UserRole, obj.value("bssid").toString() );
  it->setText(1, obj.value("signal").toString() );
  it->setText(2, ssid.isEmpty() ? "[Hidden] "+obj.value("bssid").toString() : ssid);
  if(obj.value("is_locked").toBool()){ it->setIcon(0, QIcon::fromTheme("password")); }
  //See if the network is currently known
  bool is_known = NETWORK->is_known(obj);
  obj.insert("is_known", is_known);
  if(is_known){ it->setIcon(2, QIcon::fromTheme("tag")); }
  //See if the network is currently active
  bool is_active = Networking::sameNetwork(obj, active);
  obj.insert("is_active", is_active);
  if(is_active){it->setIcon(1, QIcon::fromTheme("network-wireless")); }
  //Save the current info object to the item
  it->setData(1, Qt::UserRole, obj);
  return it;
}

void mainUI::updateWifiConnections(){
  QJsonObject scan = NETWORK->wifi_scan_results();
  QJsonObject activeWifi = NETWORK->active_wifi_network();
  //Make sure the current item stays selected if possible
  QString citem = "";
  if(ui->tree_wifi_networks->currentItem() != 0){
    citem = ui->tree_wifi_networks->currentItem()->data(0, Qt::UserRole).toString();
  }
  //Now update the tree widget
  ui->tree_wifi_networks->clear();
  QStringList ssids = scan.keys();
  QTreeWidgetItem *sel = 0;
  for(int i=0; i<ssids.length(); i++){
    QJsonValue info = scan.value(ssids[i]);
    QTreeWidgetItem *it = 0;

    if(info.isObject()){
      it = generateWifi_item(info.toObject(), activeWifi);
      if(it->data(0, Qt::UserRole).toString() == citem){ sel = it; }

    }else if(info.isArray()){
      QJsonArray arr = info.toArray();
      it = new QTreeWidgetItem();
      it->setText(1, "---");
      it->setText(2, ssids[i].isEmpty() ? "[Hidden]" : ssids[i]);
      it->setData(0, Qt::UserRole, ssids[i]); //Mesh network - identify with the ssid instead of bssid
      for(int a=0; a<arr.count(); a++){
        QTreeWidgetItem *sit = generateWifi_item(arr[a].toObject(), activeWifi);
        if(sit->data(0, Qt::UserRole).toString() == citem){ sel = sit; }
        it->addChild(sit);
      }

    }
    ui->tree_wifi_networks->addTopLevelItem(it);
  }

  ui->tree_wifi_networks->sortItems(1, Qt::DescendingOrder);
  ui->tree_wifi_networks->resizeColumnToContents(0);
  ui->tree_wifi_networks->resizeColumnToContents(1);
  //Now re-select the item before the refresh if possible
  if(sel!=0){
    ui->tree_wifi_networks->setCurrentItem(sel);
    ui->tree_wifi_networks->scrollToItem(sel);
  }else{
    on_tree_wifi_networks_currentItemChanged(0);
  }
}

void mainUI::on_tree_wifi_networks_currentItemChanged(QTreeWidgetItem *it){
  bool is_known = false;
  bool is_active = false;
  bool is_group = false;
  if(it!=0){
    is_group = (it->childCount() > 0);
    QJsonObject info = it->data(1, Qt::UserRole).toJsonObject();
    is_known = info.value("is_known").toBool(false);
    is_active = info.value("is_active").toBool(false);
  }else{ is_group = true; } //just hide everything if no item selected
  ui->tool_forget_wifi->setVisible(is_known && !is_group);
  ui->tool_connect_wifi->setVisible(!is_active && !is_group);
}

void mainUI::on_tool_dev_restart_clicked(){
  QString cdev = ui->combo_conn_devices->currentText();
  if(cdev.isEmpty()){ return; } //no devices loaded (yet)
  NETWORK->setDeviceState(cdev, Networking::StateRestart);
  QTimer::singleShot(500, this, SLOT(updateConnectionInfo()));
  //Send a couple automatic status updates 5 & 10 seconds later
  QTimer::singleShot(5000, this, SLOT(updateConnectionInfo()));
  QTimer::singleShot(10000, this, SLOT(updateConnectionInfo()));
}

void mainUI::on_tool_dev_start_clicked(){
  QString cdev = ui->combo_conn_devices->currentText();
  if(cdev.isEmpty()){ return; } //no devices loaded (yet)
  NETWORK->setDeviceState(cdev, Networking::StateRunning);
  QTimer::singleShot(500, this, SLOT(updateConnectionInfo()));
  //Send a couple automatic status updates 5 & 10 seconds later
  QTimer::singleShot(5000, this, SLOT(updateConnectionInfo()));
  QTimer::singleShot(10000, this, SLOT(updateConnectionInfo()));
}

void mainUI::on_tool_dev_stop_clicked(){
  QString cdev = ui->combo_conn_devices->currentText();
  if(cdev.isEmpty()){ return; } //no devices loaded (yet)
  NETWORK->setDeviceState(cdev, Networking::StateStopped);
  QTimer::singleShot(500, this, SLOT(updateConnectionInfo()));
}

void mainUI::on_tool_forget_wifi_clicked(){
  QTreeWidgetItem *curit = ui->tree_wifi_networks->currentItem();
  if(curit == 0){ return; } //nothing selected
  QJsonObject info = curit->data(1, Qt::UserRole).toJsonObject();
  if(info.isEmpty()){ return; } // nothing to do
  //Get the known network entry which matches
  QJsonArray known = NETWORK->known_wifi_networks();
  for(int i=0; i<known.count(); i++){
    if( Networking::sameNetwork(known[i].toObject(), info) ){
      if( NETWORK->save_wifi_network(known[i].toObject(), true) ){
        QTimer::singleShot(50, this, SLOT(updateWifiConnections()) );
      }else{
        QMessageBox::warning(this, tr("Error"), QString(tr("Could not forget network settings: %1")).arg(info.value("ssid").toString()) );
      }
      break; //found network entry
    }
  }
}

void mainUI::on_tool_connect_wifi_clicked(){
  QTreeWidgetItem *curit = ui->tree_wifi_networks->currentItem();
  if(curit == 0){ return; } //nothing selected
  QJsonObject info = curit->data(1, Qt::UserRole).toJsonObject();
  bool part_of_group = (curit->parent() !=0);
  QString id = curit->data(0, Qt::UserRole).toString();
  if(info.isEmpty() || id.isEmpty()){ return; } //nothing selected
  //qDebug() << "Connect to wifi:" << id << info;
  if(NETWORK->is_known(info)){
    bool ok = NETWORK->connect_to_wifi_network(info); //just connect to this known network
    if(!ok){
      QMessageBox::warning(this, tr("Error"), QString(tr("Could not connect to network: %1")).arg(info.value("ssid").toString()) );
    }
  }else{
    //See if we need to save connection info first
    bool secure = info.value("is_locked").toBool();
    if(secure){
      QString psk = QInputDialog::getText(this, tr("Wifi Passphrase"), tr("Enter the wifi access point passphrase"), QLineEdit::Password);
      if(psk.isEmpty()){ return; } //cancelled
      info.insert("psk",psk);
    }
    if(part_of_group){
      //Ask whether to save the entire group, or just that one access point
      bool roam = (QMessageBox::Yes == QMessageBox::question(this, tr("Roam group?"), tr("Do you want to roam between access points for this network?")) );
      if(roam){ info.remove("bssid"); } //Just use the generic ssid
    }
    NETWORK->save_wifi_network(info, false);
    if(!secure){ QTimer::singleShot(5000, this, SLOT(LaunchBrowser())); } //for capture-portal login
  }
  ui->tabs_conn->setCurrentWidget(ui->tab_conn_status);
  QTimer::singleShot(50, this, SLOT(updateConnectionInfo()) );
}

void mainUI::starting_wifi_scan(){
  ui->tool_wifi_refresh->setEnabled(false);
}

void mainUI::finished_wifi_scan(){
  ui->tool_wifi_refresh->setEnabled(true);
}

void mainUI::loadStaticProfiles(){
  QJsonObject settings = NETWORK->list_config();
  QStringList keys = settings.keys();
  QString curkey = ui->combo_conn_static_profile->currentText();
  ui->combo_conn_static_profile->clear();
  int index = -1;
  for(int i=0; i<keys.length(); i++){
    QJsonObject obj = settings.value(keys[i]).toObject();
    if(obj.isEmpty()){ continue; }
    ui->combo_conn_static_profile->addItem(keys[i], obj);
    if(keys[i] == curkey){ index = i; }
  }
  if(index>=0){ ui->combo_conn_static_profile->setCurrentIndex(index); }
  on_combo_conn_static_profile_currentIndexChanged(-1);
}

void mainUI::on_tool_conn_profile_apply_clicked(){
  QJsonObject out;
  for(int i=0; i<ui->combo_conn_static_profile->count(); i++){
    QJsonObject obj = ui->combo_conn_static_profile->itemData(i).toJsonObject();
    out.insert(obj.value("profile").toString(), obj);
  }
  bool ok = NETWORK->set_config(out);
  if(!ok){
    QMessageBox::warning(this, tr("Error"), tr("Could not save static IP settings!"));
  }
  QTimer::singleShot(50, this, SLOT(updateConnectionInfo()) );
}

void mainUI::on_tool_conn_new_profile_clicked(){
  QString profile = QInputDialog::getText(this, tr("New static profile"), tr("Sentinal IP address:"));
  if(profile.isEmpty()){ return; } //cancelled
  //Check if it is an IPv4 address

  //Check if it already exists as a profile

  //Add it to the profile list
  QJsonObject obj;
  obj.insert("profile", profile);
  ui->combo_conn_static_profile->addItem(profile, obj);
}

void mainUI::on_tool_conn_remove_profile_clicked(){
  int index = ui->combo_conn_static_profile->currentIndex();
  if(index>=0){ ui->combo_conn_static_profile->removeItem(index); }
}

void mainUI::on_combo_conn_static_profile_currentIndexChanged(int){
  int index = ui->combo_conn_static_profile->currentIndex();
  ui->tool_conn_remove_profile->setVisible(index>=0);
  ui->line_static_v4_address->setEnabled(index>=0);
  ui->line_static_v4_gateway->setEnabled(index>=0);
  if( index<0 ){
    //No profiles available
    ui->line_static_v4_address->clear();
    ui->line_static_v4_gateway->clear();
  }else{
    QJsonObject info = ui->combo_conn_static_profile->currentData().toJsonObject();
    ui->line_static_v4_address->setText(info.value("ip_address").toString());
    ui->line_static_v4_gateway->setText(info.value("routers").toString());
  }
}

void mainUI::on_line_static_v4_address_textEdited(const QString &text){
  int index = ui->combo_conn_static_profile->currentIndex();
  if(index < 0){ return; } //nothing to do
  QJsonObject info = ui->combo_conn_static_profile->currentData().toJsonObject();
    info.insert("ip_address", text);
  ui->combo_conn_static_profile->setItemData(index, info);
}

void mainUI::on_line_static_v4_gateway_textEdited(const QString &text){
  int index = ui->combo_conn_static_profile->currentIndex();
  if(index < 0){ return; } //nothing to do
  QJsonObject info = ui->combo_conn_static_profile->currentData().toJsonObject();
    info.insert("routers", text);
  ui->combo_conn_static_profile->setItemData(index, info);
}

void mainUI::rescan_current_dns(){
  ui->text_current_dns->setPlainText( NETWORK->current_dns() );
}

void mainUI::update_dns_settings(){
  QJsonObject current = NETWORK->custom_dns_settings();
  //qDebug() << "Custom DNS:" << current;
  ui->list_dns_custom->clear();
  QJsonArray tmp = current.value("before-auto").toArray();
  for(int i=0; i<tmp.count(); i++){
    ui->list_dns_custom->addItem(tmp[i].toString());
  }
  ui->list_dns_custom->addItem(QString("-- ")+tr("Automatic")+" --");
  ui->list_dns_custom->item( ui->list_dns_custom->count()-1)->setWhatsThis("auto");
  tmp = current.value("after-auto").toArray();
  for(int i=0; i<tmp.count(); i++){
    ui->list_dns_custom->addItem(tmp[i].toString());
  }
  on_list_dns_custom_currentRowChanged(0);
}

void mainUI::apply_dns_settings(){
  QJsonArray pre, post;
  bool pre_auto = true;
  for(int i=0; i<ui->list_dns_custom->count(); i++){
    QListWidgetItem *it = ui->list_dns_custom->item(i);
    if(!it->whatsThis().isEmpty()){ pre_auto = false; }
    else if(pre_auto){ pre << it->text(); }
    else{ post << it->text(); }
  }
  QJsonObject obj;
  obj.insert("before-auto", pre);
  obj.insert("after-auto", post);
  //qDebug() << "Apply DNS settings:" << obj;
  bool ok = NETWORK->save_custom_dns_settings(obj);
  if(!ok){
    QMessageBox::warning(this, tr("Error"), tr("Could not save custom DNS entries"));
  }else{
    ui->tabWidget_dns->setCurrentIndex(0);
    QTimer::singleShot(1000, this, SLOT(rescan_current_dns()) );
  }
}

void mainUI::on_tool_dns_add_clicked(){
  QString dns = QInputDialog::getText(this, tr("Add Custom DNS"), tr("DNS server address:"));
  if(dns.isEmpty()){ return; }
  ui->list_dns_custom->insertItem(0, dns);
  ui->list_dns_custom->setCurrentRow(0); //keep the same item selected;
}

void mainUI::on_tool_dns_down_clicked(){
  int row = ui->list_dns_custom->currentRow();
  if(row>= (ui->list_dns_custom->count()-1)){ return; } //nothing to do
  ui->list_dns_custom->insertItem(row+1, ui->list_dns_custom->takeItem(row));
  ui->list_dns_custom->setCurrentRow(row+1); //keep the same item selected;
}

void mainUI::on_tool_dns_remove_clicked(){
  int row = ui->list_dns_custom->currentRow();
  if(row<0 || !ui->list_dns_custom->currentItem()->whatsThis().isEmpty()){ return; } //nothing to do
  delete ui->list_dns_custom->takeItem(row);
}

void mainUI::on_tool_dns_up_clicked(){
  int row = ui->list_dns_custom->currentRow();
  if(row<=0){ return; } //nothing to do
  ui->list_dns_custom->insertItem(row-1, ui->list_dns_custom->takeItem(row));
  ui->list_dns_custom->setCurrentRow(row-1); //keep the same item selected;
}

void mainUI::on_list_dns_custom_currentRowChanged(int){
  QListWidgetItem *it = ui->list_dns_custom->currentItem();
  int crow = ui->list_dns_custom->currentRow();
  ui->tool_dns_remove->setEnabled(it != 0 && it->whatsThis().isEmpty());
  ui->tool_dns_up->setEnabled(it!=0 && crow>0);
  ui->tool_dns_down->setEnabled(it!=0 && crow < (ui->list_dns_custom->count()-1) );
}

void mainUI::refresh_current_wireguard(){
  static QJsonObject prev_info;
  QJsonObject tmp = NETWORK->current_wireguard_profiles();
  QStringList keys = tmp.keys();
  if(tmp != prev_info){
    QString curitem;
    if(ui->tree_wireguard->currentItem()!=0){
      curitem = ui->tree_wireguard->currentItem()->text(1);
    }
    ui->tree_wireguard->clear();
    for(int i=0; i<keys.length(); i++){
      QJsonObject obj = tmp.value(keys[i]).toObject();
      bool running = obj.value("is_running").toBool();
      QTreeWidgetItem *it = new QTreeWidgetItem();
        it->setText(0, running ? tr("Active") : tr("Disabled"));
        it->setIcon(0, QIcon::fromTheme(running ? "system-run" : "cancel"));
        it->setText(1, keys[i]);
        it->setData(0, Qt::UserRole, obj);
      ui->tree_wireguard->addTopLevelItem(it);
      if(keys[i] == curitem){
        ui->tree_wireguard->setCurrentItem(it);
      }
    }
    if(prev_info.isEmpty()){ ui->tree_wireguard->sortItems(1, Qt::AscendingOrder); }
    prev_info = tmp;
    ui->tree_wireguard->resizeColumnToContents(0);
  }else{
    //no new info - see if we are waiting for a status change
    for(int i=0; i<ui->tree_wireguard->topLevelItemCount(); i++){
      if( !ui->tree_wireguard->topLevelItem(i)->data(1, Qt::UserRole).toString().isEmpty()){
         QTimer::singleShot(500, this, SLOT(refresh_current_wireguard()));
        break;
      }
    }
  }
  on_tree_wireguard_itemSelectionChanged();
}

void mainUI::on_tool_wg_add_clicked(){
  QStringList list = QFileDialog::getOpenFileNames(this, tr("Import WireGuard Configurations"), QDir::homePath(), "Configuration files (*.conf)");
  for(int i=0; i<list.length(); i++){
    bool ok = NETWORK->add_wireguard_profile(list[i]);
    if(!ok){
      QMessageBox::warning(this, tr("Error"), tr("Could not import Wireguard configuration file:")+"/n"+list[i].section("/",-1) );
    }
  }
  QTimer::singleShot(500, this, SLOT(refresh_current_wireguard()));
}

void mainUI::on_tool_wg_remove_clicked(){
  QTreeWidgetItem *it = ui->tree_wireguard->currentItem();
  if(it==0){ return; }
  QJsonObject info = it->data(0, Qt::UserRole).toJsonObject();
  NETWORK->remove_wireguard_profile(info.value("profile").toString());
  QTimer::singleShot(500, this, SLOT(refresh_current_wireguard()));
}

void mainUI::on_tool_wg_start_clicked(){
  QTreeWidgetItem *it = ui->tree_wireguard->currentItem();
  if(it==0){ return; }
  QJsonObject info = it->data(0, Qt::UserRole).toJsonObject();
  bool ok = NETWORK->start_wireguard_profile(info.value("profile").toString());
  //qDebug() << "Started WG profile" << ok;
  if(ok){
    it->setText(0, tr("Starting"));
    it->setData(1, Qt::UserRole, "pending");
    ui->tree_wireguard->resizeColumnToContents(0);
  }
  on_tree_wireguard_itemSelectionChanged();
  QTimer::singleShot(100, this, SLOT(refresh_current_wireguard()));
}

void mainUI::on_tool_wg_stop_clicked(){
  QTreeWidgetItem *it = ui->tree_wireguard->currentItem();
  if(it==0){ return; }
  QJsonObject info = it->data(0, Qt::UserRole).toJsonObject();
  bool ok = NETWORK->stop_wireguard_profile(info.value("profile").toString());
  if(ok){
    it->setText(0, tr("Stopping"));
    it->setData(1, Qt::UserRole, "pending");
    ui->tree_wireguard->resizeColumnToContents(0);
  }
  //qDebug() << "Stopped WG profile" << ok;
  on_tree_wireguard_itemSelectionChanged();
  QTimer::singleShot(100, this, SLOT(refresh_current_wireguard()));
}

void mainUI::on_tree_wireguard_itemSelectionChanged(){
  QTreeWidgetItem *it = ui->tree_wireguard->currentItem();
  bool running = false;
  bool pending = false;
  if(it!=0){
    running = it->data(0, Qt::UserRole).toJsonObject().value("is_running").toBool(false);
    pending = !it->data(1, Qt::UserRole).toString().isEmpty();;
  }
  ui->tool_wg_remove->setEnabled(it!=0 && !running && !pending);
  ui->tool_wg_start->setEnabled(it!=0 && !running && !pending);
  ui->tool_wg_stop->setEnabled(it!=0 && running && !pending);
}

// Firewall Page
void mainUI::refresh_current_firewall(){
  bool running = NETWORK->firewall_is_running();
  ui->tool_fw_start->setVisible(!running);
  ui->tool_fw_stop->setVisible(running);
  if(ui->label_fw_status->whatsThis().isEmpty()){
    ui->label_fw_status->setWhatsThis( ui->label_fw_status->text() );
  }
  ui->label_fw_status->setText( ui->label_fw_status->whatsThis().arg(running ? tr("Active") : tr("Disabled")) );
  // Now update the profile files and custom rules
  QJsonObject current = NETWORK->current_firewall_files();
  //qDebug() << "Current state:" << current;
  QString cprofile = current.value("running_profile").toString();
  ui->combo_fw_profile->setWhatsThis(cprofile); //tag the current profile in the backend
  QStringList profiles = current.value("profiles").toObject().keys();
  ui->combo_fw_profile->clear();
  for(int i=0; i<profiles.length(); i++){
    ui->combo_fw_profile->addItem(profiles[i], current.value("profiles").toObject().value(profiles[i]).toString());
    if(profiles[i] == cprofile){
      ui->combo_fw_profile->setCurrentIndex(ui->combo_fw_profile->count()-1);
    }
  }
  if(cprofile=="NONE"){
    ui->combo_fw_profile->addItem(cprofile, "/etc/nftables.conf"); //manual file?
    ui->combo_fw_profile->setCurrentIndex(ui->combo_fw_profile->count()-1);
  }
  QString crule = ui->combo_fw_rules->currentText();
  QStringList rules = current.value("custom").toObject().keys();
  ui->combo_fw_rules->clear();
  for(int i=0; i<rules.length(); i++){
    //qDebug() << "New Rule:" << rules[i] << current.value("custom").toObject().value(rules[i]);
    ui->combo_fw_rules->addItem(rules[i], current.value("custom").toObject().value(rules[i]).toString());
    if(crule == rules[i]){ ui->combo_fw_rules->setCurrentIndex(ui->combo_fw_rules->count()-1); }
  }
  on_combo_fw_profile_currentIndexChanged(0);
  on_combo_fw_rules_currentIndexChanged(0);
}

void mainUI::on_tool_fw_start_clicked(){
  if( NETWORK->start_firewall() ){
    QTimer::singleShot(50, this, SLOT(refresh_current_firewall()) );
  }else{
    //Firewall could not be started
    QMessageBox::warning(this, tr("Error"), tr("Could not start firewall!"));
  }
}

void mainUI::on_tool_fw_stop_clicked(){
  if( NETWORK->stop_firewall() ){
    QTimer::singleShot(50, this, SLOT(refresh_current_firewall()) );
  }else{
    //Firewall could not be started
    QMessageBox::warning(this, tr("Error"), tr("Could not stop firewall!"));
  }
}

void mainUI::on_combo_fw_profile_currentIndexChanged(int){
  QString profile = ui->combo_fw_profile->currentText();
  QString oldprofile = ui->combo_fw_profile->whatsThis();
  if(!oldprofile.isEmpty() && !profile.isEmpty() && profile != oldprofile){
    //Profile changed - save the change in the backend
    NETWORK->change_firewall_profile(ui->combo_fw_profile->currentData().toString());
    QTimer::singleShot(50, this, SLOT(refresh_current_firewall()));
    return;
  }
  if(!profile.isEmpty()){ ui->combo_fw_profile->setWhatsThis(profile); }//currently-used profile name
  ui->group_fw_rules->setEnabled(profile!="NONE");
}

void mainUI::on_combo_fw_rules_currentIndexChanged(int){
  QString path = ui->combo_fw_rules->currentData().toString();
  bool ok = !path.isEmpty();
  ui->tool_fw_applyrule->setEnabled(ok);
  ui->tool_fw_rmrule->setEnabled(ok);
  ui->tool_fw_shortcuts->setEnabled(ok);
  if(ok){
    ui->text_fw_rule->setText( Networking::readFile(path).join("\n") );
  }else{
    ui->text_fw_rule->setText("");
  }
}

void mainUI::on_tool_fw_addrule_clicked(){
  //Prompt for the new rule name
  QString profile = QInputDialog::getText(this, tr("New Firewall Rules"), tr("Profile Name:") );
  QString path = "/etc/firewall-conf/custom-"+profile+".conf";
  //Make sure this profile does not already exist
  if(QFile::exists(path)){
    QMessageBox::warning(this, tr("Error"), tr("Profile already exists"));
    return;
  }
  //Add the rule into the list and pre-select it
  ui->combo_fw_rules->addItem(profile, path);
  ui->combo_fw_rules->setCurrentIndex(ui->combo_fw_rules->count()-1);
}

void mainUI::on_tool_fw_applyrule_clicked(){
  QString path = ui->combo_fw_rules->currentData().toString();
  if(path.isEmpty()){ return; }
  QStringList contents = ui->text_fw_rule->toPlainText().split("\n");
  if( !NETWORK->save_firewall_rules(path, contents) ){
   QMessageBox::warning(this, tr("Error"), tr("Could not update firewall rules:")+"\n\n"+ui->combo_fw_rules->currentText());
  }
  QTimer::singleShot(50, this, SLOT(refresh_current_firewall()));
}

void mainUI::on_tool_fw_rmrule_clicked(){
  QString path = ui->combo_fw_rules->currentData().toString();
  if(path.isEmpty()){ return; }
  //qDebug() << "Remove Firewall Rules:" << path;
  if( !NETWORK->remove_firewall_rules(path) ){
   QMessageBox::warning(this, tr("Error"), tr("Could not remove firewall rules:")+"\n\n"+ui->combo_fw_rules->currentText());
  }
  QTimer::singleShot(50, this, SLOT(refresh_current_firewall()));
}

void mainUI::open_fw_port(){
  int port = QInputDialog::getInt(this, tr("Open Firewall Port"), tr("Port Number:"));
  if(port<0){ return; } //cancelled
  QString line = "add rule inet filter %1 %2 dport %3 accept";
  QStringList newLines;
    newLines << line.arg("input", "tcp", QString::number(port));
    newLines << line.arg("output", "tcp", QString::number(port));
    newLines << line.arg("input", "udp", QString::number(port));
    newLines << line.arg("output", "udp", QString::number(port));
  ui->text_fw_rule->append("\n"+newLines.join("\n"));
}

void mainUI::open_fw_service(){
  QJsonObject svcs = NETWORK->known_services();
  QString service = QInputDialog::getItem(this, tr("Open Firewall for Service"), tr("Select a service:"), svcs.keys(), 0, false);
  if(service.isEmpty()){ return; } //cancelled
  QJsonArray info = svcs.value(service).toArray();
  //qDebug() << "Service Info:" << service << info;
  bool tcp, udp;
  for(int i=0; i<info.count(); i++){
    if(info[i].toString().endsWith("/tcp")){ tcp = true; }
    else if(info[i].toString().endsWith("/udp")){ udp = true; }
  }
  QString line = "add rule inet filter %1 %2 dport %3 accept";
  QStringList newLines;
  if(tcp){
    newLines << line.arg("input", "tcp", service);
    newLines << line.arg("output", "tcp", service);
  }
  if(udp){
    newLines << line.arg("input", "udp", service);
    newLines << line.arg("output", "udp", service);
  }
  ui->text_fw_rule->append("\n"+newLines.join("\n"));
}
