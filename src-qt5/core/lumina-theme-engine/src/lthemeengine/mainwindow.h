#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QAbstractButton>
#include <QButtonGroup>
//class QAbstractButton;

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
	void closeWindow();
	void applyWindow();

private:
    void closeEvent(QCloseEvent *);

    Ui::MainWindow *m_ui;
    QButtonGroup *bgroup;
};

#endif // MAINWINDOW_H
