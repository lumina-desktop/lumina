//===========================================
//  Lumina-DE source code
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
// Helpers shared by the LuminaOS-<OS>.cpp backends (only one backend is compiled, so these are header-only).
//  - LOSCommon::diskUsage(): file system numbers via statvfs (QStorageInfo) instead of parsing "df" output,
//    whose unchecked mountInfo[1] read crashed when df printed nothing (and which spawns a process).
//  - LOSLinuxBattery: battery state from /sys/class/power_supply instead of parsing "acpi -b" (out-of-bounds
//    read when the output has no "%", needs the acpi tool, spawns a process on every panel update).
//===========================================
#ifndef _LUMINA_LIBRARY_OS_COMMON_H
#define _LUMINA_LIBRARY_OS_COMMON_H

#include <QString>
#include <QStringList>
#include <QStorageInfo>
#include <QFile>
#include <QDir>

namespace LOSCommon{
  //Used/available/total bytes of the file system holding "dir" (false if it cannot be read)
  // "used" and "avail" follow df: used = total - free, avail = space available to unprivileged users
  inline bool diskUsage(const QString &dir, qint64 &used, qint64 &avail, qint64 &total, QString *mountpoint = 0){
    QStorageInfo info(dir);
    if(!info.isValid() || !info.isReady()){ return false; }
    total = info.bytesTotal();
    used = total - info.bytesFree();
    avail = info.bytesAvailable();
    if(used < 0 || avail < 0 || (used + avail) <= 0){ return false; }
    if(mountpoint!=0){ *mountpoint = info.rootPath(); }
    return true;
  }

  //Percentage used, rounded up the way df computes its Capacity/Use% column: "NN%" (empty on error)
  inline QString capacityPercent(const QString &dir){
    qint64 used, avail, total;
    if(!diskUsage(dir, used, avail, total)){ return QString(); }
    int percent = int( (used * 100 + (used + avail) - 1) / (used + avail) );
    return QString::number(percent) + "%";
  }
}

#ifdef __linux__
namespace LOSLinuxBattery{
#ifndef LOS_POWER_SUPPLY_DIR
#define LOS_POWER_SUPPLY_DIR "/sys/class/power_supply/" //(overridable so tests can use a fake tree)
#endif
  static const char *POWER_SUPPLY_DIR = LOS_POWER_SUPPLY_DIR;

  inline QString readSysValue(const QString &path){
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)){ return QString(); }
    return QString::fromLatin1(file.readAll()).trimmed();
  }

  //System batteries only: skip peripheral batteries (mice, keyboards, headsets), which report scope "Device"
  inline QStringList systemBatteryDirs(){
    QStringList out;
    QString base = QString(POWER_SUPPLY_DIR);
    QDir dir(base);
    QStringList entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::System, QDir::Name);
    for(int i=0; i<entries.length(); i++){
      QString path = base + entries[i] + "/";
      if(readSysValue(path+"type") != "Battery"){ continue; }
      if(readSysValue(path+"scope") == "Device"){ continue; }
      if(readSysValue(path+"present") == "0"){ continue; } //empty battery bay
      out << path;
    }
    return out;
  }

  inline bool hasBattery(){ return !systemBatteryDirs().isEmpty(); }

  //Percent charge (0-100), -1 on error
  inline int charge(){
    QStringList bats = systemBatteryDirs();
    if(bats.isEmpty()){ return -1; }
    //Combine multiple batteries by stored energy (or charge) where the kernel provides it
    double now = 0, full = 0;
    bool haveAbsolute = true;
    for(int i=0; i<bats.length() && haveAbsolute; i++){
      QString n = readSysValue(bats[i]+"energy_now"), f = readSysValue(bats[i]+"energy_full");
      if(n.isEmpty() || f.isEmpty()){ n = readSysValue(bats[i]+"charge_now"); f = readSysValue(bats[i]+"charge_full"); }
      if(n.isEmpty() || f.isEmpty()){ haveAbsolute = false; break; }
      now += n.toDouble(); full += f.toDouble();
    }
    int pct = -1;
    if(haveAbsolute && full > 0){
      pct = qRound(100.0 * now / full);
    }else{
      //Fall back to the average of the per-battery percentages
      int total = 0, count = 0;
      for(int i=0; i<bats.length(); i++){
        bool ok = false;
        int cap = readSysValue(bats[i]+"capacity").toInt(&ok);
        if(ok){ total += cap; count++; }
      }
      if(count > 0){ pct = qRound(double(total) / count); }
    }
    if(pct > 100){ pct = 100; } //energy_now can slightly exceed energy_full on some hardware
    if(pct < 0){ return -1; }
    return pct;
  }

  // While plugged in, batteries report "Charging", "Full", "Not charging" or "Unknown";
  // on battery power they report "Discharging". So anything that is NOT Discharging is
  // treated as charging/plugged in (same behaviour as the previous acpi-based check).
  inline bool isCharging(){
    QStringList bats = systemBatteryDirs();
    for(int i=0; i<bats.length(); i++){
      if(readSysValue(bats[i]+"status") == "Discharging"){ return false; }
    }
    return true;
  }

  //Estimated seconds until empty while discharging, -1 if unknown (charging, or the kernel gives no rate)
  inline int secondsLeft(){
    QStringList bats = systemBatteryDirs();
    if(bats.isEmpty()){ return -1; }
    bool discharging = false;
    double energy = 0, power = 0; //uWh and uW (or uAh and uA: same ratio)
    for(int i=0; i<bats.length(); i++){
      if(readSysValue(bats[i]+"status") == "Discharging"){ discharging = true; }
      //Prefer the kernel's own estimate when the driver provides one (summed over batteries)
      QString tte = readSysValue(bats[i]+"time_to_empty_now");
      if(!tte.isEmpty() && bats.length()==1){
        bool ok = false; double s = tte.toDouble(&ok);
        if(ok && s > 0 && discharging){ return int(s); }
      }
      QString e = readSysValue(bats[i]+"energy_now"), p = readSysValue(bats[i]+"power_now");
      if(e.isEmpty() || p.isEmpty()){ e = readSysValue(bats[i]+"charge_now"); p = readSysValue(bats[i]+"current_now"); }
      if(e.isEmpty() || p.isEmpty()){ continue; }
      energy += e.toDouble();
      power += qAbs(p.toDouble()); //some drivers report the discharge rate as negative
    }
    if(!discharging || power <= 0){ return -1; }
    return int(3600.0 * energy / power);
  }
}
#endif

#endif
