//===========================================
//  Copyright (c) 2018, JT(q5sys)
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

private slots:
    void on_pushButton_SetTime_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
