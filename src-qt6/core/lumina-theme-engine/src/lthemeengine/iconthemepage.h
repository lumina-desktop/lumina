#ifndef ICONTHEMEPAGE_H
#define ICONTHEMEPAGE_H

#include <QIcon>
#include "tabpage.h"

namespace Ui {
class IconThemePage;
}

class IconThemePage : public TabPage
{
    Q_OBJECT

public:
    explicit IconThemePage(QWidget *parent = 0);
    ~IconThemePage();

    void writeSettings();

private:
    void readSettings();
    void loadThemes();
    void loadTheme(const QString &path);
    QIcon findIcon(const QString &themePath, int size, const QString &name);
    Ui::IconThemePage *m_ui;
};

#endif // ICONTHEMEPAGE_H
