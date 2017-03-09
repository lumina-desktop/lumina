//-------------------------------------------------
// Created by q5sys
// Released under MIT License 2017-03-08
// A Simple GUI Dialog Program
//-------------------------------------------------

#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QMessageBox *messageBox = new QMessageBox;
        messageBox->setText(argv[1]);
        QPushButton *pushButtonOk = messageBox->addButton(argv[2], QMessageBox::YesRole);
        messageBox->QDialog::setWindowTitle(argv[3]);
    messageBox->show();
    return a.exec();
}
