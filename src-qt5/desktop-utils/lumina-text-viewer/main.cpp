#include "ltv.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ltv w;

    w.show();

    return a.exec();
}
