#include <qpa/qplatformthemeplugin.h>
#include "lthemeengineplatformtheme.h"

QT_BEGIN_NAMESPACE

class lthemeenginePlatformThemePlugin: public QPlatformThemePlugin{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QPA.QPlatformThemeFactoryInterface.5.1" FILE "lthemeengine.json")
public:
    QPlatformTheme *create(const QString &key, const QStringList &params) override;
};

QPlatformTheme *lthemeenginePlatformThemePlugin::create(const QString &key, const QStringList &params){
  Q_UNUSED(params);
  if (key.toLower() == "lthemeengine") { return new lthemeenginePlatformTheme(); }
  return nullptr;
}

QT_END_NAMESPACE

#include "main.moc"
