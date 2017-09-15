#ifndef CURSORTHEMEPAGE_H
#define CURSORTHEMEPAGE_H

#include <QIcon>
#include "tabpage.h"

namespace Ui {
class CursorThemePage;
}

class CursorThemePage : public TabPage
{
    Q_OBJECT

public:
    explicit CursorThemePage(QWidget *parent = 0);
    ~CursorThemePage();

    void writeSettings();

private:
    void readSettings();
    void loadThemes();
    void loadTheme(const QString &path);
    QIcon findIcon(const QString &themePath, int size, const QString &name);
    Ui::CursorThemePage *m_ui;
};

#endif // ICONTHEMEPAGE_H
