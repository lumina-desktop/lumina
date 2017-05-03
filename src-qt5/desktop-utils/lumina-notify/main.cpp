//-------------------------------------------------
// Created by q5sys (JT)
// Released under MIT License 2017-03-08
// A Simple GUI Dialog Program
//-------------------------------------------------

#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);   
    int answer;
    QMessageBox *messageBox = new QMessageBox;
        messageBox->setText(argv[1]);
        QPushButton *pushButtonOk = messageBox->addButton(argv[2], QMessageBox::AcceptRole);
        QPushButton *pushButtonNo = messageBox->addButton(argv[3], QMessageBox::RejectRole);
        messageBox->QDialog::setWindowTitle(argv[4]);
        messageBox->show();
        if(messageBox->exec() == QMessageBox::AcceptRole){ answer = 0; QTextStream cout(stdout); cout << answer;}
        else { answer = 1; QTextStream cout(stdout); cout << answer;}
}
