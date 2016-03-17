#include <QApplication>
#include "test.h"

int main(int argc, char ** argv)
{
   QApplication a(argc, argv);
    Test w;
    w.show();

    int retCode = a.exec();
    return retCode;
}