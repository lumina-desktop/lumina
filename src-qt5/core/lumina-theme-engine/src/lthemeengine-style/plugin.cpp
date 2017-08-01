#include <QStylePlugin>
#include <QSettings>
#include <QStyleFactory>
#include <lthemeengine/lthemeengine.h>
#include "lthemeengineproxystyle.h"

class lthemeengineStylePlugin : public QStylePlugin{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QStyleFactoryInterface" FILE "lthemeengine.json")

public:
    QStyle *create(const QString &key);
};

QStyle *lthemeengineStylePlugin::create(const QString &key){
  if (key == "lthemeengine-style"){
    QSettings settings(lthemeengine::configFile(), QSettings::IniFormat);
    QString style = settings.value("Appearance/style", "Fusion").toString();
    if(key == style || !QStyleFactory::keys().contains(style)){ style = "Fusion"; }
    return new lthemeengineProxyStyle(style);
    }
  return 0;
}

#include "plugin.moc"
