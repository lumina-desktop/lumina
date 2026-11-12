//===========================================
//  qalarm source code
//  Copyright (c) 2017, q5sys
//  Available under the MIT License
//  See the LICENSE file for full details
//===========================================

#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    mainWindow w;
    w.show();

    return a.exec();
}
