#include <QSettings>
#include <lthemeengine/lthemeengine.h>
#include "lthemeengineproxystyle.h"

lthemeengineProxyStyle::lthemeengineProxyStyle(const QString &key) : QProxyStyle(key){
  QSettings settings(lthemeengine::configFile(), QSettings::IniFormat);
  m_dialogButtonsHaveIcons = settings.value("Interface/dialog_buttons_have_icons", Qt::PartiallyChecked).toInt();
  m_activateItemOnSingleClick = settings.value("Interface/activate_item_on_single_click", Qt::PartiallyChecked).toInt();
}

lthemeengineProxyStyle::~lthemeengineProxyStyle(){
  //qDebug("%s", Q_FUNC_INFO);
}

int lthemeengineProxyStyle::styleHint(QStyle::StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const{
  if(hint == QStyle::SH_DialogButtonBox_ButtonsHaveIcons){
    if(m_dialogButtonsHaveIcons == Qt::Unchecked){ return 0; }
    else if(m_dialogButtonsHaveIcons == Qt::Checked){ return 1; }
    }
  else if(hint == QStyle::QStyle::SH_ItemView_ActivateItemOnSingleClick){
    if(m_activateItemOnSingleClick == Qt::Unchecked){ return 0; }
    else if(m_activateItemOnSingleClick == Qt::Checked){ return 1; }
    }
  return QProxyStyle::styleHint(hint, option, widget, returnData);
}
