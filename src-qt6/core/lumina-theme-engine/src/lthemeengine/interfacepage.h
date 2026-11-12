#ifndef INTERFACEPAGE_H
#define INTERFACEPAGE_H

#include "tabpage.h"

namespace Ui {
class InterfacePage;
}

class InterfacePage : public TabPage
{
    Q_OBJECT

public:
    explicit InterfacePage(QWidget *parent = 0);
    ~InterfacePage();

    void writeSettings();

private:
    void readSettings();

    Ui::InterfacePage *m_ui;
};

#endif // INTERFACEPAGE_H
