#include <network.h>
#include <QDebug>
#include <QNetworkInterface>
#include <QtConcurrent>

static QRegExp ip4regex = QRegExp("([0-9]{1,3}\\.){3}[0-9]{1,3}");
static QRegExp bssidRegex = QRegExp("([^:]{2}:){5}[^:]{2}");
static QString DHCPConf = "/etc/dhcpcd.conf";
QStringList wifidevs;

Networking::Networking(QObject *parent) : QObject(parent){
  NETMAN = new QNetworkConfigurationManager(this);
  qSudoProc = 0; // uninitialized yet
}

Networking::~Networking(){

}

QStringList Networking::list_devices(){
  QList<QNetworkConfiguration> configs = NETMAN->allConfigurations();
  //qDebug() << "isOnline:" << NETMAN->isOnline();
  QStringList devs;
  QStringList newwifidevs;
  for(int i=0; i<configs.length(); i++){
    //qDebug() << "config:" << configs[i].identifier() << configs[i].bearerTypeName() << configs[i].name() << configs[i].state();
    devs << configs[i].name();
    if(configs[i].name().startsWith("wl")){
      newwifidevs << configs[i].name(); //save this for scanning later
    }
  }
  wifidevs = newwifidevs; //save this list for later
  return devs;
}

QJsonObject Networking::list_config(){
  QStringList lines = readFile(DHCPConf);
  // Need to read /etc/dhcpcd.conf and pull out all profile entries
  // Example entry
  // arping 192.168.0.1
  // profile 192.168.0.1
  // static ip_address=192.168.0.10/24
  // static routers=192.168.0.1
  bool inblock = false;

  QJsonObject out;
  QJsonObject obj;
  for(int i=0; i<lines.length(); i++){
    if(lines[i].startsWith("arping ")){
      QJsonArray checks = out.value("pings").toArray();
      QStringList tmp = lines[i].section(" ",1,-1).split(" ", QString::SkipEmptyParts);
      for(int j=0; j<tmp.length(); j++){ checks << tmp[j]; }
      out.insert("pings", checks);
    }else if(lines[i].startsWith("profile ") ){
        //Starting a profile block
        inblock = true;
        obj.insert("profile", lines[i].section(" ",1,1));
    }else if(inblock){
      if(lines[i].startsWith("#") || lines[i].simplified().isEmpty()){
        //End of profile block - save it to the output object
        inblock = false;
        out.insert(obj.value("profile").toString(), obj);
        obj = QJsonObject();
      }else if(lines[i].startsWith("static ") ){
        //value within a profile block
        QString key = lines[i].section(" ",1,-1).section("=",0,0).simplified();
        obj.insert(key, lines[i].section("=",1,-1).simplified());
      }
    }
  }
  if(!obj.isEmpty()){
    //Last item might not reach the end tag. Add it in if not empty at this point
    out.insert(obj.value("profile").toString(), obj);
  }
  //qDebug() << "Read Config:" << out << obj;
  return out;
}

QJsonObject Networking::current_info(QString device){
  QNetworkInterface config = QNetworkInterface::interfaceFromName(device);
  QJsonObject obj;
  if(!config.isValid()){ return obj; }
  obj.insert("hardware_address", config.hardwareAddress());
  obj.insert("is_wifi", config.type() == QNetworkInterface::Wifi || device.startsWith("wl"));
  obj.insert("is_up", config.flags().testFlag(QNetworkInterface::IsUp));
  obj.insert("is_running", config.flags().testFlag(QNetworkInterface::IsRunning));
  obj.insert("can_broadcast", config.flags().testFlag(QNetworkInterface::CanBroadcast));
  obj.insert("is_loopback", config.flags().testFlag(QNetworkInterface::IsLoopBack));
  obj.insert("can_multicast", config.flags().testFlag(QNetworkInterface::CanMulticast));
  obj.insert("is_pt2pt", config.flags().testFlag(QNetworkInterface::IsPointToPoint));
  QList<QNetworkAddressEntry> addresses = config.addressEntries();
  bool ok = false;
  for(int i=0; i<addresses.length(); i++){
    addresses[i].ip().toIPv4Address(&ok);
    if(ok){
      obj.insert("ipv4", addresses[i].ip().toString());
      obj.insert("ipv4_netmask", addresses[i].netmask().toString());
      obj.insert("ipv4_broadcast", addresses[i].broadcast().toString());
      //obj.insert("ipv4_gateway", addresses[i].gateway().toString());
    }else{
      obj.insert("ipv6", addresses[i].ip().toString());
      obj.insert("ipv6_netmask", addresses[i].netmask().toString());
      obj.insert("ipv6_broadcast", addresses[i].broadcast().toString());
     // obj.insert("ipv6_gateway", addresses[i].gateway().toString());
    }
  }
  //qDebug() << "Device Info:" << device << obj;
  bool active_connection = obj.value("is_up").toBool(false) && !obj.value("ipv4").toString().isEmpty();
  obj.insert("is_active", active_connection);
  //Now do any wifi-specific checks
  if( obj.value("is_wifi").toBool() && obj.value("is_up").toBool() ){
    QJsonObject connection;
    QStringList tmp = CmdOutput("wpa_cli", QStringList() << "-i" << device << "status").split("\n");
    for(int i=0; i<tmp.length(); i++){
      if( !tmp[i].contains("=")){ continue; }
      connection.insert( tmp[i].section("=",0,0), tmp[i].section("=",1,-1) );
    }
    //qDebug() << "wifi Status\n" << tmp;
    obj.insert("wifi", connection);
    //qDebug() << " - Parsed:" << connection;
  }else if (obj.value("is_up").toBool() ){
    //Wired connection
    QJsonObject connection;
/*    QString tmp = CmdOutput("ifconfig", QStringList() << device);
    //qDebug() << "wifi Status\n" << tmp;
    if(!tmp.isEmpty()){
      connection.insert("media", tmp.section("\tmedia: ",-1).section("\n",0,0).simplified());
    }*/
    obj.insert("lan", connection);
  }
  return obj;
}

bool Networking::set_config(QJsonObject config){
  //qDebug() << "set Config:" << config;
  // Example entry
  // arping 192.168.0.1 192.168.1.1
  // profile 192.168.0.1
  // static ip_address=192.168.0.10/24
  // static routers=192.168.0.1
  //
  // profile 192.168.1.1
  // static ip_address=192.168.1.10/24
  // static routers=192.168.1.1
  QStringList contents = readFile(DHCPConf);
  int startindex = -1;
  for(int i=0; i<contents.length(); i++){
    if(contents[i].startsWith("# -- Trident-networkmgr config below --")){
      startindex = i-1; break; //make sure we "start" one line above this
    }
  }
  bool changed = false;
  if(startindex>=0){
    changed = true; //have to delete entries from the end of the file
    // Delete the end of the file as needed - gets replaced in a moment
    for(int i=contents.length()-1; i>startindex; i--){ contents.removeAt(i); }
  }
  if(!config.keys().isEmpty()){
    changed = true; //have to add entries to the bottom of the file
    contents << "# -- Trident-networkmgr config below --";
    contents << "# -- Place all manual changes above this --";
    QStringList pings = config.keys();
    contents << "arping "+pings.join(" ");
    for(int i=0; i<pings.length(); i++){
      QJsonObject profile = config.value(pings[i]).toObject();
      if(profile.isEmpty() || !profile.contains("profile") ){ continue; }
      contents << "";
      contents << "profile " +profile.value("profile").toString();
      if(profile.contains("ip_address")){ contents << "static ip_address="+profile.value("ip_address").toString(); }
      if(profile.contains("routers")){ contents << "static routers="+profile.value("routers").toString(); }
    }
  }
  if(!changed){ return true; } //nothing to do.
  if(contents.last() != ""){ contents << ""; } //always leave a blank line at the end
  return writeFileAsRoot(DHCPConf, contents, QStringList() << "dhcpcd" << "--rebind", "644");
}

Networking::State Networking::deviceState(QString device){
  QNetworkInterface config = QNetworkInterface::interfaceFromName(device);
  if(!config.isValid()){ return StateUnknown; }
  if( config.flags().testFlag(QNetworkInterface::IsUp) ){ return StateRunning; }
  else{ return StateStopped; }
}

QJsonObject Networking::wifi_scan_results(){
  return last_wifi_scan;
}

QJsonArray Networking::known_wifi_networks(){
  static QJsonArray idcache;
  static QDateTime cachecheck;
  QDateTime lastMod = QFileInfo("/etc/wpa_supplicant/wpa_supplicant.conf").lastModified();
  if(cachecheck.isNull() || (lastMod > cachecheck) ){
    //Need to re-read the file to assemble the list of ID's.
    idcache = QJsonArray();
    QStringList contents = CmdOutput("wpa_cli", QStringList() << "-i" << wifidevs[0] << "list_networks").split("\n");
    cachecheck = QDateTime::currentDateTime();
    //qDebug() << "WPA Contents:" <<  contents;
    for(int i=0; i<contents.length(); i++){
      QStringList elems = contents[i].split("\t");
      // Elements: [ network id, ssid, bssid, flags]
      if(elems.length() < 3){ continue; } //not a valid entry line
      QJsonObject obj;
        obj.insert("id", elems[0]);
        obj.insert("ssid", elems[1]);
        if(elems[2] != "any"){ obj.insert("bssid", elems[2]); }
      idcache << obj;
    }
    //qDebug() << "Known wifi networks:" << idcache;
  }

  return idcache;
}

QJsonObject Networking::active_wifi_network(){
  QJsonObject obj;
  QStringList info = CmdOutput("wpa_cli", QStringList() << "-i" << wifidevs[0] << "status").split("\n");
  for(int i=0; i<info.length(); i++){
    QString key = info[i].section("=",0,0);
    if(key == "id" || key == "bssid" || key == "ssid"){
      obj.insert(key, info[i].section("=",1,-1));
    }
  }
  return obj;
}

bool Networking::is_known(QJsonObject obj){
  QJsonArray known = known_wifi_networks();
  for(int i=0; i<known.count(); i++){
    if( sameNetwork(known[i].toObject(), obj)){ return true; }
  }
  return false;
}

bool Networking::save_wifi_network(QJsonObject obj, bool clearonly){
  QString id = obj.value("id").toString();
  QString ssid = obj.value("ssid").toString();
  QString bssid = obj.value("bssid").toString();
  QString password = obj.value("psk").toString();
  if(ssid.isEmpty() && bssid.isEmpty() && !clearonly){ return false; }
  else if(id.isEmpty() && clearonly){ return false; }
  if(clearonly){ qDebug() << "Remove Wifi Network:" << ssid; }
  else{ qDebug() << "Save Wifi Network:" << ssid; }
  if(clearonly) {
    CmdReturn("wpa_cli", QStringList() << "-i" << wifidevs[0] << "remove_network" << id);
  }else{
    if(id.isEmpty()){
      //Need to get a new ID
      QStringList val = CmdOutput("wpa_cli", QStringList() << "-i" << wifidevs[0] << "add_network").split("\n");
      for(int i=val.length()-1; i>=0; i-- ){
        if(val[i].simplified().isEmpty()){ continue; }
        else if(val[i].simplified().toInt() >= 0){ id = val[i].simplified(); break; }
      }
      if(id.isEmpty()){ return false; }
      //qDebug() << "New network ID:" << id << "raw:" << val;
    }
    if(!ssid.isEmpty()){ CmdReturn("wpa_cli", QStringList() << "-i" << wifidevs[0] << "set_network" << id << "ssid" << "\""+ssid+"\""); }
    if(!bssid.isEmpty()){ CmdReturn("wpa_cli", QStringList() << "-i" << wifidevs[0] << "set_network" << id << "bssid" << bssid); }
    if(!password.isEmpty()){ CmdReturn("wpa_cli", QStringList() << "-i" << wifidevs[0] << "set_network" << id << "psk" << "\""+password+"\""); }
    else{ CmdReturn("wpa_cli", QStringList() << "-i" << wifidevs[0] << "set_network" << id << "key_mgmt" << "NONE"); }
  }
  CmdReturn("wpa_cli", QStringList() << "-i" << wifidevs[0] << "save_config");
  bool ok = false;
  if(clearonly){ ok = !is_known(obj); }
  else { ok = is_known(obj); }
  if(!clearonly && ok){ connect_to_wifi_network(obj); }
  return ok;
}

bool Networking::remove_wifi_network(QString id){
  //Note - it is better to call the save_wifi_network with the clear flag, can filter by ssid AND bssid in one pass
  QJsonObject obj;
  obj.insert("id", id);
  return save_wifi_network(obj, true); //remove only
}

bool Networking::connect_to_wifi_network(QJsonObject info, bool noretry){
  QString id = knownNetworkID(info);
  //The "id" here needs to be the integer number, not the ssid name
  //qDebug() << "Connect to network:" << id << info;
  bool ok = CmdOutput("wpa_cli", QStringList() << "-i" << wifidevs[0] << "select_network" << id).contains("OK");
  if(!ok && !noretry){
    CmdReturn("wpa_cli", QStringList() << "-i" << wifidevs[0] << "reconfigure"); //poke wpa to start attempting again
    QThread::sleep(1);
    ok = connect_to_wifi_network(info, true); //do not re-try again
  }
  if(ok){ CmdReturn("wpa_cli", QStringList() << "-i" << wifidevs[0] << "save_config"); } //go ahead and update config
  return ok;
}


// DNS specific functionality
QString Networking::current_dns(){
  return readFile("/etc/resolv.conf").join("\n");
}

QJsonObject Networking::custom_dns_settings(){
  QJsonArray pre, post;
  QStringList contents = readFile("/etc/resolvconf.conf");
  for(int i=0; i<contents.length(); i++){
    if(contents[i].startsWith("name_servers=")){
      QString val = contents[i].section("=",1,-1);
      if(val.contains("\"")){ val.remove("\""); }
      QStringList tmp = val.split(" ", QString::SkipEmptyParts);
      for(int j=0; j<tmp.length(); j++){ pre << tmp[j]; }
    }else if(contents[i].startsWith("name_servers_append=")){
      QString val = contents[i].section("=",1,-1);
      if(val.contains("\"")){ val.remove("\""); }
      QStringList tmp = val.split(" ", QString::SkipEmptyParts);
      for(int j=0; j<tmp.length(); j++){ post << tmp[j]; }
    }
  }
  QJsonObject out;
  out.insert("before-auto", pre);
  out.insert("after-auto", post);
  return out;
}

inline QStringList jsonArrayToStringList(QJsonArray arr){
  QStringList out;
  for(int i=0; i<arr.count(); i++){ out << arr[i].toString(); }
  out.removeDuplicates();
  return out;
}

bool Networking::save_custom_dns_settings(QJsonObject obj){
  QJsonArray pre = obj.value("before-auto").toArray();
  QJsonArray post = obj.value("after-auto").toArray();
  QStringList contents = readFile("/etc/resolvconf.conf");
  bool changed = false;
  for(int i=0; i<contents.length(); i++){
    if(contents[i].startsWith("name_servers")){
      changed = true;
      contents.removeAt(i);
      i--;
    }
  }
  if(!pre.isEmpty()){
    changed = true;
    contents << "name_servers=\""+ jsonArrayToStringList(pre).join(" ")+"\"";
  }
  if(!post.isEmpty()){
    changed = true;
    contents << "name_servers_append=\""+ jsonArrayToStringList(post).join(" ")+"\"";
  }
  //qDebug() << "Save custom DNS settings:" << changed << contents;
  if(!changed){ return true; } //nothing to do
  bool ok = writeFileAsRoot("/etc/resolvconf.conf", contents, QStringList() << "resolvconf" << "-u", "744");
  return ok;
}


// Wireguard specific functionality
QJsonObject Networking::current_wireguard_profiles(){
  QStringList netdevs = list_devices(); //current network devices
  // (wireguard profiles show up in interface list here when running)
  QJsonObject out;
  QDir dir("/etc/wireguard");
  QStringList files = dir.entryList( QStringList() << "*.conf", QDir::Files, QDir::Name);
  if(files.isEmpty() && !dir.isReadable()){
    //The dir is probaby not readable right now - make it so (files inside are still unreadable/secure)
    bool ok = CmdReturn("qsudo", QStringList() << "chmod" << "755" << "/etc/wireguard", qSudoProc);
    if(ok){ files = dir.entryList( QStringList() << "*.conf", QDir::Files, QDir::Name); } //try again
  }
  for(int i=0; i<files.length(); i++){
    QJsonObject tmp;
    QString profile = files[i].section(".conf",0,-2).simplified();
    tmp.insert("profile", profile);
    tmp.insert("path", dir.absoluteFilePath(files[i]));
    tmp.insert("is_running", netdevs.contains(profile));
    out.insert(profile, tmp);
  }
  return out;
}

bool Networking::add_wireguard_profile(QString curpath){
  //Now write the temporary script to swap over the file (single qsudo request)
  bool mkdir = QFile::exists("/etc/wireguard");
  QString path = "/etc/wireguard/"+curpath.section("/",-1);
  QStringList script;
  script << "#!/bin/bash";
  if(mkdir){
    script << "mkdir /etc/wireguard";
    script << "chmod 755 /etc/wireguard";
  }
  script << "mv -f \""+curpath+"\" \""+path+"\"";
  script << "chown root:root \""+path+"\"";
  script << "chmod 700 \""+path+"\"";
  QString tmpscript = "/tmp/.update-"+path.section("/",-1).section(".",0,0)+".sh";
  bool ok = writeFile(tmpscript, script);
  QFile::setPermissions(tmpscript, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner \
		|  QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ExeGroup \
		| QFileDevice::ReadOther | QFileDevice::WriteOther | QFileDevice::ExeOther);

  ok = CmdReturn("qsudo", QStringList() << tmpscript, qSudoProc);
  QFile::remove(tmpscript);
  return ok;
}

bool Networking::remove_wireguard_profile(QString name){
  return CmdReturn("qsudo", QStringList() << "rm" << "/etc/wireguard/"+name+".conf", qSudoProc);
}

bool Networking::start_wireguard_profile(QString name){
  //qDebug() << "Start WG:" << name;
  return CmdReturn("qsudo", QStringList() << "wg-quick" << "up" << name, qSudoProc);
}

bool Networking::stop_wireguard_profile(QString name){
  //qDebug() << "Stop WG:" << name;
  return CmdReturn("qsudo", QStringList() << "wg-quick" << "down" << name, qSudoProc);
}

// Firewall functionality
bool Networking::firewall_is_running(){
  return CmdReturn("pgrep", QStringList() << "-fx" << "nftables");
}

bool Networking::start_firewall(){
return CmdReturn("qsudo", QStringList() << "sv" << "start" << "nftables", qSudoProc);
}

bool Networking::stop_firewall(){
return CmdReturn("qsudo", QStringList() << "sv" << "stop" << "nftables", qSudoProc);
}

QJsonObject Networking::current_firewall_files(){
  QJsonObject out;
  //First figure out what profile is currently running
  QFileInfo finfo("/etc/nftables.conf");
  if(finfo.isSymLink()){
    QString file = finfo.symLinkTarget();
    out.insert("running_profile", file.section("/",-1).section(".",0,-2) );
    out.insert("running_profile_file", file);
  }else{
    out.insert("running_profile", "NONE");
  }
  //Now read through the available profile/custom files
  QDir dir("/etc/firewall-conf");
  QStringList files = dir.entryList(QStringList() << "*.conf", QDir::Files, QDir::Name);
  QJsonObject profiles, custom;
  for(int i=0; i<files.length(); i++){
    QString abspath = dir.absoluteFilePath(files[i]);
    //qDebug() << "File:" << files[i] << abspath;
    if(files[i].startsWith("custom-")){
      custom.insert(files[i].section("-",1,-1).section(".",0,-2), abspath);
    }else{
      profiles.insert(files[i].section(".",0,-2), abspath);
    }
  }
  out.insert("profiles", profiles);
  out.insert("custom", custom);
  return out;
}

bool Networking::change_firewall_profile(QString path){
  QStringList script;
  script << "ln -sf \""+path+"\" /etc/nftables.conf";
  script << "sv restart nftables";
  return runScriptAsRoot(script, "config-nftables");
}

bool Networking::save_firewall_rules(QString path, QStringList contents){
  return writeFileAsRoot(path, contents, QStringList() << "sv" << "restart" << "nftables", "644");
}

bool Networking::remove_firewall_rules(QString path){
  if(!QFile::exists(path)){ return true; } //does not exist in the first place
  if(!QFileInfo(path).canonicalFilePath().startsWith("/etc/firewall-conf/")){ qDebug() << "Canonical Path:" << QFileInfo(path).canonicalPath(); return false; }
  QStringList script;
  script << "rm -f \""+path+"\"";
  script << "sv restart nftables";
  return runScriptAsRoot(script, "config-nftables");
}

QJsonObject Networking::known_services(){
  static QJsonObject known;
  if(known.isEmpty()){
    QStringList contents = readFile("/etc/services");
    for(int i=0; i<contents.length(); i++){
      QStringList info = contents[i].split(" ",QString::SkipEmptyParts);
      if(info.length() != 2){ continue; }
      known.insert(info[0], known.value(info[0]).toArray() << info[1]);
    }
  }
  return known;
}

//General Purpose functions
QStringList Networking::readFile(QString path){
  QFile file(path);
  QStringList contents;
  if(file.open(QIODevice::ReadOnly)){
    QTextStream in(&file);
    contents = in.readAll().split("\n");
    file.close();
  }else{
    qDebug() << "Could not read file:" << path;
  }
  return contents;
}

bool Networking::writeFile(QString path, QStringList contents){
  QString newpath = path+".new";
  //Make sure the parent directory exists first
  QDir dir; dir.mkpath(path.section("/",0,-2));
  //Write the file to a new location
  QFile file(newpath);
  if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate)){ return false; } //could not open the file
  if(!contents.isEmpty()){
    QTextStream out(&file);
    out << contents.join("\n");
    //Most system config files need to end with a newline to be valid
    if(!contents.last().endsWith("\n")){ out << "\n"; }
  }
  file.close();
  //Now replace the original file
  if(QFile::exists(path)){ QFile::remove(path); }
  return QFile::rename(newpath, path); //now do a simple rename of the file.
}

bool Networking::sameNetwork(QJsonObject A, QJsonObject B){
  if(A.contains("bssid") && B.contains("bssid")){
    return (A.value("bssid") == B.value("bssid"));
  }else if(A.contains("ssid") && B.contains("ssid")){
    return (A.value("ssid") == B.value("ssid"));
  }
  return false;
}

bool Networking::writeFileAsRoot(QString path, QStringList contents, QStringList loadCmd, QString perms){
  QString tmppath = "/tmp/."+path.section("/",-1);
  bool ok = writeFile(tmppath, contents);
  if(!ok){ return false; } //could not write the temp file
  //Now write the temporary script to swap over the files (single qsudo request)
  bool overwrite = QFile::exists(path);
  QStringList script;
  script << "#!/bin/bash";
  if(overwrite){
    script << "mv -f \""+path+"\" \""+path+".old\"";
    script << "if [ $? -ne 0 ] ; then exit 1 ; fi";
  }
  script << "mv \""+tmppath+"\" \""+path+"\"";
  script << "if [ $? -ne 0 ] ; then";
  script << "  mv -f \""+path+".old\" \""+path+"\"";
  script << "  exit 1";
  script << "fi";
  script << "chown root:root \""+path+"\"";
  if(!perms.isEmpty()){ script << "chmod "+perms+" \""+path+"\""; }
  if(!loadCmd.isEmpty()){
    script << loadCmd.join(" ");
    script << "ret=$?";
    if(overwrite){
      script << "if [ ${ret} -ne 0 ] ; then";
      script << "  mv -f \""+path+".old\" \""+path+"\"";
      script << loadCmd.join(" ");
      script << "fi";
    }
    script << "exit ${ret}";
  }
  ok = runScriptAsRoot(script, "update-"+path.section("/",-1).section(".",0,0)+".sh");
  if(!ok && QFile::exists(tmppath)){ QFile::remove(tmppath); } //cleanup leftover file
  return ok;
}

bool Networking::runScriptAsRoot(QStringList script, QString scriptName){
  if(scriptName.isEmpty()){ scriptName = "update-config.sh"; }
  else if(!scriptName.endsWith(".sh")){ scriptName.append(".sh"); }
  QString tmpscript = "/tmp/."+scriptName.section("/",-1);
  if(script.first()!="#!/bin/bash"){ script.prepend("#!/bin/bash"); }
  bool ok = writeFile(tmpscript, script);
  QFile::setPermissions(tmpscript, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner \
		|  QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ExeGroup \
		| QFileDevice::ReadOther | QFileDevice::WriteOther | QFileDevice::ExeOther);

  ok = CmdReturn("qsudo", QStringList() << tmpscript, qSudoProc);
  QFile::remove(tmpscript);
  return ok;

}

QString Networking::CmdOutput(QString proc, QStringList args, QProcess *qsudoproc){
  if(proc=="qsudo"){
    if(qsudoproc == 0){ qsudoproc = new QProcess();
      qsudoproc->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    }
    qsudoproc->start(proc, args);
    qsudoproc->waitForFinished();
    return qsudoproc->readAll();
  }else{
    QProcess P;
      P.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
      P.start(proc, args);
      P.waitForFinished();
    return P.readAll();
  }
}

int Networking::CmdReturnCode(QString proc, QStringList args, QProcess *qsudoproc){
  if(proc=="qsudo"){
    if(qsudoproc == 0){
      qsudoproc = new QProcess();
      qsudoproc->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    }
    qsudoproc->start(proc, args);
    qsudoproc->waitForFinished();
    return qsudoproc->exitCode();
  }else{
    QProcess P;
      P.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
      P.start(proc, args);
      P.waitForFinished();
    return P.exitCode();
  }
}

bool Networking::CmdReturn(QString proc, QStringList args, QProcess *qsudoproc){
  int retcode = CmdReturnCode(proc,args, qsudoproc);
  return (retcode == 0);
}

//  === PRIVATE ===
void Networking::performWifiScan(QStringList wifi_devices){
  if(wifi_devices.isEmpty()){ return; }
  this->emit starting_wifi_scan();
  CmdOutput("wpa_cli", QStringList() << "-i" << wifi_devices[0] << "scan" );
  for(int i=0; i<10; i++){
    QThread::sleep(1);
    QStringList lines = CmdOutput("wpa_cli", QStringList() << "scan_results" ).split("\n");
    parseWifiScanResults(lines);
  }
  this->emit finished_wifi_scan();
}

void Networking::parseWifiScanResults(QStringList lines){
  QJsonObject out;
  for(int i=1; i<lines.length(); i++){ //first line is column headers
    if(lines[i].simplified().isEmpty()){ continue; }
    //Columns: [bssid, frequency, signal level, [flags], ssid]
    QJsonObject tmp;
    QString ssid = lines[i].section("\t",-1).simplified();
    QString bssid = lines[i].section("\t",0,0, QString::SectionSkipEmpty);
    if( !bssidRegex.exactMatch(bssid) ){ continue; }
    tmp.insert("ssid", ssid);
    tmp.insert("bssid", bssid);
    tmp.insert("freq",  lines[i].section("\t",1,1, QString::SectionSkipEmpty));
    int sig = lines[i].section("\t",2,2, QString::SectionSkipEmpty).simplified().toInt();
    tmp.insert("sig_db_level", sig);
    //Quick and dirty percent: assume -80DB for noise, and double the difference between Sig/Noise
    int sigpercent = (sig+80) *2;
    if(sigpercent<0){ sigpercent = 0; }
    else if(sigpercent>100){ sigpercent = 100; }
    if(sigpercent<10){ tmp.insert("signal", "00"+QString::number(sigpercent)+"%"); }
    else if(sigpercent<100){ tmp.insert("signal", "0"+QString::number(sigpercent)+"%"); }
    else{ tmp.insert("signal", QString::number(sigpercent)+"%"); }
    tmp.insert("sig_percent", sigpercent);
    QStringList cap = lines[i].section("[",1,-1).section("]",0,-2).split("][");
    //for(int j=6; j<columns.length(); j++){ cap << columns[j].split(" ", QString::SkipEmptyParts); }
    tmp.insert("capabilities", QJsonArray::fromStringList(cap));
    tmp.insert("is_locked", !cap.filter("WPA").isEmpty());
    if(out.contains(ssid)){
      //Convert this to an array of access points with the same ssid
      QJsonArray arr;
      if(out.value(ssid).isArray()){ arr = out.value(ssid).toArray(); }
      else { arr << out.value(ssid).toObject(); }
      arr << tmp;
      out.insert(ssid, arr);
    }else{
      out.insert(ssid, tmp); //first object with this ssid
    }
  }
  if(out != last_wifi_scan){
    last_wifi_scan = out;
    this->emit new_wifi_scan_results();
  }
}

QString Networking::knownNetworkID(QJsonObject info){
  if(info.contains("id")){ return info.value("id").toString(); }
  //Need to search for the numeric number for this network really quick
  QJsonArray nets = known_wifi_networks();
  for(int i=0; i<nets.count(); i++){
    if( sameNetwork(nets[i].toObject(), info) ){
      return nets[i].toObject().value("id").toString();
    }
  }
  return "";
}

// === PUBLIC SLOTS ===
bool Networking::setDeviceState(QString device, State stat){
  bool ok = false;
  State curstate = deviceState(device);
  if(curstate == stat){ return true; } //nothing to do
  switch(stat){
    case StateRunning:
      //Start the network device
      qDebug() << "Starting network device:" << device;
      ok = CmdReturn("qsudo", QStringList() << "ip" << "link" << "set" << device << "up", qSudoProc);
      break;
    case StateStopped:
      //Stop the network device
      qDebug() << "Stopping network device:" << device;
      ok = CmdReturn("qsudo", QStringList() << "ip" << "link" << "set" << device << "down", qSudoProc);
      break;
    case StateRestart:
      //Restart the network device
      qDebug() << "Restarting network device:" << device;
      ok = runScriptAsRoot(QStringList() << "ip link set "+device+" down" << "sleep 1" << "ip link set "+device+" up", "restart-"+device);
      break;
    case StateUnknown:
      break; //do nothing
  }
  return ok;
}

void Networking::startWifiScan(){
  //Grab the first wifi device and use that for the scan
  if(wifidevs.isEmpty()){ return; }
  QtConcurrent::run(this, &Networking::performWifiScan, wifidevs);
}
