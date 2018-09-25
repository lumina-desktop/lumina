//===========================================
//  Copyright (c) 2018, JT(q5sys)
//  Available under the MIT license
//  See the LICENSE file for full details
//===========================================
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Set Date & Time");
    MainWindow w;
    w.setWindowTitle("Set Date & Time");
    w.show();

    return a.exec();
}
