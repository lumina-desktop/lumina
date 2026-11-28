#ifndef LTHEMEENGINEPROXYSTYLE_H
#define LTHEMEENGINEPROXYSTYLE_H

#include <QProxyStyle>

class lthemeengineProxyStyle : public QProxyStyle{
    Q_OBJECT
public:
    explicit lthemeengineProxyStyle(const QString &key);

    virtual ~lthemeengineProxyStyle();

    int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const;

private:
    int m_dialogButtonsHaveIcons;
    int m_activateItemOnSingleClick;

};

#endif // LTHEMEENGINEPROXYSTYLE_H
