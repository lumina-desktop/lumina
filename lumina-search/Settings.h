
#ifndef _LUMINA_SEARCH_SETTINGS_H
#define _LUMINA_SEARCH_SETTINGS_H

#include <QJsonObject>
#include <QStringList>
#include <QString>



class JSonSettings {
  
public:

  static bool loadJsonSettings(QJsonObject &jsonObject);
  static bool saveJsonSettings(QJsonObject &jsonObject);
  static QStringList getSetNames(QJsonObject &jsonObject);
  static bool getSetDetails(QJsonObject &jsonObject, int index, QString &startDir, QStringList &excludedDirs);
  static int getSetDetailsName(QJsonObject &jsonObject, QString name);
};
#endif
