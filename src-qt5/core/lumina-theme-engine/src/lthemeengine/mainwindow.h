#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

class QAbstractButton;

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

private:
    void closeEvent(QCloseEvent *);

    Ui::MainWindow *m_ui;
};

#endif // MAINWINDOW_H
