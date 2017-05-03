//===========================================
//  Copyright (c) 2017, q5sys (JT)
//  Available under the MIT license
//  See the LICENSE file for full details
//===========================================
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle(".Desktop Creator");
    w.show();

    return a.exec();
}
