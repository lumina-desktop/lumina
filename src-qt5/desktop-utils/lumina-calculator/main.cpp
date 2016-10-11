//===========================================
//  Lumina Desktop source code
//  Copyright (c) 2016, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include <QApplication>
#include <QDebug>

#include "mainUI.h"

int  main(int argc, char *argv[]) {
   QApplication a(argc, argv);

   //Now start the window
   mainUI W;
   W.show();
   return  a.exec();
}
