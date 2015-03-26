#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <LuminaXDG.h>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit Dialog(QWidget *parent = 0);

    XDGDesktop DF ;

    QString desktopFileName ;
    QString iconFileName;
    QString desktopType;

    void Initialise(QString);
    void MissingInputs();
    void LoadDesktopFile(QString);

    ~Dialog();
    
private slots:

    void on_pbCommand_clicked();

    void on_pbWorkingDir_clicked();

    void on_pbApply_clicked();

    void on_pbIcon_clicked();

    void textReplace(QString &origin, QString from, QString to, QString topic);

private:
    Ui::Dialog *ui;
};

#endif // DIALOG_H
