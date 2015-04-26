#include <Settings.h>

#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QWidget>
#include <QDir>


//TODO: add translation



bool JSonSettings::loadJsonSettings(QJsonObject &jsonObject) {
  QFile loadFile(QDir::homePath() + QStringLiteral("/.lumina/LuminaDE/lumina-search.settings"));
  if (!loadFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning("Problem to read the settings!!!");
    return false;
  }
  QByteArray jsonData = loadFile.readAll();
  loadFile.close();
  QJsonParseError parseError;
  QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
  if (parseError.error) {
    //TODO: such message could maybe in a log file
    qWarning() << "Problem to parse the setting file here:" << jsonData.mid( parseError.offset - 10, parseError.offset + 10).replace('\n', ' ');    
    qWarning() << parseError.errorString()  ;
    jsonObject = jsonDoc.object();
    return false;
  }
  jsonObject = jsonDoc.object();
  QStringList names = getSetNames(jsonObject);
  if (names.at(0) != "Default") {
  qWarning("settings incorrect, it does not contains 'Default'");
  return false;
  }
  return true;
}

bool JSonSettings::saveJsonSettings(QJsonObject &jsonObject) {
  QFile saveFile(QDir::homePath() + QStringLiteral("/.lumina/LuminaDE/lumina-search.settings"));
  if (!saveFile.open(QIODevice::WriteOnly)) {
    qWarning() << "Problem to save the settings in:" << QDir::homePath() + QStringLiteral(".lumina/LuminaDE/lumina-search.settings");
    return false;
  }
  QJsonDocument saveDoc(jsonObject);
  saveFile.write(saveDoc.toJson());
  saveFile.close();
  return true;
}

QStringList JSonSettings::getSetNames(QJsonObject &jsonObject) {
  QStringList setNames;
  QJsonArray jsonEntries = jsonObject["Sets"].toArray();
  for (int i = 0; i < jsonEntries.size(); ++i) {
    QJsonObject entry = jsonEntries[i].toObject();
    setNames.append(entry["Name"].toString());
  }
  return setNames;
}

bool JSonSettings::getSetDetails(QJsonObject &jsonObject, int index, QString &startDir, QStringList &excludedDirs){
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

int JSonSettings::getSetDetailsName(QJsonObject &jsonObject, QString name) {
  QJsonArray jsonEntries = jsonObject["Sets"].toArray();
  for (int i = 0; i < jsonEntries.count(); ++i) {
    QJsonObject entry = jsonEntries[i].toObject();
    if (entry["Name"] == name) {
      return i;
    }
  }
  return -1;
}
