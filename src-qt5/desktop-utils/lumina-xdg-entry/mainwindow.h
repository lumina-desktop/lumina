//===========================================
//  Copyright (c) 2017, q5sys (JT)
//  Available under the MIT license
//  See the LICENSE file for full details    
//===========================================

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QString name, genericname, comment, icon, executable, terminal, keywords, catList, categories, iconpath, execpath;
    QString namefield, genericnamefield, commentfield, iconfield, terminalfield,execfield, categoriesfield, keywordfield;

public slots:
    void setIcon();
    void setExec();
    void setCategories();
    void setOtherValues();
    void setDesktopFields();
    void save();


private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
