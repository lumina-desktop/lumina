#include <QApplication>
#include "dialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dialog w;
    if (argc==2) {
        w.LoadDesktopFile(QString(argv[1]).simplified());
    } else if (argc==3) {
        w.Initialise(QString(argv[1]).simplified());
        w.LoadDesktopFile(QString(argv[2]).simplified());
    } else {
        w.MissingInputs();
    }
    a.setApplicationName("Desktop Editor");
    w.show();

    return a.exec();
}
