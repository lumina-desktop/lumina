#ifndef TABPAGE_H
#define TABPAGE_H

#include <QWidget>

class TabPage : public QWidget
{
    Q_OBJECT
public:
    explicit TabPage(QWidget *parent = 0);

    virtual void writeSettings() = 0;
};

#endif // TABPAGE_H
