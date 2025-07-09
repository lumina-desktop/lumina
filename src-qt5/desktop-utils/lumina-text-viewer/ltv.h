#ifndef LTV_H
#define LTV_H

#include <QMainWindow>
#include <QDialog>
#include <QFileSystemModel>

namespace Ui { class ltv;
}

class ltv : public QMainWindow{
    Q_OBJECT

public:
    explicit ltv(QWidget *parent = 0);
    ~ltv();

private slots:
    void on_dirview_clicked(const QModelIndex &index);
    void on_fileview_clicked(const QModelIndex &index);

private:
    Ui::ltv *ui;
    QFileSystemModel *dirModel; QFileSystemModel *fileModel;
};


#endif // LTV_H
